
static void applyLoopLoadHoisting(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopLoadHoisting start");

    ArmLIR *canLICMldrList[BACKEND_OPT_RANGE];
    int canLICMldrListCount = 0;
    bool canInsLICM = true;
    for (int i = 0; i < bUnit->ldrList.instCount; i++) {
        canInsLICM = true;
        int dalvikRegInfo = DECODE_ALIAS_INFO_REG(bUnit->ldrList.inst[i]->aliasInfo);

        for (int j = 0; j < bUnit->allList.instCount; j++) {
            if (bUnit->allList.inst[j] == bUnit->ldrList.inst[i])
                continue;
            if((strcmp(EncodingMap[bUnit->allList.inst[j]->opcode].name, "vcmp.f64") == 0) ||
                (strcmp(EncodingMap[bUnit->allList.inst[j]->opcode].name, "vcmp.f32") == 0) ||
                (strcmp(EncodingMap[bUnit->allList.inst[j]->opcode].name, "cmp") == 0) ||
                (EncodingMap[bUnit->allList.inst[j]->opcode].flags & (IS_BRANCH | IS_IT)))
                continue;

            if (bUnit->ldrList.inst[i]->defMask & bUnit->allList.inst[j]->defMask & ~ENCODE_MEM) {
                canInsLICM = false;
                break;
            }
        }
        if (canInsLICM == false)
            continue;

        for (int j = 0; j < bUnit->strList.instCount; j++) {
            if (dalvikRegInfo == DECODE_ALIAS_INFO_REG(bUnit->strList.inst[j]->aliasInfo)) {
                canInsLICM = false;
                break;
            }
        }
        if (canInsLICM)
            canLICMldrList[canLICMldrListCount++] = bUnit->ldrList.inst[i];
    }

    if (canLICMldrListCount) {
        for (int i = 0; i < canLICMldrListCount; i++) {
            ArmLIR *newLoadLIR = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
            *newLoadLIR = *canLICMldrList[i];
            dvmCompilerInsertLIRBefore((LIR *)bUnit->headBlockTargetLabel, (LIR *)newLoadLIR);
            canLICMldrList[i]->flags.isNop = true;
        }
    }

    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopLoadHoisting exit");
}

static void applyLoopStoreSinking(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopStoreSinking start");

    ArmLIR *canLICMStrList[BACKEND_OPT_RANGE];
    int canLICMStrCount = 0;
    bool canInsLICM = true;

    for (int i = 0; i < bUnit->strList.instCount; i++) {
        canInsLICM = true;
        int dalvikRegInfo = DECODE_ALIAS_INFO_REG(bUnit->strList.inst[i]->aliasInfo);

        for (int j = 0; j < bUnit->ldrList.instCount; j++) {
            if (dalvikRegInfo == DECODE_ALIAS_INFO_REG(bUnit->ldrList.inst[j]->aliasInfo)) {
                canInsLICM = false;
                break;
            }
        }
        if (canInsLICM == false)
            continue;

        for (int j = 0; j < bUnit->strList.instCount; j++) {
            if (bUnit->strList.inst[j] == bUnit->strList.inst[i])
                continue;
            if (dalvikRegInfo == DECODE_ALIAS_INFO_REG(bUnit->strList.inst[j]->aliasInfo)) {
                canInsLICM = false;
                break;
            }
        }

        for (int j = bUnit->strList.position[i] + 1; j < bUnit->allList.instCount; j++) {
            if((strcmp(EncodingMap[bUnit->allList.inst[j]->opcode].name, "vcmp.f64") == 0) ||
                (strcmp(EncodingMap[bUnit->allList.inst[j]->opcode].name, "vcmp.f32") == 0) ||
                (strcmp(EncodingMap[bUnit->allList.inst[j]->opcode].name, "cmp") == 0) ||
                (EncodingMap[bUnit->allList.inst[j]->opcode].flags & (IS_BRANCH | IS_IT)))
                continue;

            if (bUnit->strList.inst[i]->useMask & bUnit->allList.inst[j]->defMask & ~ENCODE_MEM) {
                canInsLICM = false;
                break;
            }
        }

        if (canInsLICM)
            canLICMStrList[canLICMStrCount++] = bUnit->strList.inst[i];
    }

    for (int i = 0; i < bUnit->strSlotList.instCount; i++) {
        for (int j = 0; j < canLICMStrCount; j++) {
            ArmLIR *newLoadLIR = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
            *newLoadLIR = *canLICMStrList[j];
            dvmCompilerInsertLIRBefore((LIR *)bUnit->strSlotList.inst[i], (LIR *)newLoadLIR);
        }
    }

    for (int i = 0; i < canLICMStrCount; i++)
        canLICMStrList[i]->flags.isNop = true;


    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopStoreSinking exit");
}

