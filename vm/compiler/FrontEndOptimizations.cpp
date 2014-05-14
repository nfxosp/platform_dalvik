
bool dvmCompilerFindInductionVariablesOpt(struct CompilationUnit *cUnit,
                                       struct BasicBlock *bb)
{
    BitVector *isIndVarV = cUnit->loopAnalysis->isIndVarV;
    BitVector *isConstantV = cUnit->isConstantV;
    GrowableList *ivList = cUnit->loopAnalysis->ivList;
    MIR *mir;

    if (bb->blockType != kDalvikByteCode && bb->blockType != kEntryBlock) {
        return false;
    }

    /* If the bb doesn't have a phi it cannot contain an induction variable */
    if (bb->firstMIRInsn == NULL ||
        (int)bb->firstMIRInsn->dalvikInsn.opcode != (int)kMirOpPhi) {
        return false;
    }

    /* Find basic induction variable first */
    for (mir = bb->firstMIRInsn; mir; mir = mir->next) {
        int dfAttributes =
            dvmCompilerDataFlowAttributes[mir->dalvikInsn.opcode];

        if (!(dfAttributes & DF_IS_LINEAR)) continue;

        /*
         * For a basic induction variable:
         *   1) use[0] should belong to the output of a phi node
         *   2) def[0] should belong to the input of the same phi node
         *   3) the value added/subtracted is a constant
         */
        MIR *phi;
        for (phi = bb->firstMIRInsn; phi; phi = phi->next) {
            if ((int)phi->dalvikInsn.opcode != (int)kMirOpPhi) break;

            if (phi->ssaRep->defs[0] == mir->ssaRep->uses[0] &&
                phi->ssaRep->uses[1] == mir->ssaRep->defs[0]) {
                bool deltaIsConstant = false;
                int deltaValue;

                switch (mir->dalvikInsn.opcode) {
                    case OP_ADD_INT:
                        if (dvmIsBitSet(isConstantV,
                                        mir->ssaRep->uses[1])) {
                            deltaValue =
                                cUnit->constantValues[mir->ssaRep->uses[1]];
                            deltaIsConstant = true;
                        }
                        break;
                    case OP_SUB_INT:
                        if (dvmIsBitSet(isConstantV,
                                        mir->ssaRep->uses[1])) {
                            deltaValue =
                                -cUnit->constantValues[mir->ssaRep->uses[1]];
                            deltaIsConstant = true;
                        }
                        break;
                    case OP_ADD_INT_LIT8:
                        deltaValue = mir->dalvikInsn.vC;
                        deltaIsConstant = true;
                        break;
                    default:
                        break;
                }
                if (deltaIsConstant) {
                    dvmSetBit(isIndVarV, mir->ssaRep->uses[0]);
                    InductionVariableInfo *ivInfo = (InductionVariableInfo *)
                        dvmCompilerNew(sizeof(InductionVariableInfo),
                                       false);

                    ivInfo->ssaReg = mir->ssaRep->uses[0];
                    ivInfo->basicSSAReg = mir->ssaRep->uses[0];
                    ivInfo->constSSAReg = DIV_CONST;
                    ivInfo->m = 1;         // always 1 to basic iv
                    ivInfo->c = 0;         // N/A to basic iv
                    ivInfo->inc = deltaValue;
                    dvmInsertGrowableList(ivList, (intptr_t) ivInfo);
                    cUnit->loopAnalysis->numBasicIV++;
                    break;
                }
            }
        }
    }

