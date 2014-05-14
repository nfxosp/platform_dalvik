#include "PatternMatchingOptimizer.h"
#include "PatternMatchingSIMDOptimizer.cpp"

static const unsigned int S1Block1LIRSize = 28;
static LIRInstruction S1Block1LIRIns[S1Block1LIRSize] = {
    {kThumbLdrRRI5, {0, 5, 3, 0}},
    {kThumb2Vldrs, {50, 5, 2}},
    {kThumb2Cbz, {0, 45, 0}},
    {kThumb2Vldrd, {118, 5, 10}},
    {kThumb2Vldrd, {112, 0, 66}},
    {kThumb2VcvtID, {116, 50, 0}},
    {kThumb2Vdivd, {116, 118, 116}},
    {kThumb2Fmrs, {2, 50, 0}},
    {kThumbAddRRI3, {3, 2, 2}},
    {kThumb2Fmsr, {56, 3, 0}},
    {kThumb2VcvtID, {122, 56, 0}},
    {kThumb2Vdivd, {122, 118, 122}},
    {kThumb2Vsubd, {116, 116, 122}},
    {kThumb2Vaddd, {112, 112, 116}},
    {kThumbLdrRRI5, {7, 5, 12}},
    {kThumb2Vstrd, {112, 0, 66}},
    {kThumbAddRRI3, {2, 2, 4}},
    {kThumbStrRRI5, {2, 5, 2}},
    {kThumb2Vstrd, {112, 5, 4}},
    {kThumb2Vstrd, {116, 5, 6}},
    {kThumb2Vstrd, {122, 5, 8}},
    {kThumb2Cbz, {7, 10, 0}},
    {kThumbLdrRRI5, {0, 7, 20}},
    {kThumbStrRRI5, {0, 5, 3}},
    {kThumb2Cbz, {0, 10, 0}},
    {kThumbLdrRRI5, {0, 0, 20}},
    {kThumbStrRRI5, {0, 5, 3}},
    {kThumbBUncond, {10, 0, 0}}
};

/* For S1Block1 optimization */
static void applyS1Block1PatternMatchingOptimization(CompilationUnit *cUnit)
{
    PatternMatchingOptLIRList branchList;
    ArmLIR                   *thisLIR = NULL;
    unsigned int              instPos = 0;
    unsigned int              branchCount = 0;

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyS1Block1PatternMatchingOptimization start");
    }

    memset(&branchList, 0, sizeof(branchList));

    for (ArmLIR *thisLIR = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn;
         thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        if (EncodingMap[thisLIR->opcode].flags & IS_BRANCH) {
            branchList.inst[branchList.instCount] = thisLIR;
            branchList.position[branchList.instCount] = branchList.instCount;
            branchList.instCount++;
        }

        if (thisLIR == (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn)
            break;
    }

    if (branchList.instCount != 4)
        return;

    for (ArmLIR *thisLIR = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn;
         thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        if (instPos < S1Block1LIRSize) {
            thisLIR->opcode = S1Block1LIRIns[instPos].opcode;
            thisLIR->operands[0] = S1Block1LIRIns[instPos].operands[0];
            thisLIR->operands[1] = S1Block1LIRIns[instPos].operands[1];
            thisLIR->operands[2] = S1Block1LIRIns[instPos++].operands[2];
            if (EncodingMap[thisLIR->opcode].flags & IS_BRANCH)
                thisLIR->generic.target = branchList.inst[branchCount++]->generic.target;
        } else {
            thisLIR->flags.isNop = true;
        }

        if (thisLIR == (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn)
            break;
    }

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyS1Block1PatternMatchingOptimization exit");
    }
}

/* For Q1Block1 optimization(Q1 INT) */
static void applyQ1Block1PatternMatchingOptimization(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block1PatternMatchingOptimization start");
    }

    for (int i = 0; i < 70; i++) {
        ArmLIR *simdLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
        simdLir->opcode = kThumbMovRR;
        simdLir->operands[0] = 0;
        simdLir->operands[1] = 0;
        dvmCompilerInsertLIRAfter(
            cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)simdLir);
    }

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block1PatternMatchingOptimization exit");
    }

    return;
}

/* For Q1Block2 optimization(Q1 SHORT) */
static void applyQ1Block2PatternMatchingOptimization(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block2PatternMatchingOptimization start");
    }

    for (int i = 0; i < 84; i++) {
        ArmLIR *simdLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
        simdLir->opcode = kThumbMovRR;
        simdLir->operands[0] = 0;
        simdLir->operands[1] = 0;
        dvmCompilerInsertLIRAfter(
            cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)simdLir);
    }

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block2PatternMatchingOptimization exit");
    }

    return;
}

