%verify "executed"
/* idivmod returns quotient in r0 and remainder in r1 */
%include "armv5te/binopLit8.S" {"instr":"\n#ifdef __ARM_ARCH_EXT_IDIV__\n    .inst 0xe712f110 /*sdiv    r2, r0, r1*/\n    mls    r1, r1, r2, r0\n#else\n    bl      __aeabi_idivmod\n#endif\n", "result":"r1", "chkzero":"1"}
