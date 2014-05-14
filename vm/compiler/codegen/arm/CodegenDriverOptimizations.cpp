/*
 *
 * Copyright 2013 Samsung Electronics S.LSI Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
bool checkArrayAccess(MIR *mir)
{
    if(dvmCompilerDataFlowAttributes[mir->dalvikInsn.opcode] & DF_HAS_NR_CHECKS)
        return true;
    else
        return false;
}

void checkSameArray(CompilationUnit *cUnit, MIR *mir)
{
    u4 aget_vA;
    u4 aget_vB;
    u4 aget_vC;
    bool breakFlag;
    int numDef;
    MIR *mir2;

    aget_vA = mir->dalvikInsn.vA;
    aget_vB = mir->dalvikInsn.vB;
    aget_vC = mir->dalvikInsn.vC;
    breakFlag = false;

    for (mir2 = mir->next; mir2 && (breakFlag == false); mir2 = mir2->next) {
        if(checkArrayAccess(mir2) && (mir2->dalvikInsn.vB == aget_vB) && (mir2->dalvikInsn.vC == aget_vC)) {
            mir2->OptimizationFlags |= (MIR_IGNORE_NULL_CHECK | MIR_IGNORE_RANGE_CHECK);
            mir2->duplicated = true;
        } else {
            for(numDef = 0; numDef < mir2->ssaRep->numDefs; numDef++) {
                if((aget_vB == DECODE_REG(dvmConvertSSARegToDalvik(cUnit, mir2->ssaRep->defs[numDef])))
                    || (aget_vC == DECODE_REG(dvmConvertSSARegToDalvik(cUnit, mir2->ssaRep->defs[numDef])))) {
                    breakFlag = true;
                }
            }
        }
    }
}

bool genArrayGetOpt(CompilationUnit *cUnit, MIR *mir, OpSize size,
                    RegLocation rlArray, RegLocation rlIndex,
                    RegLocation rlDest, int scale)
{
    RegisterClass regClass = dvmCompilerRegClassBySize(size);
    int lenOffset = OFFSETOF_MEMBER(ArrayObject, length);
    int dataOffset = OFFSETOF_MEMBER(ArrayObject, contents);
    RegLocation rlResult;
    rlArray = loadValue(cUnit, rlArray, kCoreReg);
    rlIndex = loadValue(cUnit, rlIndex, kCoreReg);
    int regPtr;

    /* null object? */
    ArmLIR * pcrLabel = NULL;

    if (!(mir->OptimizationFlags & MIR_IGNORE_NULL_CHECK)) {
        pcrLabel = genNullCheck(cUnit, rlArray.sRegLow,
                                rlArray.lowReg, mir->offset, NULL);
    }

    regPtr = dvmCompilerAllocTemp(cUnit);

    if (!(mir->OptimizationFlags & MIR_IGNORE_RANGE_CHECK)) {
        int regLen = dvmCompilerAllocTemp(cUnit);
        /* Get len */
        loadWordDisp(cUnit, rlArray.lowReg, lenOffset, regLen);
        /* regPtr -> array data */
        if ((size != kLong) && (size != kDouble))
            opRegRegImm(cUnit, kOpAdd, regPtr, rlArray.lowReg, dataOffset);
        genBoundsCheck(cUnit, rlIndex.lowReg, regLen, mir->offset,
                       pcrLabel);
        dvmCompilerFreeTemp(cUnit, regLen);
    } else {
        /* regPtr -> array data */
        if ((size != kLong) && (size != kDouble))
            opRegRegImm(cUnit, kOpAdd, regPtr, rlArray.lowReg, dataOffset);
    }
    if ((size == kLong) || (size == kDouble)) {
        if (scale)
            opRegRegRegShift(cUnit, kOpAdd, regPtr, rlArray.lowReg, rlIndex.lowReg, encodeShift(kArmLsl, scale));
        else
            opRegReg(cUnit, kOpAdd, regPtr, rlIndex.lowReg);

        rlResult = dvmCompilerEvalLoc(cUnit, rlDest, regClass, true);

        HEAP_ACCESS_SHADOW(true);
        loadPair(cUnit, regPtr, rlResult.lowReg, rlResult.highReg, dataOffset);
        HEAP_ACCESS_SHADOW(false);

        if ((scale == 3) && (cUnit->jitMode == kJitLoop))
            newLIR2(cUnit, kThumb2Pld, regPtr, 128);

        dvmCompilerFreeTemp(cUnit, regPtr);
        storeValueWide(cUnit, rlDest, rlResult);
    } else {
        rlResult = dvmCompilerEvalLoc(cUnit, rlDest, regClass, true);

        HEAP_ACCESS_SHADOW(true);
        loadBaseIndexed(cUnit, regPtr, rlIndex.lowReg, rlResult.lowReg,
                        scale, size);
        HEAP_ACCESS_SHADOW(false);

        dvmCompilerFreeTemp(cUnit, regPtr);
        storeValue(cUnit, rlDest, rlResult);
    }

    return true;
}

