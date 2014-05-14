
static u4 applyTraceStoreEliminationAndSinkingBackEndAnalysis(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    if (cUnit->jitMode == kJitTrace) {
        if (bUnit->cUnitHasInnerBranch)
            return DO_NOT_OPT;

        for (int i = 0; i < bUnit->branchList.instCount; i++) {
            if (bUnit->branchList.inst[i]->generic.target == NULL)
                return DO_NOT_OPT;
        }

        if (bUnit->strSlotList.instCount != bUnit->branchList.instCount)
            return DO_NOT_OPT;

        if (bUnit->strList.instCount)
            return TRACE_STR_ELIMINATION_SINKING;
    }
    return DO_NOT_OPT;
}

static u4 applyLICMBackEndAnalysis(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    u4 loopPass = 0;

    if (cUnit->jitMode != kJitLoop)
        return DO_NOT_OPT;

    if (bUnit->cUnitHasInnerBranch)
        return DO_NOT_OPT;

    for (int i = 0; i < bUnit->branchList.instCount; i++) {
        if (bUnit->branchList.inst[i]->generic.target == NULL)
            return DO_NOT_OPT;
    }

    if (bUnit->ldrList.instCount)
        loopPass |= LOOP_LDR_HOISTING;

    if (bUnit->strList.instCount && cUnit->backEndOptInfo.normalCodeBlockCount == 1 &&
        cUnit->backEndOptInfo.normalChainningBlockCount == 1)
        loopPass |= LOOP_STR_ELIMINATION_SINKING;

    if (bUnit->ldrList.instCount && bUnit->strList.instCount
        && cUnit->backEndOptInfo.normalCodeBlockCount == 1 &&
        cUnit->backEndOptInfo.normalChainningBlockCount == 1)
        loopPass |= LOOP_LDR_STR_PAIR_LICM;

    return loopPass;
}

static u4 applyLoopInversionBackEndAnalysis(CompilationUnit *cUnit)
{
    u4 loopPass = 0;

    if (cUnit->jitMode == kJitLoop) {
        ArmLIR *uncondBLIR = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn;
        ArmLIR *condBLIR = (ArmLIR *)PREV_LIR(uncondBLIR);

        if(uncondBLIR->opcode == kThumbBUncond
            && (condBLIR->opcode == kThumb2BCond || condBLIR->opcode == kThumbBCond)) {
            if(condBLIR->generic.target > uncondBLIR->generic.target) {
                loopPass |= LOOP_INVERSION;
            }
        }
    }

    return loopPass;
}

static bool commonBackEndAnalysis(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    for (ArmLIR *thisLIR = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn; thisLIR = NEXT_LIR(thisLIR)) {
        if ((thisLIR->flags.isNop != true) && !isPseudoOpcode(thisLIR->opcode)) {
            if (bUnit->allList.instCount >= BACKEND_OPT_RANGE)
                return false;

            bUnit->allList.inst[bUnit->allList.instCount] = thisLIR;
            bUnit->allList.position[bUnit->allList.instCount] = bUnit->allList.instCount;
            bUnit->allList.instCount++;
        } else {
            if (isPseudoOpcode(thisLIR->opcode)) {
                bUnit->labelList.inst[bUnit->labelList.instCount] = thisLIR;
                bUnit->labelList.position[bUnit->labelList.instCount++] = bUnit->allList.instCount;
            }
            continue;
        }

        if (EncodingMap[thisLIR->opcode].flags & IS_BRANCH) {
            bUnit->branchList.inst[bUnit->branchList.instCount] = thisLIR;
            bUnit->branchList.position[bUnit->branchList.instCount++] = bUnit->allList.instCount - 1;
        }

        if (thisLIR == (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn)
            break;

        if (thisLIR->operands[1] != r5FP)
            continue;

        if (EncodingMap[thisLIR->opcode].flags & IS_LOAD) {
            bUnit->ldrList.inst[bUnit->ldrList.instCount] = thisLIR;
            bUnit->ldrList.position[bUnit->ldrList.instCount++] = bUnit->allList.instCount - 1;
        }

        if (EncodingMap[thisLIR->opcode].flags & IS_STORE) {
            bUnit->strList.inst[bUnit->strList.instCount] = thisLIR;
            bUnit->strList.position[bUnit->strList.instCount++] = bUnit->allList.instCount - 1;
        }
    }

    for (int i = 0; i < bUnit->branchList.instCount; i++) {
        if ((bUnit->branchList.inst[i]->generic.target ==
            cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn) ||
            (bUnit->branchList.inst[i]->generic.target == NULL))
            continue;

        bool hasInnerBranch = false;

        for (int j = 0; j < bUnit->labelList.instCount; j++) {
            if (bUnit->branchList.inst[i]->generic.target == (LIR *)bUnit->labelList.inst[j]) {
                hasInnerBranch = true;
                break;
            }
        }

        if (hasInnerBranch) {
            bUnit->cUnitHasInnerBranch = true;
            continue;
        }

        bool isDup = false;
        for (int j = 0; j < bUnit->strSlotList.instCount; j++) {
            if (bUnit->strSlotList.inst[j]->generic.target == bUnit->branchList.inst[i]->generic.target) {
                isDup = true;
                break;
            }
        }

        if (isDup == false) {
            bUnit->strSlotList.inst[bUnit->strSlotList.instCount] = bUnit->branchList.inst[i];
            bUnit->strSlotList.position[bUnit->strSlotList.instCount++] = bUnit->branchList.position[i];
        }
    }

    return true;
}