    /* Find dependent induction variable now */
    for (mir = bb->firstMIRInsn; mir; mir = mir->next) {
        int dfAttributes =
            dvmCompilerDataFlowAttributes[mir->dalvikInsn.opcode];

        if (!(dfAttributes & DF_IS_LINEAR)) continue;

        /* Skip already identified induction variables */
        if (dvmIsBitSet(isIndVarV, mir->ssaRep->defs[0])) continue;

        /*
         * For a dependent induction variable:
         *  1) use[0] should be an induction variable (basic/dependent)
         *  2) operand2 should be a constant
         */
        if (dvmIsBitSet(isIndVarV, mir->ssaRep->uses[0])) {
            int srcDalvikReg = dvmConvertSSARegToDalvik(cUnit,
                                                        mir->ssaRep->uses[0]);
            int dstDalvikReg = dvmConvertSSARegToDalvik(cUnit,
                                                        mir->ssaRep->defs[0]);

            bool cIsConstant = false;
            bool cIsConstantReg = false;
            int c = 0;

            switch (mir->dalvikInsn.opcode) {
                case OP_ADD_INT:
                    if (dvmIsBitSet(isConstantV,
                                    mir->ssaRep->uses[1])) {
                        c = cUnit->constantValues[mir->ssaRep->uses[1]];
                        cIsConstant = true;
                    } else {
                        MIR *tmp = NULL;
                        cIsConstantReg = true;
                        for (tmp = mir->prev; tmp ; tmp = tmp->prev) {
                            for (int i = 0; i < tmp->ssaRep->numDefs; i++) {
                                if (tmp->ssaRep->defs[i] == mir->ssaRep->uses[1]) {
                                    cIsConstantReg = false;
                                    break;
                                }
                            }
                            if (cIsConstantReg == false)
                                break;
                        }
                    }
                    break;
                case OP_SUB_INT:
                    if (dvmIsBitSet(isConstantV,
                                    mir->ssaRep->uses[1])) {
                        c = -cUnit->constantValues[mir->ssaRep->uses[1]];
                        cIsConstant = true;
                    }
                    break;
                case OP_ADD_INT_LIT8:
                    c = mir->dalvikInsn.vC;
                    cIsConstant = true;
                    break;
                default:
                    break;
            }

            /* Ignore the update to the basic induction variable itself */
            if (DECODE_REG(srcDalvikReg) == DECODE_REG(dstDalvikReg))  {
                cUnit->loopAnalysis->ssaBIV = mir->ssaRep->defs[0];
                cIsConstant = false;
            }

            if (cIsConstant) {
                unsigned int i;
                dvmSetBit(isIndVarV, mir->ssaRep->defs[0]);
                InductionVariableInfo *ivInfo = (InductionVariableInfo *)
                    dvmCompilerNew(sizeof(InductionVariableInfo),
                                   false);
                InductionVariableInfo *ivInfoOld = NULL ;

                for (i = 0; i < ivList->numUsed; i++) {
                    ivInfoOld = (InductionVariableInfo *) ivList->elemList[i];
                    if (ivInfoOld->ssaReg == mir->ssaRep->uses[0]) break;
                }

                /* Guaranteed to find an element */
                assert(i < ivList->numUsed);

                ivInfo->ssaReg = mir->ssaRep->defs[0];
                ivInfo->basicSSAReg = ivInfoOld->basicSSAReg;
                ivInfo->constSSAReg = DIV_CONST;
                ivInfo->m = ivInfoOld->m;
                ivInfo->c = c + ivInfoOld->c;
                ivInfo->inc = ivInfoOld->inc;
                dvmInsertGrowableList(ivList, (intptr_t) ivInfo);
            } else if (cIsConstantReg) {
                unsigned int i;
                dvmSetBit(isIndVarV, mir->ssaRep->defs[0]);
                InductionVariableInfo *ivInfo = (InductionVariableInfo *)
                    dvmCompilerNew(sizeof(InductionVariableInfo),
                                   false);
                InductionVariableInfo *ivInfoOld = NULL ;

                for (i = 0; i < ivList->numUsed; i++) {
                    ivInfoOld = (InductionVariableInfo *) ivList->elemList[i];
                    if (ivInfoOld->ssaReg == mir->ssaRep->uses[0])
                        break;
                }

                /* Guaranteed to find an element */
                assert(i < ivList->numUsed);

                ivInfo->ssaReg = mir->ssaRep->defs[0];
                ivInfo->basicSSAReg = ivInfoOld->basicSSAReg;
                ivInfo->constSSAReg = mir->ssaRep->uses[1];
                ivInfo->m = ivInfoOld->m;
                ivInfo->c = 0;
                ivInfo->inc = ivInfoOld->inc;
                dvmInsertGrowableList(ivList, (intptr_t) ivInfo);
            }
        }
    }
    return true;
}

