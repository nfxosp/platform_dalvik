%verify "executed"
%include "armv5te/binopLit8.S" {"instr":"\n#ifdef __ARM_ARCH_EXT_IDIV__\n    .inst 0xe710f110 /*  sdiv    r0, r0, r1 */\n#else\n    bl     __aeabi_idiv\n#endif\n","chkzero":"1"}
