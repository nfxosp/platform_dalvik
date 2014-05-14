#include "PatternMatchingOptimizations.h"

#include "PatternMatchingAnalyzer.cpp"
#include "PatternMatchingOptimizer.cpp"

PatternType pattern;

/* For S1Block1 detection */
static const int S1Block1PerceivedSequence = 2591;
static const unsigned int S1Block1MIRSize = 15;
static MIRInstruction S1Block1MIRIns[S1Block1MIRSize] = {
    {OP_IGET_WIDE_QUICK, 4, 3, 264},
    {OP_INT_TO_DOUBLE, 6, 2, 0},
    {OP_DIV_DOUBLE, 6, 10, 6},
    {OP_ADD_INT_LIT8, 8, 2, 2},
    {OP_INT_TO_DOUBLE, 8, 8, 0},
    {OP_DIV_DOUBLE, 8, 10, 8},
    {OP_SUB_DOUBLE_2ADDR, 6, 8, 0},
    {OP_ADD_DOUBLE_2ADDR, 4, 6, 0},
    {OP_IPUT_WIDE_QUICK , 4, 3, 264},
    {OP_ADD_INT_LIT8, 2, 2, 4},
    {OP_GOTO, (u4)-68, 0, 0},
    {OP_IGET_OBJECT_QUICK, 3, 12, 80},
    {OP_INVOKE_STATIC, 1, 2578, 3},
    {OP_IGET_OBJECT_QUICK, 3, 3, 80},
    {OP_MOVE_RESULT_OBJECT, 3, 0, 0},
};

/* For Q1Block1(int) detection */
static const int Q1Block1PerceivedSequence = 1378;
static const unsigned int Q1Block1MIRSize = 9;
static MIRInstruction Q1Block1MIRIns[Q1Block1MIRSize] = {
    {(Opcode)kMirOpPhi, 2, 0, 0},
    {(Opcode)kMirOpPhi, 0, 0, 0},
    {OP_MUL_INT, 6, 0, 0},
    {OP_ADD_INT_LIT8, 6, 6, 1},
    {OP_ADD_INT_2ADDR, 2, 6, 0},
    {OP_ADD_INT_LIT8, 0, 0, 1},
    {OP_GOTO, (u4)-11, 0, 0},
    {OP_CONST_16, 6, 101, 0},
    {OP_IF_GE, 0, 6, 10}
};

/* For Q1Block2(short) detection */
static const int Q1Block2PerceivedSequence = 1664;
static const unsigned int Q1Block2MIRSize = 11;
static MIRInstruction Q1Block2MIRIns[Q1Block2MIRSize] = {
    {(Opcode)kMirOpPhi, 2, 0, 0},
    {(Opcode)kMirOpPhi, 0, 0, 0},
    {OP_MUL_INT, 6, 0, 0},
    {OP_ADD_INT_LIT8, 6, 6, 1},
    {OP_ADD_INT_2ADDR, 2, 6, 0},
    {OP_INT_TO_SHORT, 2, 2, 0},
    {OP_ADD_INT_LIT8, 0, 0, 1},
    {OP_INT_TO_SHORT, 0, 0, 0},
    {OP_GOTO, (u4)-13, 0, 0},
    {OP_CONST_16, 6, 77, 0},
    {OP_IF_GE, 0, 6, 12}
};

/* For Q1Block3(long) detection */
static const int Q1Block3PerceivedSequence = 1956;
static const unsigned int Q1Block3MIRSize = 14;
static MIRInstruction Q1Block3MIRIns[Q1Block3MIRSize] = {
    {(Opcode)kMirOpPhi, 3, 0, 0},
    {(Opcode)kMirOpPhi, 2, 0, 0},
    {(Opcode)kMirOpPhi, 1, 0, 0},
    {(Opcode)kMirOpPhi, 0, 0, 0},
    {OP_MUL_LONG, 10, 0, 0},
    {OP_CONST_WIDE_16, 12, 1, 0},
    {OP_ADD_LONG_2ADDR, 10, 12, 0},
    {OP_ADD_LONG_2ADDR, 2, 10, 0},
    {OP_CONST_WIDE_16, 10, 1, 0},
    {OP_ADD_LONG_2ADDR, 0, 10, 0},
    {OP_GOTO, (u4)-15, 0, 0},
    {OP_CONST_WIDE_16, 10, 59, 0},
    {OP_CMP_LONG, 10, 0, 10},
    {OP_IF_GEZ, 10, 12, 0}
};