bool updateRangeCheckInfoOpt(CompilationUnit *cUnit, int arrayReg,
                                 int idxReg)
{
    InductionVariableInfo *ivInfo;
    LoopAnalysis *loopAnalysis = cUnit->loopAnalysis;
    unsigned int i, j;

    for (i = 0; i < loopAnalysis->ivList->numUsed; i++) {
        ivInfo = GET_ELEM_N(loopAnalysis->ivList, InductionVariableInfo*, i);

        if (ivInfo->ssaReg == idxReg && ivInfo->constSSAReg == DIV_CONST) {
            ArrayAccessInfo *arrayAccessInfo = NULL;
            for (j = 0; j < loopAnalysis->arrayAccessInfo->numUsed; j++) {
                ArrayAccessInfo *existingArrayAccessInfo =
                    GET_ELEM_N(loopAnalysis->arrayAccessInfo,
                               ArrayAccessInfo*,
                               j);
                if ((existingArrayAccessInfo->arrayReg == arrayReg) &&
                    (existingArrayAccessInfo->constReg == DIV_CONST)) {
                    if (ivInfo->c > existingArrayAccessInfo->maxC)
                        existingArrayAccessInfo->maxC = ivInfo->c;
                    if (ivInfo->c < existingArrayAccessInfo->minC)
                        existingArrayAccessInfo->minC = ivInfo->c;
                    arrayAccessInfo = existingArrayAccessInfo;
                    break;
                }
            }
            if (arrayAccessInfo == NULL) {
                arrayAccessInfo =
                    (ArrayAccessInfo *)dvmCompilerNew(sizeof(ArrayAccessInfo),
                                                      false);
                arrayAccessInfo->ivReg = ivInfo->basicSSAReg;
                arrayAccessInfo->arrayReg = arrayReg;
                arrayAccessInfo->constReg = DIV_CONST;
                arrayAccessInfo->maxC = (ivInfo->c > 0) ? ivInfo->c : 0;
                arrayAccessInfo->minC = (ivInfo->c < 0) ? ivInfo->c : 0;
                dvmInsertGrowableList(loopAnalysis->arrayAccessInfo,
                                      (intptr_t) arrayAccessInfo);
            }
            break;
        } else if (ivInfo->ssaReg == idxReg && ivInfo->constSSAReg != DIV_CONST) {
            ArrayAccessInfo *arrayAccessInfo = NULL;
            for (j = 0; j < loopAnalysis->arrayAccessInfo->numUsed; j++) {
                ArrayAccessInfo *existingArrayAccessInfo =
                    GET_ELEM_N(loopAnalysis->arrayAccessInfo,
                               ArrayAccessInfo*,
                               j);
                if (existingArrayAccessInfo->arrayReg == arrayReg) {
                    if (existingArrayAccessInfo->constReg == ivInfo->constSSAReg)
                        arrayAccessInfo = existingArrayAccessInfo;
                    break;
                }
            }
            if (arrayAccessInfo == NULL) {
                arrayAccessInfo =
                    (ArrayAccessInfo *)dvmCompilerNew(sizeof(ArrayAccessInfo),
                                                      false);
                arrayAccessInfo->ivReg = ivInfo->basicSSAReg;
                arrayAccessInfo->arrayReg = arrayReg;
                arrayAccessInfo->constReg = ivInfo->constSSAReg;
                arrayAccessInfo->maxC = 0;
                arrayAccessInfo->minC = 0;
                dvmInsertGrowableList(loopAnalysis->arrayAccessInfo,
                                      (intptr_t) arrayAccessInfo);
            }
        }
    }
    return true;
}

