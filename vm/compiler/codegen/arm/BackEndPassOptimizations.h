#ifndef DALVIK_VM_COMPILER_CODEGEN_ARM_BACKENPASSOPTIMIZATIONS_H__
#define DALVIK_VM_COMPILER_CODEGEN_ARM_BACKENPASSOPTIMIZATIONS_H__

struct ArmLIR;

#define BACKEND_OPT_RANGE 200

/* Bit flags describing the behavior of optimization Pass */
typedef enum _OptimizationPassPositons {
    /* Normal Trace Optimization Pass */
    kTraceStrEliminationSinking,
    /* Loop Trace Optimization Pass */
    kLoopLdrHoisting,
    kLoopStrEliminationSinking,
    kLoopLdrStrPairLICM,
    kLoopInversion,
} OptimizationPassPositons;

/* Optimization Pass */
#define DO_NOT_OPT                       0
#define LOOP_INVERSION                  (1 << kLoopInversion)
#define LOOP_LDR_HOISTING               (1 << kLoopLdrHoisting)
#define LOOP_STR_ELIMINATION_SINKING    (1 << kLoopStrEliminationSinking)
#define LOOP_LDR_STR_PAIR_LICM          (1 << kLoopLdrStrPairLICM)
#define TRACE_STR_ELIMINATION_SINKING   (1 << kTraceStrEliminationSinking)

/* Optimization Level */
#define BACKEND_OPT_LEVEL1 (LOOP_INVERSION)
#define BACKEND_OPT_LEVEL2 (BACKEND_OPT_LEVEL1 | LOOP_LDR_HOISTING | LOOP_STR_ELIMINATION_SINKING | LOOP_LDR_STR_PAIR_LICM)
#define BACKEND_OPT_LEVEL3 (BACKEND_OPT_LEVEL2 | TRACE_STR_ELIMINATION_SINKING)

typedef struct _OptLIRList {
    ArmLIR *inst[BACKEND_OPT_RANGE];
    u1      position[BACKEND_OPT_RANGE];
    int     instCount;
} OptLIRList;

typedef struct _BackEndOptUnit {
    u4         backEndOptLevel;
    u4         backEndOptPass;

    /* to analyze */
    bool       cUnitHasInnerBranch;
    OptLIRList labelList;
    OptLIRList strSlotList;
    OptLIRList branchList;
    OptLIRList strList;
    OptLIRList ldrList;
    OptLIRList allList;

    /* to optimize */
    ArmLIR    *headBlockTargetLabel;
} BackEndOptUnit;

#endif