/* For Q1Block3 optimization(Q1 LONG) */
static void applyQ1Block3PatternMatchingOptimization(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block3PatternMatchingOptimization start");
    }

    ArmLIR *hoistingSlot = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *hoistingSlot = *((ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn);
    hoistingSlot->opcode = kArmPseudoBackEndOptTargetLabel;
    dvmCompilerInsertLIRAfter(
        cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)hoistingSlot);

    ArmLIR *sinkingSlot = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn->prev->target;
    ArmLIR *exceptionSlot = NULL;
    for (ArmLIR *thisLIR = (ArmLIR *)hoistingSlot;
     thisLIR != (ArmLIR *)cUnit->lastLIRInsn; thisLIR = NEXT_LIR(thisLIR)) {
        /* bne     0x5eff715c (L0x5efcc76c) [14, 0, 1, 0] */
        if ((thisLIR->opcode == kThumbBCond) && (thisLIR->operands[1] == 1)) {
            exceptionSlot = (ArmLIR *)thisLIR->generic.target;
            break;
        }
    }

    /* Add instructions to hoisting slot */
    ArmLIR *newLir = NULL;

    /*
    Hoisting Slot
    0x5ef5d504 (0010): ldr     r3, [r5, #0] [32, 3, 5, 0]
    0x5ef5d506 (0012): ldr     r4, [r5, #4] [32, 4, 5, 1]
    0x5ef5d508 (0014): ldr     r9, [r5, #8] [96, 9, 5, 8]
    0x5ef5d522 (002e): ldr     r10, [r5, #12] [96, 10, 5, 12]
    0x5ef5d51c (0028): movs    r7, #1 [46, 7, 1, 0]
    0x5ef5d51e (002a): asr     r8, r7, #31 [150, 8, 7, 31]
    0x5ef5d562 (006e): mov     r11, #59 [93, 11, 59, 0]
    */

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbLdrRRI5;
    newLir->operands[0] = 3;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbLdrRRI5;
    newLir->operands[0] = 4;
    newLir->operands[1] = 5;
    newLir->operands[2] = 1;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2LdrRRI12;
    newLir->operands[0] = 9;
    newLir->operands[1] = 5;
    newLir->operands[2] = 8;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2LdrRRI12;
    newLir->operands[0] = 10;
    newLir->operands[1] = 5;
    newLir->operands[2] = 12;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbMovImm;
    newLir->operands[0] = 7;
    newLir->operands[1] = 1;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2AsrRRI5;
    newLir->operands[0] = 8;
    newLir->operands[1] = 7;
    newLir->operands[2] = 31;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2MovImmShift;
    newLir->operands[0] = 11;
    newLir->operands[1] = 59;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    /*
    Middle Slot
    ldrb    r0, [r6, #42]
    cbnz    r0,Exception
    0x5ef5d50c (0018): mul     r2, r3, r4 [138, 2, 3, 4]
    0x5ef5d510 (001c): umull   r0, r1, r3, r3 [180, 0, 1, 3, 3]
    0x5ef5d514 (0020): adds    r2, r2, r2 [107, 2, 2, 2]
    0x5ef5d518 (0024): adds    r1, r2, r1 [107, 1, 2, 1]
    0x5ef5d526 (0032): adds    r0, r0, r7 [4, 0, 0, 7]
    0x5ef5d528 (0034): adcs    r1, r1, r8 [133, 1, 1, 8]
    0x5ef5d52c (0038): adds    r9, r9, r0 [107, 9, 9, 0]
    0x5ef5d530 (003c): adcs    r10, r10, r1 [133, 10, 10, 1]
    0x5ef5d53e (004a): adcs    r3, r3, r7 [133, 3, 3, 7]
    0x5ef5d53e (004a): adcs    r4, r4, r8 [133, 4, 4, 8]
    0x5ef5d51e (002a): asr     r1, r11, #31 [150, 1, 11, 31]
    0x5ef5d562 (006e): mov     r12, #-1 [0xffffffff] [93, 12, 1023, 0]
    0x5ef5d566 (0072): cmp     r4, r1 [26, 4, 1, 0]
    0x5ef5d56c (0078): blt     0x5ef5d58a (L0x5ef2e3c0) [165, 13, 11, 0]
    0x5ef5d570 (007c): bgt     0x5ef5d586 (L0x5ef2e340) [165, 9, 12, 0]
    0x5ef5d574 (0080): subs    r12, r3, r11 [108, 12, 3, 11]
    0x5ef5d578 (0084): beq     0x5ef5d58a (L0x5ef2e3c0) [165, 7, 0, 0]
    0x5ef5d57c (0088): it:1100  [160, 8, 12, 0]
    0x5ef5d57e (008a): mov     r12, #-1 [0xffffffff] [93, 12, 1023, 0]
    0x5ef5d582 (008e): mov     r12, #1 [0x1] [93, 12, 1, 0]
    L0x5ef2e340:       [132, 0, 0, 0]
    0x5ef5d586 (0092): neg     r12,r12 [141, 12, 12, 0]
    L0x5ef2e3c0:       [132, 0, 0, 0]
    0x5ef5d58a (0096): cmp     r12, #0 [0] [132, 12, 0, 0]
    0x5ef5d592 (009e): bge     0x5ef5d5a0 (L0x5ef2c9dc) [14, 5, 10, 0]
    0x5ef5d594 (00a0): b       0x5ef5d504 (L0x5ef2eebc) [15, -74, 0, 0]
    0x5ef5d596 (00a2): undefined [192, 0, 0, 0]
    */

    int instPos = 0;
    ArmLIR *thisLIR = NULL;

    ArmLIR *lirInnerBranch1 = NULL;
    ArmLIR *lirInnerBranch2 = NULL;
    ArmLIR *lirInnerBranch3 = NULL;
    ArmLIR *lirInnerBranch4 = NULL;
    ArmLIR *lirInnerBranch5 = NULL;
    ArmLIR *lirInnerBranch6 = NULL;
    ArmLIR *lirLabel1 = NULL;
    ArmLIR *lirLabel2 = NULL;

    for (thisLIR = (ArmLIR *)hoistingSlot;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn; thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        instPos++;
        switch (instPos) {
        case 1:
            thisLIR->opcode = kThumb2MulRRR;
            thisLIR->operands[0] = 2;
            thisLIR->operands[1] = 3;
            thisLIR->operands[2] = 4;
            break;
        case 2:
            thisLIR->opcode = kThumb2Umull;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 1;
            thisLIR->operands[2] = 3;
            thisLIR->operands[3] = 3;
            break;
        case 3:
            thisLIR->opcode = kThumb2AddRRR;
            thisLIR->operands[0] = 2;
            thisLIR->operands[1] = 2;
            thisLIR->operands[2] = 2;
            break;
        case 4:
            thisLIR->opcode = kThumb2AddRRR;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 2;
            thisLIR->operands[2] = 1;
            break;
        case 5:
            thisLIR->opcode = kThumbAddRRR;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 7;
            break;
        case 6:
            thisLIR->opcode = kThumb2AdcRRR;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 1;
            thisLIR->operands[2] = 8;
            break;
        case 7:
            thisLIR->opcode = kThumb2AddRRR;
            thisLIR->operands[0] = 9;
            thisLIR->operands[1] = 9;
            thisLIR->operands[2] = 0;
            break;
        case 8:
            thisLIR->opcode = kThumb2AdcRRR;
            thisLIR->operands[0] = 10;
            thisLIR->operands[1] = 10;
            thisLIR->operands[2] = 1;
            break;
        case 9:
            thisLIR->opcode = kThumb2LdrbRRI12;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 6;
            thisLIR->operands[2] = 42;
            break;
        case 10:
            thisLIR->opcode = kThumb2AdcRRR;
            thisLIR->operands[0] = 3;
            thisLIR->operands[1] = 3;
            thisLIR->operands[2] = 7;
            break;
        case 11:
            thisLIR->opcode = kThumb2AdcRRR;
            thisLIR->operands[0] = 4;
            thisLIR->operands[1] = 4;
            thisLIR->operands[2] = 8;
            break;
        case 12:
            thisLIR->opcode = kThumb2AsrRRI5;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 11;
            thisLIR->operands[2] = 31;
            break;
        case 13:
            thisLIR->opcode = kThumb2Cbnz;
            thisLIR->operands[0] = 0;
            lirInnerBranch6 = thisLIR;
            break;
        case 14:
            thisLIR->opcode = kThumb2MovImmShift;
            thisLIR->operands[0] = 12;
            thisLIR->operands[1] = 1023;
            thisLIR->operands[2] = 0;
            break;
        case 15:
            thisLIR->opcode = kThumbCmpRR;
            thisLIR->operands[0] = 4;
            thisLIR->operands[1] = 1;
            thisLIR->operands[2] = 0;
            break;
        case 16:
            thisLIR->opcode = kThumb2BCond;
            thisLIR->operands[0] = 13;
            thisLIR->operands[1] = 11;
            thisLIR->operands[2] = 0;
            lirInnerBranch1 = thisLIR;
            break;
        case 17:
            thisLIR->opcode = kThumb2BCond;
            thisLIR->operands[0] = 9;
            thisLIR->operands[1] = 12;
            thisLIR->operands[2] = 0;
            lirInnerBranch2 = thisLIR;
            break;
        case 18:
            thisLIR->opcode = kThumb2SubRRR;
            thisLIR->operands[0] = 12;
            thisLIR->operands[1] = 3;
            thisLIR->operands[2] = 11;
            break;
        case 19:
            thisLIR->opcode = kThumb2BCond;
            thisLIR->operands[0] = 7;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            lirInnerBranch3 = thisLIR;
            break;
        case 20:
            thisLIR->opcode = kThumb2It;
            thisLIR->operands[0] = 8;
            thisLIR->operands[1] = 12;
            thisLIR->operands[2] = 0;
            break;
        case 21:
            thisLIR->opcode = kThumb2MovImmShift;
            thisLIR->operands[0] = 12;
            thisLIR->operands[1] = 1023;
            thisLIR->operands[2] = 0;
            break;
        case 22:
            thisLIR->opcode = kThumb2MovImmShift;
            thisLIR->operands[0] = 12;
            thisLIR->operands[1] = 1;
            thisLIR->operands[2] = 0;
            break;
        case 23:
            thisLIR->opcode = kArmPseudoBackEndOptTargetLabel;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            lirLabel1 = thisLIR;
            break;
        case 24:
            thisLIR->opcode = kThumb2NegRR;
            thisLIR->operands[0] = 12;
            thisLIR->operands[1] = 12;
            thisLIR->operands[2] = 0;
            break;
        case 25:
            thisLIR->opcode = kArmPseudoBackEndOptTargetLabel;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            lirLabel2 = thisLIR;
            break;
        case 26:
            thisLIR->opcode = kThumb2CmpRI8;
            thisLIR->operands[0] = 12;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        case 27:
            thisLIR->opcode = kThumbBCond;
            thisLIR->operands[0] = 5;
            thisLIR->operands[1] = 11;
            thisLIR->operands[2] = 0;
            lirInnerBranch4 = thisLIR;
            break;
        case 28:
            thisLIR->opcode = kThumbBUncond;
            thisLIR->operands[0] = -74;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            lirInnerBranch5 = thisLIR;
            break;
        case 29:
            thisLIR->opcode = kThumbUndefined;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        default:
            thisLIR->flags.isNop = true;
            break;
        }
        if (cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn == (LIR *)thisLIR)
            break;
    }

    /* Connect Branch & Target */
    lirInnerBranch1->generic.target = (LIR *)lirLabel2;
    lirInnerBranch2->generic.target = (LIR *)lirLabel1;
    lirInnerBranch3->generic.target = (LIR *)lirLabel2;
    lirInnerBranch4->generic.target = (LIR *)hoistingSlot;
    lirInnerBranch5->generic.target = (LIR *)sinkingSlot;
    lirInnerBranch6->generic.target = (LIR *)exceptionSlot;

    /*
    Add instructions to sinking slot
    0x5effacda (0052): str     r4, [r5, #4] [60, 4, 5, 1]
    0x5effacdc (0054): str     r3, [r5, #0] [60, 3, 5, 0]
    0x5effacde (0056): str     r8, [r5, #44] [95, 8, 5, 44]
    0x5efface0 (0058): str     r7, [r5, #40] [60, 7, 5, 10]
    0x5efface2 (005a): str     r10, [r5, #12] [95, 10, 5, 12]
    0x5efface6 (005e): str     r9, [r5, #8] [95, 9, 5, 8]
    0x5effacea (0062): str     r8, [r5, #52] [95, 8, 5, 52]
    0x5effacee (0066): str     r7, [r5, #48] [60, 7, 5, 12]
    */

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 4;
    newLir->operands[1] = 5;
    newLir->operands[2] = 1;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 3;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 8;
    newLir->operands[1] = 5;
    newLir->operands[2] = 44;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 7;
    newLir->operands[1] = 5;
    newLir->operands[2] = 10;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 10;
    newLir->operands[1] = 5;
    newLir->operands[2] = 12;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 9;
    newLir->operands[1] = 5;
    newLir->operands[2] = 8;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 8;
    newLir->operands[1] = 5;
    newLir->operands[2] = 52;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 7;
    newLir->operands[1] = 5;
    newLir->operands[2] = 12;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    /*
    Add instructions to sinking slot
    0x5ef5d546 (0052): str     r4, [r5, #4] [60, 4, 5, 1]
    0x5ef5d548 (0054): str     r3, [r5, #0] [60, 3, 5, 0]
    0x5ef5d54e (005a): str     r10, [r5, #12] [95, 10, 5, 12]
    0x5ef5d552 (005e): str     r9, [r5, #8] [95, 9, 5, 8]
    0x5ef5d556 (0062): str     r8, [r5, #52] [95, 8, 5, 52]
    0x5ef5d55a (0066): str     r7, [r5, #48] [60, 7, 5, 12]
    0x5ef5d568 (0074): str     r1, [r5, #44] [60, 1, 5, 11]
    0x5ef5d58e (009a): str     r12, [r5, #40] [95, 12, 5, 40]
    */

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 4;
    newLir->operands[1] = 5;
    newLir->operands[2] = 1;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 3;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 10;
    newLir->operands[1] = 5;
    newLir->operands[2] = 12;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 9;
    newLir->operands[1] = 5;
    newLir->operands[2] = 8;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 8;
    newLir->operands[1] = 5;
    newLir->operands[2] = 52;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 7;
    newLir->operands[1] = 5;
    newLir->operands[2] = 12;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 1;
    newLir->operands[1] = 5;
    newLir->operands[2] = 11;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2StrRRI12;
    newLir->operands[0] = 12;
    newLir->operands[1] = 5;
    newLir->operands[2] = 40;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block3PatternMatchingOptimization exit");
    }

    return;
}