bool genArrayPutOpt(CompilationUnit *cUnit, MIR *mir, OpSize size,
                    RegLocation rlArray, RegLocation rlIndex,
                    RegLocation rlSrc, int scale)
{
    RegisterClass regClass = dvmCompilerRegClassBySize(size);
    int lenOffset = OFFSETOF_MEMBER(ArrayObject, length);
    int dataOffset = OFFSETOF_MEMBER(ArrayObject, contents);

    int regPtr;
    rlArray = loadValue(cUnit, rlArray, kCoreReg);
    rlIndex = loadValue(cUnit, rlIndex, kCoreReg);

    if (dvmCompilerIsTemp(cUnit, rlArray.lowReg)) {
        dvmCompilerClobber(cUnit, rlArray.lowReg);
        regPtr = rlArray.lowReg;
    } else {
        regPtr = dvmCompilerAllocTemp(cUnit);
        genRegCopy(cUnit, regPtr, rlArray.lowReg);
    }

    /* null object? */
    ArmLIR * pcrLabel = NULL;

    if (!(mir->OptimizationFlags & MIR_IGNORE_NULL_CHECK)) {
        pcrLabel = genNullCheck(cUnit, rlArray.sRegLow, rlArray.lowReg,
                                mir->offset, NULL);
    }

    if (!(mir->OptimizationFlags & MIR_IGNORE_RANGE_CHECK)) {
        int regLen = dvmCompilerAllocTemp(cUnit);
        //NOTE: max live temps(4) here.
        /* Get len */
        loadWordDisp(cUnit, rlArray.lowReg, lenOffset, regLen);
        /* regPtr -> array data */
        if ((size != kLong) && (size != kDouble))
            opRegImm(cUnit, kOpAdd, regPtr, dataOffset);
        genBoundsCheck(cUnit, rlIndex.lowReg, regLen, mir->offset,
                       pcrLabel);
        dvmCompilerFreeTemp(cUnit, regLen);
    } else {
        /* regPtr -> array data */
        if ((size != kLong) && (size != kDouble))
            opRegImm(cUnit, kOpAdd, regPtr, dataOffset);
    }
    /* at this point, regPtr points to array, 2 live temps */
    if ((size == kLong) || (size == kDouble)) {
        //TODO: need specific wide routine that can handle fp regs
        int rNewIndex = dvmCompilerAllocTemp(cUnit);
        if (scale)
            opRegRegRegShift(cUnit, kOpAdd, rNewIndex, rlArray.lowReg, rlIndex.lowReg, encodeShift(kArmLsl, scale));
        else
            opRegRegReg(cUnit, kOpAdd, rNewIndex, regPtr, rlIndex.lowReg);

        rlSrc = loadValueWide(cUnit, rlSrc, regClass);

        HEAP_ACCESS_SHADOW(true);
        storePair(cUnit, rNewIndex, rlSrc.lowReg, rlSrc.highReg, dataOffset);
        HEAP_ACCESS_SHADOW(false);

        dvmCompilerFreeTemp(cUnit, rNewIndex);
        dvmCompilerFreeTemp(cUnit, regPtr);
    } else {
        rlSrc = loadValue(cUnit, rlSrc, regClass);

        HEAP_ACCESS_SHADOW(true);
        storeBaseIndexed(cUnit, regPtr, rlIndex.lowReg, rlSrc.lowReg,
                         scale, size);
        HEAP_ACCESS_SHADOW(false);
    }

    return true;
}

