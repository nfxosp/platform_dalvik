#include <sys/mman.h>

/* Q1Block1 int */
static unsigned char Q1Block1OptCode[] = {
    0x28, 0x68, 0xAA, 0x68, 0x4F, 0xF0, 0x65, 0x03, 0x00, 0x28,
    0x3E, 0xDB, 0x61, 0x28, 0x32, 0xDA, 0x4F, 0xF0, 0x04, 0x04,
    0xA0, 0xEE, 0x10, 0x0B, 0x20, 0x44, 0x4F, 0xF0, 0x00, 0x07,
    0xA4, 0xEE, 0x10, 0x4B, 0x02, 0xEE, 0x10, 0x7A, 0x4F, 0xF0,
    0x01, 0x07, 0x02, 0xEE, 0x90, 0x7A, 0x4F, 0xF0, 0x02, 0x07,
    0x80, 0xEF, 0x51, 0x60, 0x03, 0xEE, 0x10, 0x7A, 0x4F, 0xF0,
    0x03, 0x07, 0xC0, 0xEF, 0x50, 0x00, 0x03, 0xEE, 0x90, 0x7A,
    0x20, 0xEF, 0x42, 0x08, 0x60, 0xEF, 0x50, 0x29, 0x62, 0xEF,
    0xC6, 0x28, 0x60, 0xEF, 0xE2, 0x08, 0x20, 0xEF, 0x44, 0x08,
    0x20, 0x44, 0x98, 0x42, 0xF4, 0xDB, 0xF8, 0xFF, 0x60, 0x02,
    0x70, 0xEF, 0xA1, 0x08, 0x10, 0xEE, 0x90, 0xAB, 0x52, 0x44,
    0xA0, 0xEB, 0x04, 0x00, 0x98, 0x42, 0x16, 0xDA, 0x00, 0xFB,
    0x00, 0xF1, 0x01, 0xF1, 0x01, 0x01, 0x0A, 0x44, 0x00, 0xF1,
    0x01, 0x00, 0x98, 0x42, 0xF6, 0xDB, 0x0C, 0xE0, 0x00, 0xFB,
    0x00, 0xF1, 0x01, 0xF1, 0x01, 0x01, 0x0A, 0x44, 0x00, 0xF1,
    0x01, 0x00, 0x96, 0xF8, 0x2A, 0x40, 0x34, 0xB9, 0x98, 0x42,
    0xF3, 0xDB, 0xFF, 0xE7, 0x28, 0x60, 0xAA, 0x60, 0xAB, 0x61,
    0x07, 0xE0, 0x28, 0x60, 0xAA, 0x60, 0xA9, 0x61, 0xFF, 0xE7
};