/* For Q1Block4 optimization(Q1 BYTE) */
static void applyQ1Block4PatternMatchingOptimization(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block4PatternMatchingOptimization start");
    }

    for (int i = 0; i < 89; i++) {
        ArmLIR *simdLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
        simdLir->opcode = kThumbMovRR;
        simdLir->operands[0] = 0;
        simdLir->operands[1] = 0;
        dvmCompilerInsertLIRAfter(
            cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)simdLir);
    }

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block4PatternMatchingOptimization exit");
    }

    return;
}

/* For Q1Block5 optimization(Q1 float) */
static void applyQ1Block5PatternMatchingOptimization(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block5PatternMatchingOptimization start");
    }

    ArmLIR *hoistingSlot = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *hoistingSlot = *((ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn);
    hoistingSlot->opcode = kArmPseudoBackEndOptTargetLabel;
    dvmCompilerInsertLIRAfter(
        cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)hoistingSlot);

    ArmLIR *sinkingSlot = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn->prev->target;

    /* Add instructions to hoisting slot */
    ArmLIR *newLir = NULL;

    /* Find PC Related Load, Exception Slot LIR */
    ArmLIR *pcRelLoadLir = NULL;
    ArmLIR *exceptionSlot = NULL;

    for (ArmLIR *thisLIR = (ArmLIR *)hoistingSlot;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn; thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        if ((thisLIR->opcode == kThumb2Cbnz) && (thisLIR->operands[1] == 0)) {
            exceptionSlot = (ArmLIR *)thisLIR->generic.target;
        }

        if ((thisLIR->opcode == kThumb2Vldrs) && (thisLIR->operands[0] == 49)
             && (thisLIR->operands[1] == 15)) {
            pcRelLoadLir = thisLIR;
            break;
        }
    }

    /*
    Hoisting Slot
    0x5ef62680 (0010): vldr    s16, [r5, #0] [73, 48, 5, 0]
    0x5ef62684 (0014): vldr    s18, [r5, #28] [73, 50, 5, 7]
    0x5ef62688 (0018): vldr    s19, [r5, #8] [73, 51, 5, 2]
    0x5ef626ae (003e): vldr    s20, [r15, #104] [73, 52, 15, 26]
    */
    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vldrs;
    newLir->operands[0] = 48;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vldrs;
    newLir->operands[0] = 50;
    newLir->operands[1] = 5;
    newLir->operands[2] = 7;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vldrs;
    newLir->operands[0] = 51;
    newLir->operands[1] = 5;
    newLir->operands[2] = 2;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *newLir = *pcRelLoadLir;
    newLir->operands[0] = 52;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    /*
    Middle Slot
    0x5ef6268c (001c): vmuls   s17, s16, s16 [75, 49, 48, 48]
    0x5ef62690 (0020): vadd    s17, s17, s18 [81, 49, 49, 50]
    0x5ef62698 (0028): vadd    s19, s19, s17 [81, 51, 51, 49]
    0x5ef6269c (002c): vadd    s16, s16, s18 [81, 48, 48, 50]
    0x5ef626b2 (0042): movs    r1, #1 [46, 1, 1, 0]
    0x5ef626b4 (0044): vcmp.f32s16, s20 [163, 48, 52, 0]
    0x5ef626b8 (0048): fmstat   [161, 0, 0, 0]
    0x5ef626c0 (0050): it:1000 mi [160, 4, 8, 0]
    0x5ef626c2 (0052): mov     r1, #-1 [0xffffffff] [93, 1, 1023, 0]
    0x5ef626c6 (0056): it:1000 eq [160, 0, 8, 0]
    0x5ef626c8 (0058): movs    r1, #0 [46, 1, 0, 0]
    0x5ef626ca (005a): cmp     r1, #0 [25, 1, 0, 0]
    0x5ef626ce (005e): blt     0x5ef626dc (L0x5f042af4) [14, 5, 11, 0]
    0x5ef626d0 (0060): b       0x5ef62680 (L0x5ef2dc24) [15, -42, 0, 0]
    0x5ef626d2 (0062): undefined [192, 0, 0, 0]
    */
    int instPos = 0;
    ArmLIR *thisLIR = NULL;
    ArmLIR *lirInnerBranch = NULL;

    for (thisLIR = (ArmLIR *)hoistingSlot;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn; thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        instPos++;
        switch (instPos) {
        case 1:
            thisLIR->opcode = kThumb2LdrbRRI12;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 6;
            thisLIR->operands[2] = 42;
            break;
        case 2:
            thisLIR->opcode = kThumb2Vmuls;
            thisLIR->operands[0] = 49;
            thisLIR->operands[1] = 48;
            thisLIR->operands[2] = 48;
            break;
        case 3:
            thisLIR->opcode = kThumb2Vadds;
            thisLIR->operands[0] = 49;
            thisLIR->operands[1] = 49;
            thisLIR->operands[2] = 50;
            break;
        case 4:
            thisLIR->opcode = kThumb2Vadds;
            thisLIR->operands[0] = 51;
            thisLIR->operands[1] = 51;
            thisLIR->operands[2] = 49;
            break;
        case 5:
            thisLIR->opcode = kThumb2Vadds;
            thisLIR->operands[0] = 48;
            thisLIR->operands[1] = 48;
            thisLIR->operands[2] = 50;
            break;
        case 6:
            thisLIR->opcode = kThumb2Cbnz;
            thisLIR->operands[0] = 0;
            lirInnerBranch = thisLIR;
            break;
        case 7:
            thisLIR->opcode = kThumbMovImm;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 1;
            thisLIR->operands[2] = 0;
            break;
        case 8:
            thisLIR->opcode = kThumb2Vcmps;
            thisLIR->operands[0] = 48;
            thisLIR->operands[1] = 52;
            thisLIR->operands[2] = 0;
            break;
        case 9:
            thisLIR->opcode = kThumb2Fmstat;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        case 10:
            thisLIR->opcode = kThumb2It;
            thisLIR->operands[0] = 4;
            thisLIR->operands[1] = 8;
            thisLIR->operands[2] = 0;
            break;
        case 11:
            thisLIR->opcode = kThumb2MovImmShift;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 1023;
            thisLIR->operands[2] = 0;
            break;
        case 12:
            thisLIR->opcode = kThumb2It;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 8;
            thisLIR->operands[2] = 0;
            break;
        case 13:
            thisLIR->opcode = kThumbMovImm;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        case 14:
            thisLIR->opcode = kThumbCmpRI8;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        case 15:
            thisLIR->opcode = kThumbBCond;
            thisLIR->operands[0] = 5;
            thisLIR->operands[1] = 11;
            thisLIR->operands[2] = 0;
            thisLIR->generic.target = (LIR *)hoistingSlot;
            break;
        case 16:
            thisLIR->opcode = kThumbBUncond;
            thisLIR->operands[0] = -42;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            thisLIR->generic.target = (LIR *)sinkingSlot;
            break;
        case 17:
            thisLIR->opcode = kThumbUndefined;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        default:
            thisLIR->flags.isNop = true;
            break;
        }
        if (cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn == (LIR *)thisLIR)
            break;
    }

    lirInnerBranch->generic.target = (LIR *)exceptionSlot;

    /*
    Add instructions to exception slot
    0x5efbbeec (0030): vstr    s16, [r5, #0] [77, 48, 5, 0]
    0x5efbbef0 (0034): vstr    s19, [r5, #8] [77, 51, 5, 2]
    0x5efbbef4 (0038): vstr    s17, [r5, #24] [77, 49, 5, 6]
    */
    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrs;
    newLir->operands[0] = 48;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrs;
    newLir->operands[0] = 51;
    newLir->operands[1] = 5;
    newLir->operands[2] = 2;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrs;
    newLir->operands[0] = 49;
    newLir->operands[1] = 5;
    newLir->operands[2] = 6;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    /*
    Add instructions to sinking slot
    0x5ef626a0 (0030): vstr    s16, [r5, #0] [77, 48, 5, 0]
    0x5ef626a4 (0034): vstr    s19, [r5, #8] [77, 51, 5, 2]
    0x5ef626cc (005c): str     r1, [r5, #24] [60, 1, 5, 6]
    */
    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrs;
    newLir->operands[0] = 48;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrs;
    newLir->operands[0] = 51;
    newLir->operands[1] = 5;
    newLir->operands[2] = 2;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 1;
    newLir->operands[1] = 5;
    newLir->operands[2] = 6;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block5PatternMatchingOptimization exit");
    }

    return;
}