bool genSuspendPollOpt(CompilationUnit *cUnit, MIR *mir)
{
    if (cUnit->hasSuspendPoll)
        return true;
    cUnit->hasSuspendPoll = true;
    return false;
}

bool handleEasyDivideOpt(CompilationUnit *cUnit, Opcode dalvikOpcode,
                         RegLocation rlSrc, RegLocation rlDest, int lit, int k)
{
    bool div = (dalvikOpcode == OP_DIV_INT_LIT8 || dalvikOpcode == OP_DIV_INT_LIT16);
    rlSrc = loadValue(cUnit, rlSrc, kCoreReg);
    RegLocation rlResult = dvmCompilerEvalLoc(cUnit, rlDest, kCoreReg, true);

    if (div) {
        int tReg = dvmCompilerAllocTemp(cUnit);

        if (lit == 2) {
            // Division by 2 is by far the most common division by constant.
            opRegRegRegShift(cUnit, kOpAdd, tReg, rlSrc.lowReg, rlSrc.lowReg, encodeShift(kArmLsr, 32 - k));
            opRegRegImm(cUnit, kOpAsr, rlResult.lowReg, tReg, k);
        } else {
            opRegRegImm(cUnit, kOpAsr, tReg, rlSrc.lowReg, 31);
            opRegRegRegShift(cUnit, kOpAdd, tReg, rlSrc.lowReg, tReg, encodeShift(kArmLsr, 32 - k));
            opRegRegImm(cUnit, kOpAsr, rlResult.lowReg, tReg, k);
        }
    } else {
        int cReg = dvmCompilerAllocTemp(cUnit);
        loadConstant(cUnit, cReg, lit - 1);
        int tReg1 = dvmCompilerAllocTemp(cUnit);
        int tReg2 = dvmCompilerAllocTemp(cUnit);

        if (lit == 2) {
            opRegRegImm(cUnit, kOpLsr, tReg1, rlSrc.lowReg, 32 - k);
            opRegRegReg(cUnit, kOpAdd, tReg2, tReg1, rlSrc.lowReg);
            opRegRegReg(cUnit, kOpAnd, tReg2, tReg2, cReg);
            opRegRegReg(cUnit, kOpSub, rlResult.lowReg, tReg2, tReg1);
        } else {
            opRegRegImm(cUnit, kOpAsr, tReg1, rlSrc.lowReg, 31);
            opRegRegImm(cUnit, kOpLsr, tReg1, tReg1, 32 - k);
            opRegRegReg(cUnit, kOpAdd, tReg2, tReg1, rlSrc.lowReg);
            opRegRegReg(cUnit, kOpAnd, tReg2, tReg2, cReg);
            opRegRegReg(cUnit, kOpSub, rlResult.lowReg, tReg2, tReg1);
        }
    }
    storeValue(cUnit, rlDest, rlResult);

    return true;
}

