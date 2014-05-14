#include "BackEndPassOptimizations.h"

#include "BackEndPassAnalyzer.cpp"
#include "BackEndPassOptimizer.cpp"

static void applyBackEndOptimizationLevel(CompilationUnit *cUnit, u4 *backEndOptLevel)
{
    switch(cUnit->jitOptLevel) {
    case 1:
        *backEndOptLevel = BACKEND_OPT_LEVEL1;
        break;
    case 2:
        *backEndOptLevel = BACKEND_OPT_LEVEL2;
        break;
    case 3:
        *backEndOptLevel = BACKEND_OPT_LEVEL3;
        break;
    default:
        *backEndOptLevel = DO_NOT_OPT;
        break;
    }
}

static void applyBackEndPassOptimizations(CompilationUnit *cUnit)
{
    BackEndOptUnit bUnit;
    bool canOpt = false;

    if (cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn == NULL ||
        cUnit->backEndOptInfo.normalCodeBlockEndLIRInsn == NULL) {
        ALOGW("backEndOptInfo is insufficient");
        return;
    }

    cUnit->jitOptLevel = 2; /* todo: The jitOptLevel should be set dynamically.*/
    memset(&bUnit, 0, sizeof(bUnit));

    /* set Optimization Level */
    applyBackEndOptimizationLevel(cUnit, &bUnit.backEndOptLevel);
    if (bUnit.backEndOptLevel == DO_NOT_OPT)
        return;

    /* start Analyzer */
    canOpt = commonBackEndAnalysis(cUnit, &bUnit);
    if (!canOpt)
        return;

    /* set Optimization Pass based on the analysis */
    if (bUnit.backEndOptLevel & TRACE_STR_ELIMINATION_SINKING)
        bUnit.backEndOptPass |= (bUnit.backEndOptLevel & applyTraceStoreEliminationAndSinkingBackEndAnalysis(cUnit, &bUnit));

    if (bUnit.backEndOptLevel & LOOP_INVERSION)
        bUnit.backEndOptPass |= (bUnit.backEndOptLevel & applyLoopInversionBackEndAnalysis(cUnit));

    if (bUnit.backEndOptLevel & (LOOP_LDR_HOISTING | LOOP_STR_ELIMINATION_SINKING | LOOP_LDR_STR_PAIR_LICM))
        bUnit.backEndOptPass |= (bUnit.backEndOptLevel & applyLICMBackEndAnalysis(cUnit, &bUnit));

    /* start Optimizer */
    canOpt = commonPrepareBackEndOptimization(cUnit, &bUnit);
    if (!canOpt)
        return;

    if (gDvmJit.jitDebug) {
        if (bUnit.backEndOptPass & 0x1f) {
            ALOGD("[SWE] available optimizations for %s%s, 0x%x:", cUnit->method->clazz->descriptor,
                    cUnit->method->name, cUnit->traceDesc->trace[0].info.frag.startOffset);
            if (bUnit.backEndOptPass & LOOP_INVERSION)
                ALOGD("[SWE] - Loop Inversion");
            if(bUnit.backEndOptPass & LOOP_LDR_HOISTING)
                ALOGD("[SWE] - Load Hoisting");
            if(bUnit.backEndOptPass & LOOP_STR_ELIMINATION_SINKING)
                ALOGD("[SWE] - Store Sinking");
            if(bUnit.backEndOptPass & LOOP_LDR_STR_PAIR_LICM)
                ALOGD("[SWE] - Load/Store Pair LICM");
            if(bUnit.backEndOptPass & TRACE_STR_ELIMINATION_SINKING)
                ALOGD("[SWE] - Store Sinking for Non-Loop Trace");
        }
    }

    /* todo: call optimizer based on Optimization Pass */
    if (gDvmJit.loopInverse && (bUnit.backEndOptPass & LOOP_INVERSION))
        applyLoopInversion(cUnit);

    if (gDvmJit.ldrHoist && (bUnit.backEndOptPass & LOOP_LDR_HOISTING))
        applyLoopLoadHoisting(cUnit, &bUnit);

    if (gDvmJit.strSink && (bUnit.backEndOptPass & LOOP_STR_ELIMINATION_SINKING))
        applyLoopStoreSinking(cUnit, &bUnit);

    if (gDvmJit.licm && (bUnit.backEndOptPass & LOOP_LDR_STR_PAIR_LICM))
        applyLoopLoadStorePairLICM(cUnit, &bUnit);

    if (gDvmJit.traceStrSink && (bUnit.backEndOptPass & TRACE_STR_ELIMINATION_SINKING))
        applyTraceStoreEliminationAndSinking(cUnit, &bUnit);
}
