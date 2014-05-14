#define PATTERNMATCHING_OPT_RANGE 200

typedef struct _LIRInstruction {
    ArmOpcode opcode;
    int       operands[4];
} LIRInstruction;

typedef struct _PatternMatchingOptLIRList {
    ArmLIR *inst[PATTERNMATCHING_OPT_RANGE];
    u1      position[PATTERNMATCHING_OPT_RANGE];
    int     instCount;
} PatternMatchingOptLIRList;