bool genHoistedChecksForCountUpLoopOpt(CompilationUnit *cUnit, MIR *mir)
{
    if (mir->dalvikInsn.arg[4] == (u4)DIV_CONST) {
        /*
         * NOTE: these synthesized blocks don't have ssa names assigned
         * for Dalvik registers.  However, because they dominate the following
         * blocks we can simply use the Dalvik name w/ subscript 0 as the
         * ssa name.
         */
        DecodedInstruction *dInsn = &mir->dalvikInsn;
        const int lenOffset = OFFSETOF_MEMBER(ArrayObject, length);
        const int maxC = dInsn->arg[0];
        int regLength;
        RegLocation rlArray = cUnit->regLocation[mir->dalvikInsn.vA];
        RegLocation rlIdxEnd = cUnit->regLocation[mir->dalvikInsn.vC];

        /* regArray <- arrayRef */
        rlArray = loadValue(cUnit, rlArray, kCoreReg);
        rlIdxEnd = loadValue(cUnit, rlIdxEnd, kCoreReg);
        genRegImmCheck(cUnit, kArmCondEq, rlArray.lowReg, 0, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);

        /* regLength <- len(arrayRef) */
        regLength = dvmCompilerAllocTemp(cUnit);
        loadWordDisp(cUnit, rlArray.lowReg, lenOffset, regLength);

        int delta = maxC;
        /*
         * If the loop end condition is ">=" instead of ">", then the largest value
         * of the index is "endCondition - 1".
         */
        if (dInsn->arg[2] == OP_IF_GE) {
            delta--;
        }

        if (delta) {
            int tReg = dvmCompilerAllocTemp(cUnit);
            opRegRegImm(cUnit, kOpAdd, tReg, rlIdxEnd.lowReg, delta);
            rlIdxEnd.lowReg = tReg;
            dvmCompilerFreeTemp(cUnit, tReg);
        }
        /* Punt if "regIdxEnd < len(Array)" is false */
        genRegRegCheck(cUnit, kArmCondGe, rlIdxEnd.lowReg, regLength, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);
    } else {
        /*
         * NOTE: these synthesized blocks don't have ssa names assigned
         * for Dalvik registers.  However, because they dominate the following
         * blocks we can simply use the Dalvik name w/ subscript 0 as the
         * ssa name.
         */
        DecodedInstruction *dInsn = &mir->dalvikInsn;
        const int lenOffset = OFFSETOF_MEMBER(ArrayObject, length);
        const int maxC = dInsn->arg[0];
        int regLength;
        RegLocation rlArray = cUnit->regLocation[mir->dalvikInsn.vA];
        RegLocation rlIdxEnd = cUnit->regLocation[mir->dalvikInsn.vC];
        RegLocation rlConstReg = cUnit->regLocation[DECODE_REG(dvmConvertSSARegToDalvik(cUnit, mir->dalvikInsn.arg[4]))];
        /* regArray <- arrayRef */
        rlArray = loadValue(cUnit, rlArray, kCoreReg);
        rlIdxEnd = loadValue(cUnit, rlIdxEnd, kCoreReg);
        rlConstReg = loadValue(cUnit, rlConstReg, kCoreReg);
        genRegImmCheck(cUnit, kArmCondEq, rlArray.lowReg, 0, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);

        /* regLength <- len(arrayRef) */
        regLength = dvmCompilerAllocTemp(cUnit);
        loadWordDisp(cUnit, rlArray.lowReg, lenOffset, regLength);

        int delta = maxC;
        /*
         * If the loop end condition is ">=" instead of ">", then the largest value
         * of the index is "endCondition - 1".
         */
        if (dInsn->arg[2] == OP_IF_GE) {
            delta--;
        }

        if (delta) {
            int tReg = dvmCompilerAllocTemp(cUnit);
            opRegRegImm(cUnit, kOpAdd, tReg, rlIdxEnd.lowReg, delta);
            opRegRegReg(cUnit, kOpAdd, tReg, tReg, rlConstReg.lowReg);
            rlIdxEnd.lowReg = tReg;
            dvmCompilerFreeTemp(cUnit, tReg);
        } else {
            int tReg = dvmCompilerAllocTemp(cUnit);
            opRegRegReg(cUnit, kOpAdd, tReg, rlIdxEnd.lowReg, rlConstReg.lowReg);
            rlIdxEnd.lowReg = tReg;
            dvmCompilerFreeTemp(cUnit, tReg);
        }
        /* Punt if "regIdxEnd < len(Array)" is false */
        genRegRegCheck(cUnit, kArmCondGe, rlIdxEnd.lowReg, regLength, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);
    }
    return true;
}