/* Q1Block1 short */
static unsigned char Q1Block2OptCode[] = {
    0x28, 0x68, 0xAA, 0x68, 0x4F, 0xF0, 0x4D, 0x03, 0x00, 0x28,
    0x4E, 0xDB, 0x45, 0x28, 0x40, 0xDA, 0x4F, 0xF0, 0x08, 0x04,
    0xA0, 0xEE, 0x30, 0x0B, 0x20, 0x44, 0x40, 0xF3, 0x0F, 0x00,
    0x00, 0x27, 0xC0, 0xF2, 0x01, 0x07, 0x02, 0xEE, 0x10, 0x7A,
    0x02, 0x27, 0xC0, 0xF2, 0x03, 0x07, 0x02, 0xEE, 0x90, 0x7A,
    0x04, 0x27, 0xC0, 0xF2, 0x05, 0x07, 0x03, 0xEE, 0x10, 0x7A,
    0x06, 0x27, 0xC0, 0xF2, 0x07, 0x07, 0x03, 0xEE, 0x90, 0x7A,
    0x10, 0xEF, 0x42, 0x08, 0xA4, 0xEE, 0x30, 0x4B, 0x80, 0xEF,
    0x51, 0x68, 0xC0, 0xEF, 0x50, 0x08, 0x50, 0xEF, 0x50, 0x29,
    0x52, 0xEF, 0xC6, 0x28, 0x50, 0xEF, 0xE2, 0x08, 0x10, 0xEF,
    0x44, 0x08, 0x20, 0x44, 0x40, 0xF3, 0x0F, 0x00, 0x98, 0x42,
    0xF2, 0xDB, 0xF4, 0xFF, 0x60, 0x02, 0xF8, 0xFF, 0x60, 0x02,
    0x70, 0xEF, 0xA1, 0x08, 0x10, 0xEE, 0xB0, 0xAB, 0x52, 0x44,
    0x42, 0xF3, 0x0F, 0x02, 0xA0, 0xEB, 0x04, 0x00, 0x40, 0xF3,
    0x0F, 0x00, 0x98, 0x42, 0x26, 0xDA, 0x00, 0xFB, 0x00, 0xF1,
    0x01, 0x31, 0x52, 0x18, 0x42, 0xF3, 0x0F, 0x02, 0x01, 0x30,
    0x40, 0xF3, 0x0F, 0x00, 0x98, 0x42, 0xF4, 0xDB, 0x0E, 0xE0,
    0x00, 0xFB, 0x00, 0xF1, 0x01, 0x31, 0x52, 0x18, 0x42, 0xF3,
    0x0F, 0x02, 0x01, 0x30, 0x40, 0xF3, 0x0F, 0x00, 0x96, 0xF8,
    0x2A, 0x40, 0x34, 0xB9, 0x98, 0x42, 0xF1, 0xDB, 0xFF, 0xE7,
    0x28, 0x60, 0xAA, 0x60, 0xAB, 0x61, 0x07, 0xE0, 0x28, 0x60,
    0xAA, 0x60, 0xA9, 0x61, 0xFF, 0xE7
};

/* Q1Block1 byte */
static unsigned char Q1Block4OptCode[] = {
    0x28, 0x68, 0xAA, 0x68, 0x4E, 0x23, 0x00, 0x28, 0x54, 0xDB,
    0x3E, 0x28, 0x46, 0xDA, 0x40, 0xF2, 0x10, 0x04, 0xE0, 0xEE,
    0x10, 0x0B, 0x20, 0x44, 0x40, 0xF3, 0x07, 0x00, 0x40, 0xF2,
    0x00, 0x17, 0xC0, 0xF2, 0x02, 0x37, 0x02, 0xEE, 0x10, 0x7A,
    0x40, 0xF2, 0x04, 0x57, 0xC0, 0xF2, 0x06, 0x77, 0x02, 0xEE,
    0x90, 0x7A, 0x40, 0xF6, 0x08, 0x17, 0xC0, 0xF6, 0x0A, 0x37,
    0x03, 0xEE, 0x10, 0x7A, 0x40, 0xF6, 0x0C, 0x57, 0xC0, 0xF6,
    0x0E, 0x77, 0x03, 0xEE, 0x90, 0x7A, 0x00, 0xEF, 0x42, 0x08,
    0xE4, 0xEE, 0x10, 0x4B, 0x80, 0xEF, 0x51, 0x6E, 0xC0, 0xEF,
    0x50, 0x0E, 0x40, 0xEF, 0x50, 0x29, 0x42, 0xEF, 0xC6, 0x28,
    0x40, 0xEF, 0xE2, 0x08, 0x00, 0xEF, 0x44, 0x08, 0x20, 0x44,
    0x40, 0xF3, 0x07, 0x00, 0x98, 0x42, 0xF2, 0xDB, 0xF0, 0xFF,
    0x60, 0x02, 0xF4, 0xFF, 0x60, 0x02, 0xF8, 0xFF, 0x60, 0x02,
    0x70, 0xEF, 0xA1, 0x08, 0x50, 0xEE, 0x90, 0xAB, 0x52, 0x44,
    0x42, 0xF3, 0x07, 0x02, 0xA0, 0xEB, 0x04, 0x00, 0x40, 0xF3,
    0x07, 0x00, 0x98, 0x42, 0x27, 0xDA, 0x00, 0xFB, 0x00, 0xF1,
    0x01, 0x31, 0x52, 0x18, 0x42, 0xF3, 0x07, 0x02, 0x01, 0x30,
    0x40, 0xF3, 0x07, 0x00, 0x98, 0x42, 0xF4, 0xDB, 0x0E, 0xE0,
    0x00, 0xFB, 0x00, 0xF1, 0x01, 0x31, 0x52, 0x18, 0x42, 0xF3,
    0x07, 0x02, 0x01, 0x30, 0x40, 0xF3, 0x07, 0x00, 0x96, 0xF8,
    0x2A, 0x40, 0x34, 0xB9, 0x98, 0x42, 0xF1, 0xDB, 0xFF, 0xE7,
    0x28, 0x60, 0xAA, 0x60, 0xAB, 0x61, 0x08, 0xE0, 0x28, 0x60,
    0xAA, 0x60, 0xA9, 0x61, 0xFF, 0xE7
};

