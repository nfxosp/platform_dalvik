#include "BackEndPassOptimizerLoop.cpp"
#include "BackEndPassOptimizerTrace.cpp"

/* prepare Custom LIR Set for OptimizationPass */
static bool commonPrepareBackEndOptimization(CompilationUnit *cUnit, BackEndOptUnit *bUnit)
{
    /* make start block target label */
    bUnit->headBlockTargetLabel = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
    *(bUnit->headBlockTargetLabel) = *((ArmLIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn);
    bUnit->headBlockTargetLabel->opcode = kArmPseudoBackEndOptTargetLabel;
    dvmCompilerInsertLIRAfter(
        (LIR *)cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn, (LIR *)bUnit->headBlockTargetLabel);

    for (int i = 0; i < bUnit->branchList.instCount; i++) {
        if (bUnit->branchList.inst[i]->generic.target == cUnit->backEndOptInfo.normalCodeBlockStartLIRInsn)
            bUnit->branchList.inst[i]->generic.target = (LIR *)bUnit->headBlockTargetLabel;
    }

    /* make str slot list */
    for (int i = 0; i < bUnit->strSlotList.instCount; i++) {
        ArmLIR* strSlot = (ArmLIR *)bUnit->strSlotList.inst[i]->generic.target;
        ArmLIR* pseudoLabel = (ArmLIR *)dvmCompilerNew(sizeof(ArmLIR), true);
        *pseudoLabel = *strSlot;
        pseudoLabel->opcode = kArmPseudoBackEndOptTargetLabel;
        dvmCompilerInsertLIRAfter((LIR *)strSlot, (LIR *)pseudoLabel);
        bUnit->strSlotList.inst[i] = pseudoLabel;
    }

    return true;
}