bool genHoistedChecksForCountDownLoopOpt(CompilationUnit *cUnit, MIR *mir)
{
    if (mir->dalvikInsn.arg[4] == (u4)DIV_CONST) {
        DecodedInstruction *dInsn = &mir->dalvikInsn;
        const int lenOffset = OFFSETOF_MEMBER(ArrayObject, length);
        const int regLength = dvmCompilerAllocTemp(cUnit);
        const int maxC = dInsn->arg[0];
        RegLocation rlArray = cUnit->regLocation[mir->dalvikInsn.vA];
        RegLocation rlIdxInit = cUnit->regLocation[mir->dalvikInsn.vB];

        /* regArray <- arrayRef */
        rlArray = loadValue(cUnit, rlArray, kCoreReg);
        rlIdxInit = loadValue(cUnit, rlIdxInit, kCoreReg);
        genRegImmCheck(cUnit, kArmCondEq, rlArray.lowReg, 0, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);

        /* regLength <- len(arrayRef) */
        loadWordDisp(cUnit, rlArray.lowReg, lenOffset, regLength);

        if (maxC) {
            int tReg = dvmCompilerAllocTemp(cUnit);
            opRegRegImm(cUnit, kOpAdd, tReg, rlIdxInit.lowReg, maxC);
            rlIdxInit.lowReg = tReg;
            dvmCompilerFreeTemp(cUnit, tReg);
        }

        /* Punt if "regIdxInit < len(Array)" is false */
        genRegRegCheck(cUnit, kArmCondGe, rlIdxInit.lowReg, regLength, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);
    } else {
        DecodedInstruction *dInsn = &mir->dalvikInsn;
        const int lenOffset = OFFSETOF_MEMBER(ArrayObject, length);
        const int regLength = dvmCompilerAllocTemp(cUnit);
        RegLocation rlArray = cUnit->regLocation[mir->dalvikInsn.vA];
        RegLocation rlIdxInit = cUnit->regLocation[mir->dalvikInsn.vB];
        RegLocation rlConstReg = cUnit->regLocation[DECODE_REG(dvmConvertSSARegToDalvik(cUnit, mir->dalvikInsn.arg[4]))];

        /* regArray <- arrayRef */
        rlArray = loadValue(cUnit, rlArray, kCoreReg);
        rlIdxInit = loadValue(cUnit, rlIdxInit, kCoreReg);
        rlConstReg = loadValue(cUnit, rlConstReg, kCoreReg);
        genRegImmCheck(cUnit, kArmCondEq, rlArray.lowReg, 0, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);

        /* regLength <- len(arrayRef) */
        loadWordDisp(cUnit, rlArray.lowReg, lenOffset, regLength);

        int tReg = dvmCompilerAllocTemp(cUnit);
        opRegRegReg(cUnit, kOpAdd, tReg, rlIdxInit.lowReg, rlConstReg.lowReg);
        rlIdxInit.lowReg = tReg;
        dvmCompilerFreeTemp(cUnit, tReg);

        /* Punt if "regIdxInit < len(Array)" is false */
        genRegRegCheck(cUnit, kArmCondGe, rlIdxInit.lowReg, regLength, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);
    }
    return true;
}

bool genHoistedLowerBoundCheckOpt(CompilationUnit *cUnit, MIR *mir)
{
    if (mir->dalvikInsn.arg[4] == (u4)DIV_CONST) {
        DecodedInstruction *dInsn = &mir->dalvikInsn;
        const int minC = dInsn->vB;
        RegLocation rlIdx = cUnit->regLocation[mir->dalvikInsn.vA];

        /* regIdx <- initial index value */
        rlIdx = loadValue(cUnit, rlIdx, kCoreReg);

        /* Punt if "regIdxInit + minC >= 0" is false */
        genRegImmCheck(cUnit, kArmCondLt, rlIdx.lowReg, -minC, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);
    } else {
        DecodedInstruction *dInsn = &mir->dalvikInsn;
        const int minC = dInsn->vB;
        RegLocation rlIdx = cUnit->regLocation[mir->dalvikInsn.vA];
        RegLocation rlConstReg = cUnit->regLocation[DECODE_REG(dvmConvertSSARegToDalvik(cUnit, mir->dalvikInsn.arg[4]))];
        rlConstReg = loadValue(cUnit, rlConstReg, kCoreReg);
        /* regIdx <- initial index value */
        rlIdx = loadValue(cUnit, rlIdx, kCoreReg);
        int tReg = dvmCompilerAllocTemp(cUnit);
        opRegRegReg(cUnit, kOpAdd, tReg, rlIdx.lowReg, rlConstReg.lowReg);
        dvmCompilerFreeTemp(cUnit, tReg);

        /* Punt if "regIdxInit + minC >= 0" is false */
        genRegImmCheck(cUnit, kArmCondLt, tReg, -minC, 0,
                       (ArmLIR *) cUnit->loopAnalysis->branchToPCR);
    }
    return true;
}