/* For Q1Block4(byte) detection */
static const int Q1Block4PerceivedSequence = 1660;
static const unsigned int Q1Block4MIRSize = 11;
static MIRInstruction Q1Block4MIRIns[Q1Block4MIRSize] = {
    {(Opcode)kMirOpPhi, 2, 0, 0},
    {(Opcode)kMirOpPhi, 0, 0, 0},
    {OP_MUL_INT, 6, 0, 0},
    {OP_ADD_INT_LIT8, 6, 6, 1},
    {OP_ADD_INT_2ADDR, 2, 6, 0},
    {OP_INT_TO_BYTE, 2, 2, 0},
    {OP_ADD_INT_LIT8, 0, 0, 1},
    {OP_INT_TO_BYTE, 0, 0, 0},
    {OP_GOTO, (u4)-13, 0, 0},
    {OP_CONST_16, 6, 78, 0},
    {OP_IF_GE, 0, 6, 12}
};

/* For Q1Block5(float) detection */
static const int Q1Block5PerceivedSequence = 1440;
static const unsigned int Q1Block5MIRSize = 10;
static MIRInstruction Q1Block5MIRIns[Q1Block5MIRSize] = {
    {(Opcode)kMirOpPhi, 2, 0, 0},
    {(Opcode)kMirOpPhi, 0, 0, 0},
    {OP_MUL_FLOAT, 6, 0, 0},
    {OP_ADD_FLOAT_2ADDR, 6, 7, 0},
    {OP_ADD_FLOAT_2ADDR, 2, 6, 0},
    {OP_ADD_FLOAT_2ADDR, 0, 7, 0},
    {OP_GOTO, (u4)-11, 0, 0},
    {OP_CONST_HIGH16, 6, 17038, 0},
    {OP_CMPG_FLOAT, 6, 0, 6},
    {OP_IF_GEZ, 6, 8, 0}
};

/* For Q1Block6(double) detection */
static const int Q1Block6PerceivedSequence = 2028;
static const unsigned int Q1Block6MIRSize = 14;
static MIRInstruction Q1Block6MIRIns[Q1Block6MIRSize] = {
    {(Opcode)kMirOpPhi, 3, 0, 0},
    {(Opcode)kMirOpPhi, 2, 0, 0},
    {(Opcode)kMirOpPhi, 1, 0, 0},
    {(Opcode)kMirOpPhi, 0, 0, 0},
    {OP_MUL_DOUBLE, 10, 0, 0},
    {OP_CONST_WIDE_HIGH16, 12, 16368, 0},
    {OP_ADD_DOUBLE_2ADDR, 10, 12, 0},
    {OP_ADD_DOUBLE_2ADDR, 2, 10, 0},
    {OP_CONST_WIDE_HIGH16, 10, 16368, 0},
    {OP_ADD_DOUBLE_2ADDR, 0, 10, 0},
    {OP_GOTO, (u4)-15, 0, 0},
    {OP_CONST_WIDE_HIGH16, 10, 16452, 0},
    {OP_CMPG_DOUBLE, 10, 0, 10},
    {OP_IF_GEZ, 10, 12, 0}
};

/* For L1Block1 detection */
static const int L1Block1PerceivedSequence = 2507;    //after n&b checks, originally 0x5C3 or 1475
static const unsigned int L1Block1MIRSize = 15;       //after n&b checks, originally 11
static MIRInstruction L1Block1MIRIns[L1Block1MIRSize] = {
    {(Opcode)kMirOpNullNRangeUpCheck, 17, 2, 11},    // 257
    {(Opcode)kMirOpNullNRangeUpCheck, 14, 2, 11},    // 257
    {(Opcode)kMirOpLowerBound, 2, 0, 0},             // 259
    {(Opcode)kMirOpLowerBound, 2, 0, 0},             // 259
    {(Opcode)kMirOpPhi, 2, 0, 0},                    // 0x100
    {OP_ADD_INT, 5, 2, 18},                          // 0x90
    {OP_AGET_WIDE, 6, 17, 5},                        // 0x45
    {OP_ADD_INT, 8, 2, 15},                          // 0x90
    {OP_AGET_WIDE, 8, 14, 8},                        // 0x45
    {OP_MUL_DOUBLE_2ADDR, 8, 12, 0},                 // 0xCD
    {OP_ADD_DOUBLE_2ADDR, 6, 8, 0},                  // 0xCB
    {OP_APUT_WIDE, 6, 17, 5},                        // 0x4C
    {OP_ADD_INT_LIT8, 2, 2, 1},                      // 0xD8
    {OP_GOTO, (u4)-16, 0, 0},                        // 0x28
    {OP_IF_GE, 2, 11, (u4)-21}                       // 0x35
};