void dvmCompilerSIMDOpt(CompilationUnit *cUnit) {
    if (gDvmJit.jitDebug) {
        switch(pattern) {
        case kQ1Block1:
            ALOGD("[SWE] dvmCompilerSIMDOpt for kQ1Block1 start");
            break;
        case kQ1Block2:
            ALOGD("[SWE] dvmCompilerSIMDOpt for kQ1Block2 start");
            break;
        case kQ1Block4:
            ALOGD("[SWE] dvmCompilerSIMDOpt for kQ1Block4 start");
            break;
        default:
            break;
        }
    }

    if (pattern == kQ1Block1) {
        UNPROTECT_CODE_CACHE(cUnit->baseAddr, cUnit->totalSize);

        unsigned char *code = Q1Block1OptCode;
        unsigned int codeSize = 180;

        memcpy((char *)cUnit->baseAddr + 16, code, codeSize);

        /* Flush dcache and invalidate the icache to maintain coherence */
        dvmCompilerCacheFlush((long)cUnit->baseAddr,
                              (long)((char *) cUnit->baseAddr + cUnit->totalSize), 0);

        PROTECT_CODE_CACHE(cUnit->baseAddr, cUnit->totalSize);
    } else if (pattern == kQ1Block2) {
        UNPROTECT_CODE_CACHE(cUnit->baseAddr, cUnit->totalSize);

        unsigned char *code = Q1Block2OptCode;
        unsigned int codeSize = 216;

        memcpy((char *)cUnit->baseAddr + 16, code, codeSize);

        /* Flush dcache and invalidate the icache to maintain coherence */
        dvmCompilerCacheFlush((long)cUnit->baseAddr,
                              (long)((char *) cUnit->baseAddr + cUnit->totalSize), 0);

        PROTECT_CODE_CACHE(cUnit->baseAddr, cUnit->totalSize);
    } else if (pattern == kQ1Block4) {
        UNPROTECT_CODE_CACHE(cUnit->baseAddr, cUnit->totalSize);

        unsigned char *code = Q1Block4OptCode;
        unsigned int codeSize = 226;

        memcpy((char *)cUnit->baseAddr + 16, code, codeSize);

        /* Flush dcache and invalidate the icache to maintain coherence */
        dvmCompilerCacheFlush((long)cUnit->baseAddr,
                              (long)((char *) cUnit->baseAddr + cUnit->totalSize), 0);

        PROTECT_CODE_CACHE(cUnit->baseAddr, cUnit->totalSize);
    }


    if (gDvmJit.jitDebug) {
        switch(pattern) {
        case kQ1Block1:
            ALOGD("[SWE] dvmCompilerSIMDOpt for kQ1Block1 exit");
            break;
        case kQ1Block2:
            ALOGD("[SWE] dvmCompilerSIMDOpt for kQ1Block2 exit");
            break;
        case kQ1Block4:
            ALOGD("[SWE] dvmCompilerSIMDOpt for kQ1Block4 exit");
            break;
        default:
            break;
        }
    }
}