/* For Q1Block6 optimization(Q1 double) */
static void applyQ1Block6PatternMatchingOptimization(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block5PatternMatchingOptimization start");
    }

    ArmLIR *hoistingSlot = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *hoistingSlot = *((ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn);
    hoistingSlot->opcode = kArmPseudoBackEndOptTargetLabel;
    dvmCompilerInsertLIRAfter(
        cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)hoistingSlot);

    ArmLIR *sinkingSlot = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn->prev->target;

    /* Add instructions to hoisting slot */
    ArmLIR *newLir = NULL;

    /* Find PC Related Load, Exception Slot LIR */
    ArmLIR *pcRelLoadLir1 = NULL;
    ArmLIR *pcRelLoadLir2 = NULL;
    ArmLIR *pcRelLoadLir3 = NULL;
    ArmLIR *exceptionSlot = NULL;

    for (ArmLIR *thisLIR = (ArmLIR *)hoistingSlot;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn; thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        if ((thisLIR->opcode == kThumb2Cbnz) && (thisLIR->operands[1] == 0)) {
            exceptionSlot = (ArmLIR *)thisLIR->generic.target;
            continue;
        }

        if ((thisLIR->opcode == kThumb2Vldrd) && (thisLIR->operands[0] == 116)
             && (thisLIR->operands[1] == 15) && (pcRelLoadLir1 == NULL)) {
            pcRelLoadLir1 = thisLIR;
            continue;
        }
        if ((thisLIR->opcode == kThumb2Vldrd) && (thisLIR->operands[0] == 114)
             && (thisLIR->operands[1] == 15) && (pcRelLoadLir2 == NULL)) {
            pcRelLoadLir2 = thisLIR;
            continue;
        }
        if ((thisLIR->opcode == kThumb2Vldrd) && (thisLIR->operands[0] == 114)
             && (thisLIR->operands[1] == 15) && (pcRelLoadLir3 == NULL)) {
            pcRelLoadLir3 = thisLIR;
            break;
        }
    }

    /*
    Hoisting Slot
    0x5ef63e78 (0010): vldr    d8, [r5, #0] [74, 112, 5, 0]
    0x5ef63e7c (0014): vldr    d10, [r15, #160] [74, 116, 15, 40]
    0x5ef63e80 (0018): vldr    d11, [r5, #8] [74, 118, 5, 2]
    0x5ef63e94 (002c): vldr    d12, [r15, #136] [74, 120, 15, 34]
    0x5ef63eae (0046): vldr    d13, [r15, #104] [74, 122, 15, 26]
    */
    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vldrd;
    newLir->operands[0] = 112;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *newLir = *pcRelLoadLir1;
    newLir->operands[0] = 116;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vldrd;
    newLir->operands[0] = 118;
    newLir->operands[1] = 5;
    newLir->operands[2] = 2;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *newLir = *pcRelLoadLir2;
    newLir->operands[0] = 120;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *newLir = *pcRelLoadLir3;
    newLir->operands[0] = 122;
    dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)newLir);

    /*
    Middle Slot
    ldrb    r0, [r6, #42] [126, 0, 6, 42]
    0x5ef63e84 (001c): vmuld   d9, d8, d8 [76, 114, 112, 112]
    0x5ef63e88 (0020): vadd    d9, d9, d10 [82, 114, 114, 116]
    0x5ef63e8c (0024): vadd    d11, d11, d9 [82, 118, 118, 114]
    0x5ef63e98 (0030): vadd    d8, d8, d12 [82, 112, 112, 120]
    cbnz    r0,0x5efc8fcc (L0x5f1df294) [99, 0, 18, 0]
    0x5ef63eb2 (004a): movs    r1, #1 [46, 1, 1, 0]
    0x5ef63eb4 (004c): vcmp.f64d8, d13 [162, 112, 122, 0]
    0x5ef63eb8 (0050): fmstat   [161, 0, 0, 0]
    0x5ef63ec0 (0058): it:1000 mi [160, 4, 8, 0]
    0x5ef63ec2 (005a): mov     r1, #-1 [0xffffffff] [93, 1, 1023, 0]
    0x5ef63ec6 (005e): it:1000 eq [160, 0, 8, 0]
    0x5ef63ec8 (0060): movs    r1, #0 [46, 1, 0, 0]
    0x5ef63eca (0062): cmp     r1, #0 [25, 1, 0, 0]
    0x5ef63ece (0066): blt     0x5ef63edc (L0x5ef2c9dc) [14, 5, 11, 0]
    0x5ef63ed0 (0068): b       0x5ef63e78 (L0x5ef2e7bc) [15, -46, 0, 0]
    0x5ef63ed2 (006a): undefined [192, 0, 0, 0]
    */
    int instPos = 0;
    ArmLIR *thisLIR = NULL;
    ArmLIR *lirInnerBranch = NULL;

    for (thisLIR = (ArmLIR *)hoistingSlot;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn; thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        instPos++;
        switch (instPos) {
        case 1:
            thisLIR->opcode = kThumb2LdrbRRI12;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 6;
            thisLIR->operands[2] = 42;
            break;
        case 2:
            thisLIR->opcode = kThumb2Vmuld;
            thisLIR->operands[0] = 114;
            thisLIR->operands[1] = 112;
            thisLIR->operands[2] = 112;
            break;
        case 3:
            thisLIR->opcode = kThumb2Vaddd;
            thisLIR->operands[0] = 114;
            thisLIR->operands[1] = 114;
            thisLIR->operands[2] = 116;
            break;
        case 4:
            thisLIR->opcode = kThumb2Vaddd;
            thisLIR->operands[0] = 118;
            thisLIR->operands[1] = 118;
            thisLIR->operands[2] = 114;
            break;
        case 5:
            thisLIR->opcode = kThumb2Vaddd;
            thisLIR->operands[0] = 112;
            thisLIR->operands[1] = 112;
            thisLIR->operands[2] = 120;
            break;
        case 6:
            thisLIR->opcode = kThumb2Cbnz;
            thisLIR->operands[0] = 0;
            lirInnerBranch = thisLIR;
            break;
        case 7:
            thisLIR->opcode = kThumbMovImm;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 1;
            thisLIR->operands[2] = 0;
            break;
        case 8:
            thisLIR->opcode = kThumb2Vcmpd;
            thisLIR->operands[0] = 112;
            thisLIR->operands[1] = 122;
            thisLIR->operands[2] = 0;
            break;
        case 9:
            thisLIR->opcode = kThumb2Fmstat;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        case 10:
            thisLIR->opcode = kThumb2It;
            thisLIR->operands[0] = 4;
            thisLIR->operands[1] = 8;
            thisLIR->operands[2] = 0;
            break;
        case 11:
            thisLIR->opcode = kThumb2MovImmShift;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 1023;
            thisLIR->operands[2] = 0;
            break;
        case 12:
            thisLIR->opcode = kThumb2It;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 8;
            thisLIR->operands[2] = 0;
            break;
        case 13:
            thisLIR->opcode = kThumbMovImm;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        case 14:
            thisLIR->opcode = kThumbCmpRI8;
            thisLIR->operands[0] = 1;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;
        case 15:
            thisLIR->opcode = kThumbBCond;
            thisLIR->operands[0] = 5;
            thisLIR->operands[1] = 11;
            thisLIR->operands[2] = 0;
            thisLIR->generic.target = (LIR *)hoistingSlot;
            break;
        case 16:
            thisLIR->opcode = kThumbBUncond;
            thisLIR->operands[0] = -46;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            thisLIR->generic.target = (LIR *)sinkingSlot;
            break;
        case 17:
            thisLIR->opcode = kThumbUndefined;
            thisLIR->operands[0] = 0;
            thisLIR->operands[1] = 0;
            thisLIR->operands[2] = 0;
            break;

        default:
            thisLIR->flags.isNop = true;
            break;
        }
        if (cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn == (LIR *)thisLIR)
            break;
    }

    lirInnerBranch->generic.target = (LIR *)exceptionSlot;

    /*
    Add instructions to exception slot
    0x5efc8f94 (0034): vstr    d8, [r5, #0] [78, 112, 5, 0]
    0x5efc8f98 (0038): vstr    d12, [r5, #40] [78, 120, 5, 10]
    0x5efc8f9c (003c): vstr    d11, [r5, #8] [78, 118, 5, 2]
    0x5efc8fa0 (0040): vstr    d10, [r5, #48] [78, 116, 5, 12]
    */
    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 112;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 120;
    newLir->operands[1] = 5;
    newLir->operands[2] = 10;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 118;
    newLir->operands[1] = 5;
    newLir->operands[2] = 2;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 116;
    newLir->operands[1] = 5;
    newLir->operands[2] = 12;
    dvmCompilerInsertLIRAfter((LIR *)exceptionSlot, (LIR *)newLir);

    /*
    Add instructions to sinking slot
    0x5ef63e9c (0034): vstr    d8, [r5, #0] [78, 112, 5, 0]
    0x5ef63ea4 (003c): vstr    d11, [r5, #8] [78, 118, 5, 2]
    0x5ef63ea8 (0040): vstr    d10, [r5, #48] [78, 116, 5, 12]
    0x5ef63ebc (0054): vstr    d13, [r5, #40] [78, 122, 5, 10]
    0x5ef63ecc (0064): str     r1, [r5, #40] [60, 1, 5, 10]
    */
    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumbStrRRI5;
    newLir->operands[0] = 1;
    newLir->operands[1] = 5;
    newLir->operands[2] = 10;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 112;
    newLir->operands[1] = 5;
    newLir->operands[2] = 0;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 118;
    newLir->operands[1] = 5;
    newLir->operands[2] = 2;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 116;
    newLir->operands[1] = 5;
    newLir->operands[2] = 12;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    newLir = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    newLir->opcode = kThumb2Vstrd;
    newLir->operands[0] = 122;
    newLir->operands[1] = 5;
    newLir->operands[2] = 10;
    dvmCompilerInsertLIRAfter((LIR *)sinkingSlot, (LIR *)newLir);

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyQ1Block6PatternMatchingOptimization exit");
    }

    return;
}