/* main function of Pattern Matching */
static bool applyPatternMatchingOptimizations(CompilationUnit *cUnit)
{
    pattern = kNone;
    MIRInstruction        *pBlockMIRInsn = NULL;
    unsigned int           blockSize = 0;

    doPrefilteringForPatternMatchingAnalysis(cUnit);

    switch (cUnit->backEndOptInfo.perceivedSequence) {
    case S1Block1PerceivedSequence:
        pattern = kS1Block1;
        pBlockMIRInsn = S1Block1MIRIns;
        blockSize = S1Block1MIRSize;
        break;
    case Q1Block1PerceivedSequence:
        pattern = kQ1Block1;
        pBlockMIRInsn = Q1Block1MIRIns;
        blockSize = Q1Block1MIRSize;
        break;
    case Q1Block2PerceivedSequence:
        pattern = kQ1Block2;
        pBlockMIRInsn = Q1Block2MIRIns;
        blockSize = Q1Block2MIRSize;
        break;
    case Q1Block3PerceivedSequence:
        pattern = kQ1Block3;
        pBlockMIRInsn = Q1Block3MIRIns;
        blockSize = Q1Block3MIRSize;
        break;
    case Q1Block4PerceivedSequence:
        pattern = kQ1Block4;
        pBlockMIRInsn = Q1Block4MIRIns;
        blockSize = Q1Block4MIRSize;
        break;
#if 0
    /* Disable optimizations on float/double for compatibility issue */
    case Q1Block5PerceivedSequence:
        pattern = kQ1Block5;
        pBlockMIRInsn = Q1Block5MIRIns;
        blockSize = Q1Block5MIRSize;
        break;
    case Q1Block6PerceivedSequence:
        pattern = kQ1Block6;
        pBlockMIRInsn = Q1Block6MIRIns;
        blockSize = Q1Block6MIRSize;
        break;
#endif
    case L1Block1PerceivedSequence:
        pattern = kL1Block1;
        pBlockMIRInsn = L1Block1MIRIns;
        blockSize = L1Block1MIRSize;
        break;
    default:
        return false;
    }

    if (!applyPatternMatchingAnalysis(cUnit, pBlockMIRInsn, blockSize)) {
        pattern = kNone;
        return false;
    }

    if (gDvmJit.jitDebug) {
        ALOGD("[SWE] applyPatternMatchingOptimizations start");

        switch(pattern) {
        case kS1Block1:
            ALOGD("[SWE] - S1Block1 pattern detected");
            break;
        case kQ1Block1:
            ALOGD("[SWE] - Q1Block1 pattern detected");
            break;
        case kQ1Block2:
            ALOGD("[SWE] - Q1Block2 pattern detected");
            break;
        case kQ1Block3:
            ALOGD("[SWE] - Q1Block3 pattern detected");
            break;
        case kQ1Block4:
            ALOGD("[SWE] - Q1Block4 pattern detected");
            break;
        case kQ1Block5:
            ALOGD("[SWE] - Q1Block5 pattern detected");
            break;
        case kQ1Block6:
            ALOGD("[SWE] - Q1Block6 pattern detected");
            break;
        case kL1Block1:
            ALOGD("[SWE] - L1Block1 pattern detected");
            break;
        default:
            break;
        }
    }

    switch (pattern) {
    case kS1Block1:
        applyS1Block1PatternMatchingOptimization(cUnit);
        break;
    case kQ1Block1:
        applyQ1Block1PatternMatchingOptimization(cUnit);
        break;
    case kQ1Block2:
        applyQ1Block2PatternMatchingOptimization(cUnit);
        break;
    case kQ1Block3:
        applyQ1Block3PatternMatchingOptimization(cUnit);
        break;
    case kQ1Block4:
        applyQ1Block4PatternMatchingOptimization(cUnit);
        break;
    case kQ1Block5:
        applyQ1Block5PatternMatchingOptimization(cUnit);
        break;
    case kQ1Block6:
        applyQ1Block6PatternMatchingOptimization(cUnit);
        break;
    case kL1Block1:
        applyL1Block1PatternMatchingOptimization(cUnit);
        break;
    default:
        ALOGW("Optimized for what? (%d)", pattern);
        break;
    }

    if (gDvmJit.jitDebug)
        ALOGD("[SWE] applyPatternMatchingOptimizations exit");

    return true;
}