static void applyLoopLoadStorePairLICM(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    ArmLIR *canLICMldrList[BACKEND_OPT_RANGE];
    ArmLIR *canLICMStrList[BACKEND_OPT_RANGE];
    int canLICMldrCount = 0;
    int canLICMStrCount = 0;
    int strMatchCount = 0;
    int strMatchPos = 0;
    bool canInsLICM = true;

    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopLoadStorePairLICM start");

    for (int i = 0; i < bUnit->ldrList.instCount; i++) {
        int dalvikRegInfo = DECODE_ALIAS_INFO_REG(bUnit->ldrList.inst[i]->aliasInfo);

        canInsLICM = true;
        for (int j = 0; j < bUnit->ldrList.instCount; j++) {
            if (bUnit->ldrList.inst[i] == bUnit->ldrList.inst[j])
                continue;
            if (bUnit->ldrList.inst[i]->operands[0] == bUnit->ldrList.inst[j]->operands[0]) {
                canInsLICM = false;
                break;
            }
        }
        if (canInsLICM == false)
            break;

        for (int j = 0; j < bUnit->strList.instCount; j++) {
            if (dalvikRegInfo == DECODE_ALIAS_INFO_REG(bUnit->strList.inst[j]->aliasInfo)) {
                if (bUnit->ldrList.inst[i]->operands[0] != bUnit->strList.inst[j]->operands[0]) {
                    canInsLICM = false;
                    break;
                }
            }
        }
        if (canInsLICM == false)
            break;

        for (int j = 0; j < bUnit->ldrList.instCount; j++) {
            if (bUnit->ldrList.inst[i] == bUnit->ldrList.inst[j])
                continue;
            if (dalvikRegInfo == DECODE_ALIAS_INFO_REG(bUnit->ldrList.inst[j]->aliasInfo)) {
                canInsLICM = false;
                break;
            }
        }
        if (canInsLICM == false)
            break;

        strMatchCount = 0;
        for (int j = 0; j < bUnit->strList.instCount; j++) {
            if ((bUnit->ldrList.inst[i]->operands[0] == bUnit->strList.inst[j]->operands[0]) &&
                (DECODE_ALIAS_INFO_REG(bUnit->ldrList.inst[i]->aliasInfo) == DECODE_ALIAS_INFO_REG(bUnit->strList.inst[j]->aliasInfo))) {
                strMatchCount++;
                strMatchPos = j;
            }
        }
        if (strMatchCount > 0) {
            int k = 0;
            for (int j = 0; j < bUnit->allList.instCount; j++) {
                k = j;
                if (bUnit->ldrList.inst[i] == bUnit->allList.inst[j]) {
                    for (k = k - 1; k >= 0; k--) {
                        if((strcmp(EncodingMap[bUnit->allList.inst[k]->opcode].name, "vcmp.f64") == 0) ||
                            (strcmp(EncodingMap[bUnit->allList.inst[k]->opcode].name, "vcmp.f32") == 0) ||
                            (strcmp(EncodingMap[bUnit->allList.inst[k]->opcode].name, "cmp") == 0) ||
                            (EncodingMap[bUnit->allList.inst[k]->opcode].flags & (IS_BRANCH | IS_IT)))
                            continue;

                        if (bUnit->ldrList.inst[i]->defMask & bUnit->allList.inst[k]->defMask & ~ENCODE_MEM) {
                            canInsLICM = false;
                            break;
                        }
                    }
                }
                if (canInsLICM == false)
                    break;
                if (bUnit->strList.inst[strMatchPos] == bUnit->allList.inst[j]) {
                    for (k = k + 1; k < bUnit->allList.instCount; k++) {
                        if((strcmp(EncodingMap[bUnit->allList.inst[k]->opcode].name, "vcmp.f64") == 0) ||
                            (strcmp(EncodingMap[bUnit->allList.inst[k]->opcode].name, "vcmp.f32") == 0) ||
                            (strcmp(EncodingMap[bUnit->allList.inst[k]->opcode].name, "cmp") == 0) ||
                            (EncodingMap[bUnit->allList.inst[k]->opcode].flags & (IS_BRANCH | IS_IT)))
                            continue;

                        if (bUnit->strList.inst[strMatchPos]->useMask & bUnit->allList.inst[k]->defMask & ~ENCODE_MEM) {
                            canInsLICM = false;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if ((canInsLICM) && (0 < strMatchCount)) {
            canLICMldrList[canLICMldrCount++] = bUnit->ldrList.inst[i];
            canLICMStrList[canLICMStrCount++] = bUnit->strList.inst[strMatchPos];
        }
    }

    for (int i = 0; i < canLICMldrCount; i++) {
        ArmLIR *newLoadLIR = (ArmLIR *) dvmCompilerNew(sizeof(ArmLIR), true);
        *newLoadLIR = *canLICMldrList[i];
        dvmCompilerInsertLIRBefore((LIR *)bUnit->headBlockTargetLabel, (LIR *)newLoadLIR);
        canLICMldrList[i]->flags.isNop = true;
    }

    for (int i = 0; i < bUnit->strSlotList.instCount; i++) {
        for (int j = 0; j < canLICMStrCount; j++) {
            ArmLIR *newLoadLIR = (ArmLIR *) dvmCompilerNew(sizeof(ArmLIR), true);
            *newLoadLIR = *canLICMStrList[j];
            dvmCompilerInsertLIRBefore((LIR *)bUnit->strSlotList.inst[i], (LIR *)newLoadLIR);
        }
    }

    for (int i = 0; i < canLICMStrCount; i++)
        canLICMStrList[i]->flags.isNop = true;

    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopLoadStorePairLICM exit");
}

static void applyLoopInversion(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopInversion start");

    ArmLIR *uncondBLIR = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn;
    ArmLIR *conBLIR = (ArmLIR *)PREV_LIR(uncondBLIR);

    if(uncondBLIR->opcode != kThumbBUncond ||
         !(conBLIR->opcode == kThumb2BCond || conBLIR->opcode == kThumbBCond)) {
        /* Something wrong, but silently returns */
        return;
    }

    switch(conBLIR->operands[1]) {
        case kArmCondEq:
            conBLIR->operands[1] = kArmCondNe;
            break;
        case kArmCondNe:
            conBLIR->operands[1] = kArmCondEq;
            break;
        case kArmCondGe:
            conBLIR->operands[1] = kArmCondLt;
            break;
        case kArmCondGt:
            conBLIR->operands[1] = kArmCondLe;
            break;
        case kArmCondLe:
            conBLIR->operands[1] = kArmCondGt;
            break;
        case kArmCondLt:
            conBLIR->operands[1] = kArmCondGe;
            break;
    }

    LIR *tmpTgt;
    tmpTgt = conBLIR->generic.target;
    conBLIR->generic.target = uncondBLIR->generic.target;
    uncondBLIR->generic.target = tmpTgt;

    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyLoopInversion exit");
}
