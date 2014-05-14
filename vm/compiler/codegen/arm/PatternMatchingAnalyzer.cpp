static void doPrefilteringForPatternMatchingAnalysis(CompilationUnit *cUnit)
{
    GrowableListIterator iterator;
    BasicBlock          *pBB = NULL;
    MIR                 *pMir = NULL;

    dvmGrowableListIteratorInit(&cUnit->blockList, &iterator);

    for (unsigned int i = 0; i < iterator.size; i++) {
        pBB = (BasicBlock *)dvmGrowableListIteratorNext(&iterator);
        if (pBB == NULL)
            break;

        pMir = pBB->firstMIRInsn;
        while (pMir != NULL) {
            cUnit->backEndOptInfo.perceivedSequence += pMir->dalvikInsn.opcode;
            pMir = pMir->next;
        }
    }
}

static bool applyPatternMatchingAnalysis(CompilationUnit *cUnit,
                                         MIRInstruction pBlockMIRInsn[],
                                         unsigned int blockSize)
{
    GrowableListIterator iterator;
    BasicBlock          *pBB = NULL;
    MIR                 *pMir = NULL;
    unsigned int         nInsn = 0;
    DecodedInstruction   tmpMIRIns[20];

    dvmGrowableListIteratorInit(&cUnit->blockList, &iterator);

    for (unsigned int i = 0; i < iterator.size; i++) {
        pBB = (BasicBlock *)dvmGrowableListIteratorNext(&iterator);
        if (pBB == NULL)
            break;

        pMir = pBB->firstMIRInsn;
        while (pMir != NULL) {
            tmpMIRIns[nInsn++] = pMir->dalvikInsn;
            if (nInsn == blockSize)
                break;

            pMir = pMir->next;
        }
        if (nInsn == blockSize)
            break;
    }

    bool res = true;
    for (unsigned int i = 0; i < blockSize; i++) {
        if (tmpMIRIns[i].opcode != pBlockMIRInsn[i].opcode ||
            tmpMIRIns[i].vA != pBlockMIRInsn[i].vA ||
            tmpMIRIns[i].vB != pBlockMIRInsn[i].vB ||
            tmpMIRIns[i].vC != pBlockMIRInsn[i].vC) {
            res = false;
            break;
        }
    }

    return res;
}