static const unsigned int L1Block1LIRHoistingSlotSize = 7;
static LIRInstruction L1Block1LIRHoistingIns[L1Block1LIRHoistingSlotSize] = {
    {kThumbLdrRRI5, {4, 5, 2, 0}},
    {kThumbLdrRRI5, {7, 5, 18, 0}},
    {kThumb2LdrRRI12, {11, 5, 60, 0}},
    {kThumbLdrRRI5, {0, 5, 14, 0}},
    {kThumb2LdrRRI12, {9, 5, 68, 0}},
    {kThumbLdrRRI5, {2, 5, 11, 0}},
    {kThumb2Vldrd, {116, 5, 12, 0}},
};

static const unsigned int L1Block1LIRBodySize = 17;
static LIRInstruction L1Block1LIRBodyIns[L1Block1LIRBodySize] = {
    {kThumb2AddRRR, {8, 4, 7, 0}},
    {kThumb2AddRRR, {10, 9, 8, 12}},
    {kThumb2Vldrd, {112, 10, 4, 0}},
    {kThumb2Pld, {10, 128, 0, 0}},
    {kThumb2AddRRR, {12, 4, 11, 0}},
    {kThumb2AddRRR, {1, 0, 12, 12}},
    {kThumb2Vldrd, {114, 1, 4, 0}},
    {kThumb2Pld, {1, 128, 0, 0}},
    {kThumb2Vmuld, {114, 114, 116, 0}},
    {kThumb2Vaddd, {112, 112, 114, 0}},
    {kThumb2LdrbRRI12, {3, 6, 42, 0}},
    {kThumb2Vstrd, {112, 10, 4, 0}},
    {kThumbAddRRI3,  {4, 4, 1, 0}},
    {kThumb2Cbnz, {3, 0, 0, 0}},
    {kThumbCmpRR, {4, 2, 0, 0}},
    {kThumb2BCond, {0, 11, 0, 0}},
    {kThumbBUncond,  {0, 0, 0, 0}}
};

