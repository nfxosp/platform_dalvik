/*
 *   L1Block1   : Linpack
 *   B1Block1   : BenchmarkPI
 *   S1Block1   : Smartbench
 *   Q1Block1~6 : Quadrant (int, short, long, byte, float, double)
 */
typedef enum _PatternType {
    kNone,
    kL1Block1,
    kB1Block1,
    kS1Block1,
    kQ1Block1,
    kQ1Block2,
    kQ1Block3,
    kQ1Block4,
    kQ1Block5,
    kQ1Block6
} PatternType;

typedef struct _MIRInstruction {
    Opcode  opcode;
    u4      vA;
    u4      vB;
    u4      vC;
} MIRInstruction;

extern PatternType pattern;
