#include "BackEndPassOptimizations.cpp"
#include "PatternMatchingOptimizations.cpp"

void dvmCompilerApplyBackEndOptimizations(CompilationUnit *cUnit)
{
#if !defined(WITH_SELF_VERIFICATION)
    bool isPatternMatching = false;

    if (gDvmJit.patternMatching)
        isPatternMatching = applyPatternMatchingOptimizations(cUnit);

    if (isPatternMatching)
        return;
#endif

    applyBackEndPassOptimizations(cUnit);
}