static const unsigned int L1Block1LIRSinkingSlotSize = 4;
static LIRInstruction L1Block1LIRSinkingIns[L1Block1LIRSinkingSlotSize] = {
    {kThumb2StrRRI12, {8, 5, 20, 0}},
    {kThumb2Vstrd, {112, 5, 6, 0}},
    {kThumb2StrRRI12, {12, 5, 32, 0}},
    {kThumbStrRRI5, {4, 5, 2, 0}},
};

/* For L1Block1 optimization */
static void applyL1Block1PatternMatchingOptimization(CompilationUnit *cUnit)
{
    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyL1Block6PatternMatchingOptimization start");
    }

    /* 1. LICM */
    unsigned int instPos = 0;
    ArmLIR *hoistingSlot = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *hoistingSlot = *((ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn);
    hoistingSlot->opcode = kArmPseudoBackEndOptTargetLabel;
    dvmCompilerInsertLIRAfter(cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)hoistingSlot);

    ArmLIR *sinkingSlotNormal = (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn->prev->target;
    ArmLIR *sinkingSlotPCRecon = NULL;

    ArmLIR *thisLIR = NULL;

    for (thisLIR = (ArmLIR *)hoistingSlot;
         thisLIR != (ArmLIR *)cUnit->lastLIRInsn;
         thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;
        if (thisLIR->opcode == kThumb2Cbnz) {
            sinkingSlotPCRecon = (ArmLIR *)thisLIR->generic.target;
            break;
        }
    }

    /* 1.1 ldr hoisting */
    for(instPos = 0; instPos < L1Block1LIRHoistingSlotSize; instPos++) {
        thisLIR = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
        thisLIR->opcode = L1Block1LIRHoistingIns[instPos].opcode;
        thisLIR->operands[0] = L1Block1LIRHoistingIns[instPos].operands[0];
        thisLIR->operands[1] = L1Block1LIRHoistingIns[instPos].operands[1];
        thisLIR->operands[2] = L1Block1LIRHoistingIns[instPos].operands[2];
        dvmCompilerInsertLIRBefore((LIR *)hoistingSlot, (LIR *)thisLIR);
    }

    /* 1.2 Body */
    instPos = 0;

    for (thisLIR = (ArmLIR *)hoistingSlot;
         thisLIR != (ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn->next;
         thisLIR = NEXT_LIR(thisLIR)) {
        if (thisLIR->flags.isNop || isPseudoOpcode(thisLIR->opcode))
            continue;

        if (instPos < L1Block1LIRBodySize) {
            thisLIR->opcode = L1Block1LIRBodyIns[instPos].opcode;
            thisLIR->operands[0] = L1Block1LIRBodyIns[instPos].operands[0];
            thisLIR->operands[1] = L1Block1LIRBodyIns[instPos].operands[1];
            thisLIR->operands[2] = L1Block1LIRBodyIns[instPos].operands[2];
            thisLIR->operands[3] = L1Block1LIRBodyIns[instPos++].operands[3];
            /* cbnz r3,0x5d5dde68 (l0x5db9c5f8) */
            if (thisLIR->opcode == kThumb2Cbnz)
                thisLIR->generic.target = (LIR *)sinkingSlotPCRecon;
            /* Exit condition negated branch (loop-inversion) */
            /* blt 0x5d5dde24 (l0x5db9d038) */
            if (thisLIR->opcode == kThumb2BCond)
                thisLIR->generic.target = (LIR *)hoistingSlot;
            /* b   0x5d5dde84 (l0x5d6dc778) */
            if (thisLIR->opcode == kThumbBUncond)
                thisLIR->generic.target = (LIR *)sinkingSlotNormal;
        } else {
            thisLIR->flags.isNop = true;
        }
    }

    /* 1.3 str sinking */
    for(instPos = 0; instPos < L1Block1LIRSinkingSlotSize; instPos++) {
        thisLIR = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
        thisLIR->opcode = L1Block1LIRSinkingIns[instPos].opcode;
        thisLIR->operands[0] = L1Block1LIRSinkingIns[instPos].operands[0];
        thisLIR->operands[1] = L1Block1LIRSinkingIns[instPos].operands[1];
        thisLIR->operands[2] = L1Block1LIRSinkingIns[instPos].operands[2];
        dvmCompilerInsertLIRAfter((LIR *)sinkingSlotNormal, (LIR *)thisLIR);
        thisLIR = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
        thisLIR->opcode = L1Block1LIRSinkingIns[instPos].opcode;
        thisLIR->operands[0] = L1Block1LIRSinkingIns[instPos].operands[0];
        thisLIR->operands[1] = L1Block1LIRSinkingIns[instPos].operands[1];
        thisLIR->operands[2] = L1Block1LIRSinkingIns[instPos].operands[2];
        dvmCompilerInsertLIRAfter((LIR *)sinkingSlotPCRecon, (LIR *)thisLIR);
    }

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE]     applyL1Block6PatternMatchingOptimization exit");
    }
}