bool genHoistedChecksOpt(CompilationUnit *cUnit)
{
    unsigned int i;
    BasicBlock *entry = cUnit->entryBlock;
    LoopAnalysis *loopAnalysis = cUnit->loopAnalysis;
    int globalMaxC = 0;
    int globalMinC = 0;
    /* Should be loop invariant */
    int idxReg = 0;
    bool isMixedAccess = false;

    for (i = 0; i < loopAnalysis->arrayAccessInfo->numUsed; i++) {
        ArrayAccessInfo *arrayAccessInfo =
            GET_ELEM_N(loopAnalysis->arrayAccessInfo,
                       ArrayAccessInfo*, i);
        int arrayReg = DECODE_REG(
            dvmConvertSSARegToDalvik(cUnit, arrayAccessInfo->arrayReg));
        idxReg = DECODE_REG(
            dvmConvertSSARegToDalvik(cUnit, arrayAccessInfo->ivReg));

        MIR *rangeCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR), true);
        rangeCheckMIR->dalvikInsn.opcode = (loopAnalysis->isCountUpLoop) ?
            (Opcode)kMirOpNullNRangeUpCheck : (Opcode)kMirOpNullNRangeDownCheck;
        rangeCheckMIR->dalvikInsn.vA = arrayReg;
        rangeCheckMIR->dalvikInsn.vB = idxReg;
        rangeCheckMIR->dalvikInsn.vC = loopAnalysis->endConditionReg;
        rangeCheckMIR->dalvikInsn.arg[0] = arrayAccessInfo->maxC;
        rangeCheckMIR->dalvikInsn.arg[1] = arrayAccessInfo->minC;
        rangeCheckMIR->dalvikInsn.arg[2] = loopAnalysis->loopBranchOpcode;

        if (arrayAccessInfo->constReg != DIV_CONST)
            isMixedAccess = true;

        rangeCheckMIR->dalvikInsn.arg[4] = arrayAccessInfo->constReg;
        dvmCompilerAppendMIR(entry, rangeCheckMIR);

        if (arrayAccessInfo->constReg == DIV_CONST) {
            if (arrayAccessInfo->maxC > globalMaxC) {
                globalMaxC = arrayAccessInfo->maxC;
            }
            if (arrayAccessInfo->minC < globalMinC) {
                globalMinC = arrayAccessInfo->minC;
            }
        }
    }

    if (isMixedAccess == false) {
        if (loopAnalysis->arrayAccessInfo->numUsed != 0) {
            if (loopAnalysis->isCountUpLoop) {
                MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR), true);
                boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpLowerBound;
                boundCheckMIR->dalvikInsn.vA = idxReg;
                boundCheckMIR->dalvikInsn.vB = globalMinC;
                boundCheckMIR->dalvikInsn.arg[4] = DIV_CONST;
                dvmCompilerAppendMIR(entry, boundCheckMIR);
            } else {
                if (loopAnalysis->loopBranchOpcode == OP_IF_LT ||
                    loopAnalysis->loopBranchOpcode == OP_IF_LE) {
                    MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR), true);
                    boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpLowerBound;
                    boundCheckMIR->dalvikInsn.vA = loopAnalysis->endConditionReg;
                    boundCheckMIR->dalvikInsn.vB = globalMinC;
                    boundCheckMIR->dalvikInsn.arg[4] = DIV_CONST;
                    /*
                     * If the end condition is ">" in the source, the check in the
                     * Dalvik bytecode is OP_IF_LE. In this case add 1 back to the
                     * constant field to reflect the fact that the smallest index
                     * value is "endValue + constant + 1".
                     */
                    if (loopAnalysis->loopBranchOpcode == OP_IF_LE) {
                        boundCheckMIR->dalvikInsn.vB++;
                    }
                    dvmCompilerAppendMIR(entry, boundCheckMIR);
                } else if (loopAnalysis->loopBranchOpcode == OP_IF_LTZ) {
                    /* Array index will fall below 0 */
                    if (globalMinC < 0) {
                        MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR),
                                                                   true);
                        boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpPunt;
                        boundCheckMIR->dalvikInsn.arg[4] = DIV_CONST;
                        dvmCompilerAppendMIR(entry, boundCheckMIR);
                    }
                } else if (loopAnalysis->loopBranchOpcode == OP_IF_LEZ) {
                    /* Array index will fall below 0 */
                    if (globalMinC < -1) {
                        MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR),
                                                                   true);
                        boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpPunt;
                        boundCheckMIR->dalvikInsn.arg[4] = DIV_CONST;
                        dvmCompilerAppendMIR(entry, boundCheckMIR);
                    }
                } else {
                    ALOGE("Jit: bad case in genHoistedChecks");
                    dvmCompilerAbort(cUnit);
                }
            }
        }
    } else {
        for (i = 0; i < loopAnalysis->arrayAccessInfo->numUsed; i++) {
            ArrayAccessInfo *arrayAccessInfo =
                GET_ELEM_N(loopAnalysis->arrayAccessInfo,
                           ArrayAccessInfo*, i);
            if (loopAnalysis->isCountUpLoop) {
                MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR), true);
                boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpLowerBound;
                boundCheckMIR->dalvikInsn.vA = idxReg;
                if (arrayAccessInfo->constReg == DIV_CONST)
                    boundCheckMIR->dalvikInsn.vB = globalMinC;
                else
                    boundCheckMIR->dalvikInsn.vB = 0;
                boundCheckMIR->dalvikInsn.arg[4] = arrayAccessInfo->constReg;
                dvmCompilerAppendMIR(entry, boundCheckMIR);
            } else {
                if (loopAnalysis->loopBranchOpcode == OP_IF_LT ||
                    loopAnalysis->loopBranchOpcode == OP_IF_LE) {
                    MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR), true);
                    boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpLowerBound;
                    boundCheckMIR->dalvikInsn.vA = loopAnalysis->endConditionReg;
                    if (arrayAccessInfo->constReg == DIV_CONST)
                        boundCheckMIR->dalvikInsn.vB = globalMinC;
                    else
                        boundCheckMIR->dalvikInsn.vB = 0;
                    boundCheckMIR->dalvikInsn.arg[4] = arrayAccessInfo->constReg;
                    /*
                     * If the end condition is ">" in the source, the check in the
                     * Dalvik bytecode is OP_IF_LE. In this case add 1 back to the
                     * constant field to reflect the fact that the smallest index
                     * value is "endValue + constant + 1".
                     */
                    if (loopAnalysis->loopBranchOpcode == OP_IF_LE)
                        boundCheckMIR->dalvikInsn.vB++;
                    dvmCompilerAppendMIR(entry, boundCheckMIR);
                } else if (loopAnalysis->loopBranchOpcode == OP_IF_LTZ) {
                    if (arrayAccessInfo->constReg == DIV_CONST) {
                        /* Array index will fall below 0 */
                        if (arrayAccessInfo->minC < 0) {
                            MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR),
                                                                       true);
                            boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpPunt;
                            dvmCompilerAppendMIR(entry, boundCheckMIR);
                        }
                    } else {
                        MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR), true);
                        boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpLowerBound;
                        boundCheckMIR->dalvikInsn.vA = loopAnalysis->endConditionReg;
                        boundCheckMIR->dalvikInsn.vB = 0;
                        boundCheckMIR->dalvikInsn.arg[4] = arrayAccessInfo->constReg;
                        dvmCompilerAppendMIR(entry, boundCheckMIR);
                    }
                } else if (loopAnalysis->loopBranchOpcode == OP_IF_LEZ) {
                    if (arrayAccessInfo->constReg == DIV_CONST) {
                        /* Array index will fall below 0 */
                        if (arrayAccessInfo->minC < -1) {
                            MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR),
                                                                       true);
                            boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpPunt;
                            dvmCompilerAppendMIR(entry, boundCheckMIR);
                        }
                    } else {
                        MIR *boundCheckMIR = (MIR *)dvmCompilerNew(sizeof(MIR), true);
                        boundCheckMIR->dalvikInsn.opcode = (Opcode)kMirOpLowerBound;
                        boundCheckMIR->dalvikInsn.vA = loopAnalysis->endConditionReg;
                        boundCheckMIR->dalvikInsn.vB = 1;
                        boundCheckMIR->dalvikInsn.arg[4] = arrayAccessInfo->constReg;
                        dvmCompilerAppendMIR(entry, boundCheckMIR);
                    }
                } else {
                    dvmCompilerAbort(cUnit);
                }
            }
        }
    }
    return true;
}
