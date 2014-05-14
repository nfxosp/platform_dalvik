
static void applyTraceStoreEliminationAndSinking(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyTraceStoreEliminationAndSinking start");

    for (int i = 0; i < bUnit->strList.instCount - 1; i++) {
        int dalvikRegInfo = DECODE_ALIAS_INFO_REG(bUnit->strList.inst[i]->aliasInfo);
        int strOperland0 = bUnit->strList.inst[i]->operands[0];
        for (int j = i+1; j < bUnit->strList.instCount; j++) {
            if (dalvikRegInfo == DECODE_ALIAS_INFO_REG(bUnit->strList.inst[j]->aliasInfo)) {
                if (strOperland0 == bUnit->strList.inst[j]->operands[0]) {
                    bool isReDef = false;
                    bool isBranchAfterDef = false;
                    bool isSameAlias = false;

                    for (int k=0; k < bUnit->ldrList.instCount; k++) {
                        if ((bUnit->ldrList.position[k] > bUnit->strList.position[i]) &&
                            (bUnit->ldrList.position[k] < bUnit->strList.position[j])) {
                            if (DECODE_ALIAS_INFO_REG(bUnit->strList.inst[i]->aliasInfo) ==
                                DECODE_ALIAS_INFO_REG(bUnit->ldrList.inst[k]->aliasInfo)) {
                                isSameAlias = true;
                                break;
                            }
                        }
                    }
                    if (isSameAlias)
                        break;

                    for (int k = bUnit->strList.position[i] + 1; k < bUnit->strList.position[j]; k++) {
                        if ((bUnit->allList.inst[k]->defMask & bUnit->strList.inst[i]->useMask & ~ENCODE_MEM) &&
                            (!(EncodingMap[bUnit->allList.inst[k]->opcode].flags & IS_BRANCH)) &&
                            (!(EncodingMap[bUnit->allList.inst[k]->opcode].flags & IS_IT))) {
                            isReDef = true;
                        }
                        if (isReDef) {
                            if (EncodingMap[bUnit->allList.inst[k]->opcode].flags & IS_BRANCH)
                                isBranchAfterDef = true;
                        }
                    }
                    if (isBranchAfterDef == false) {
                        for (int l=0; l < bUnit->strSlotList.instCount; l++) {
                            if ((bUnit->strList.position[i] <= bUnit->strSlotList.position[l]) &&
                                (bUnit->strList.position[j] >= bUnit->strSlotList.position[l])) {
                                LIR *modifiedTarget = NULL;
                                ArmLIR *newLoadLIR = (ArmLIR *) dvmCompilerNew(sizeof(ArmLIR), true);
                                *newLoadLIR = *bUnit->strList.inst[i];
                                modifiedTarget = (LIR *)newLoadLIR;
                                dvmCompilerInsertLIRAfter((LIR *)bUnit->strSlotList.inst[l], (LIR *)newLoadLIR);

                                for (LIR *tmp = bUnit->strSlotList.inst[l]->generic.next->next; tmp; tmp = tmp->next) {
                                    if (((ArmLIR *)tmp)->opcode == kArmPseudoBackEndOptTargetLabel)
                                        break;
                                    if (isPseudoOpcode(((ArmLIR *)tmp)->opcode) ||
                                        (!(EncodingMap[((ArmLIR *)tmp)->opcode].flags & IS_STORE)))
                                        continue;
                                }
                            }
                        }
                        bUnit->strList.inst[i]->flags.isNop = true;
                    }
                    break;
                } else {
                    bool isBranchSeen = false;
                    bool isSameAlias = false;
                    for (int k=0; k < bUnit->branchList.instCount; k++) {
                        if ((bUnit->branchList.position[k] > bUnit->strList.position[i]) &&
                            (bUnit->strList.position[k] < bUnit->strList.position[j])) {
                                isBranchSeen = true;
                                break;
                        }
                    }
                    if (isBranchSeen)
                        break;

                    for (int k=0; k < bUnit->ldrList.instCount; k++) {
                        if ((bUnit->ldrList.position[k] > bUnit->strList.position[i]) &&
                            (bUnit->ldrList.position[k] < bUnit->strList.position[j])) {
                            if (DECODE_ALIAS_INFO_REG(bUnit->strList.inst[i]->aliasInfo) ==
                                DECODE_ALIAS_INFO_REG(bUnit->ldrList.inst[k]->aliasInfo)) {
                                isSameAlias = true;
                                break;
                            }
                        }
                    }
                    if (isSameAlias)
                        break;
                    bUnit->strList.inst[i]->flags.isNop = true;
                }
            }
        }
    }

    if (gDvmJit.jitDebug)
        ALOGD("[SWE]     applyTraceStoreEliminationAndSinking exit");

    return;
}

