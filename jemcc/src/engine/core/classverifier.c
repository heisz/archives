/**
 * JEMCC methods to support Java bytecode verification.
 * Copyright (C) 1999-2004 J.M. Heisz 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU Lesser General Public 
 * License, as well as further clarification on your rights to use this 
 * software.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include "jeminc.h"

/* Read the VM structure/method definitions */
#include "jem.h"

/* Quick and dirty macro to get at the class constant information */
#define CL_CONSTANT(x) pData->constantPool[x - 1]

/* Methods to construct u1/u2/u4 data from the bytecode data */
static u1 read_u1(const jubyte **ptr) {
    return (u1) *((*ptr)++);
}
static u2 read_u2(const jubyte **ptr) {
    return ((u2) (read_u1(ptr) << 8)) | read_u1(ptr);
}
static u4 read_u4(const jubyte **ptr) {
    return ((u4) (read_u1(ptr) << 24)) | (read_u1(ptr) << 16) |
                 (read_u1(ptr) << 8) | read_u1(ptr);
}

/* Methods to insert numeric data into the bytecode area */
static void pack_u1(u1 val, jubyte **ptr) {
    *((*ptr)++) = val;
}
static void pack_u2(u2 val, jubyte **ptr) {
    jubyte lval = val & 0xFF;

    *((*ptr)++) = val >> 8;
    *((*ptr)++) = lval;
}

/*
 * This method is one of the most complex in the Jem engine.  In the
 * most basic form, it performs the Pass 3 class verification as outlined
 * in the Java VM specification.  However, at the same time it is 
 * compacting the parsed class data references to minimize the class
 * storage requirements.
 */

#define VALID_OPCODE_FLAG 1

/* Instruction lengths by opcode (including opcode itself) */
jbyte opInstLengths[] = {
1 /* 0 - "nop" */,
1 /* 1 - "aconst_null" */,
1 /* 2 - "iconst_m1" */,
1 /* 3 - "iconst_0" */,
1 /* 4 - "iconst_1" */,
1 /* 5 - "iconst_2" */,
1 /* 6 - "iconst_3" */,
1 /* 7 - "iconst_4" */,
1 /* 8 - "iconst_5" */,
1 /* 9 - "lconst_0" */,
1 /* 10 - "lconst_1" */,
1 /* 11 - "fconst_0" */,
1 /* 12 - "fconst_1" */,
1 /* 13 - "fconst_2" */,
1 /* 14 - "dconst_0" */,
1 /* 15 - "dconst_1" */,
2 /* 16 - "bipush" */,
3 /* 17 - "sipush" */,
2 /* 18 - "ldc" */,
3 /* 19 - "ldc_w" */,
3 /* 20 - "ldc2_w" */,
2 /* 21 - "iload" */,
2 /* 22 - "lload" */,
2 /* 23 - "fload" */,
2 /* 24 - "dload" */,
2 /* 25 - "aload" */,
1 /* 26 - "iload_0" */,
1 /* 27 - "iload_1" */,
1 /* 28 - "iload_2" */,
1 /* 29 - "iload_3" */,
1 /* 30 - "lload_0" */,
1 /* 31 - "lload_1" */,
1 /* 32 - "lload_2" */,
1 /* 33 - "lload_3" */,
1 /* 34 - "fload_0" */,
1 /* 35 - "fload_1" */,
1 /* 36 - "fload_2" */,
1 /* 37 - "fload_3" */,
1 /* 38 - "dload_0" */,
1 /* 39 - "dload_1" */,
1 /* 40 - "dload_2" */,
1 /* 41 - "dload_3" */,
1 /* 42 - "aload_0" */,
1 /* 43 - "aload_1" */,
1 /* 44 - "aload_2" */,
1 /* 45 - "aload_3" */,
1 /* 46 - "iaload" */,
1 /* 47 - "laload" */,
1 /* 48 - "faload" */,
1 /* 49 - "daload" */,
1 /* 50 - "aaload" */,
1 /* 51 - "baload" */,
1 /* 52 - "caload" */,
1 /* 53 - "saload" */,
2 /* 54 - "istore" */,
2 /* 55 - "lstore" */,
2 /* 56 - "fstore" */,
2 /* 57 - "dstore" */,
2 /* 58 - "astore" */,
1 /* 59 - "istore_0" */,
1 /* 60 - "istore_1" */,
1 /* 61 - "istore_2" */,
1 /* 62 - "istore_3" */,
1 /* 63 - "lstore_0" */,
1 /* 64 - "lstore_1" */,
1 /* 65 - "lstore_2" */,
1 /* 66 - "lstore_3" */,
1 /* 67 - "fstore_0" */,
1 /* 68 - "fstore_1" */,
1 /* 69 - "fstore_2" */,
1 /* 70 - "fstore_3" */,
1 /* 71 - "dstore_0" */,
1 /* 72 - "dstore_1" */,
1 /* 73 - "dstore_2" */,
1 /* 74 - "dstore_3" */,
1 /* 75 - "astore_0" */,
1 /* 76 - "astore_1" */,
1 /* 77 - "astore_2" */,
1 /* 78 - "astore_3" */,
1 /* 79 - "iastore" */,
1 /* 80 - "lastore" */,
1 /* 81 - "fastore" */,
1 /* 82 - "dastore" */,
1 /* 83 - "aastore" */,
1 /* 84 - "bastore" */,
1 /* 85 - "castore" */,
1 /* 86 - "sastore" */,
1 /* 87 - "pop" */,
1 /* 88 - "pop2" */,
1 /* 89 - "dup" */,
1 /* 90 - "dup_x1" */,
1 /* 91 - "dup_x2" */,
1 /* 92 - "dup2" */,
1 /* 93 - "dup2_x1" */,
1 /* 94 - "dup2_x2" */,
1 /* 95 - "swap" */,
1 /* 96 - "iadd" */,
1 /* 97 - "ladd" */,
1 /* 98 - "fadd" */,
1 /* 99 - "dadd" */,
1 /* 100 - "isub" */,
1 /* 101 - "lsub" */,
1 /* 102 - "fsub" */,
1 /* 103 - "dsub" */,
1 /* 104 - "imul" */,
1 /* 105 - "lmul" */,
1 /* 106 - "fmul" */,
1 /* 107 - "dmul" */,
1 /* 108 - "idiv" */,
1 /* 109 - "ldiv" */,
1 /* 110 - "fdiv" */,
1 /* 111 - "ddiv" */,
1 /* 112 - "irem" */,
1 /* 113 - "lrem" */,
1 /* 114 - "frem" */,
1 /* 115 - "drem" */,
1 /* 116 - "ineg" */,
1 /* 117 - "lneg" */,
1 /* 118 - "fneg" */,
1 /* 119 - "dneg" */,
1 /* 120 - "ishl" */,
1 /* 121 - "lshl" */,
1 /* 122 - "ishr" */,
1 /* 123 - "lshr" */,
1 /* 124 - "iushr" */,
1 /* 125 - "lushr" */,
1 /* 126 - "iand" */,
1 /* 127 - "land" */,
1 /* 128 - "ior" */,
1 /* 129 - "lor" */,
1 /* 130 - "ixor" */,
1 /* 131 - "lxor" */,
3 /* 132 - "iinc" */,
1 /* 133 - "i2l" */,
1 /* 134 - "i2f" */,
1 /* 135 - "i2d" */,
1 /* 136 - "l2i" */,
1 /* 137 - "l2f" */,
1 /* 138 - "l2d" */,
1 /* 139 - "f2i" */,
1 /* 140 - "f2l" */,
1 /* 141 - "f2d" */,
1 /* 142 - "d2i" */,
1 /* 143 - "d2l" */,
1 /* 144 - "d2f" */,
1 /* 145 - "i2b" */,
1 /* 146 - "i2c" */,
1 /* 147 - "i2s" */,
1 /* 148 - "lcmp" */,
1 /* 149 - "fcmpl" */,
1 /* 150 - "fcmpg" */,
1 /* 151 - "dcmpl" */,
1 /* 152 - "dcmpg" */,
3 /* 153 - "ifeq" */,
3 /* 154 - "ifne" */,
3 /* 155 - "iflt" */,
3 /* 156 - "ifge" */,
3 /* 157 - "ifgt" */,
3 /* 158 - "ifle" */,
3 /* 159 - "if_icmpeq" */,
3 /* 160 - "if_icmpne" */,
3 /* 161 - "if_icmplt" */,
3 /* 162 - "if_icmpge" */,
3 /* 163 - "if_icmpgt" */,
3 /* 164 - "if_icmple" */,
3 /* 165 - "if_acmpeq" */,
3 /* 166 - "if_acmpne" */,
3 /* 167 - "goto" */,
3 /* 168 - "jsr" */,
2 /* 169 - "ret" */,
0 /* 170 - "tableswitch" */,
0 /* 171 - "lookupswitch" */,
1 /* 172 - "ireturn" */,
1 /* 173 - "lreturn" */,
1 /* 174 - "freturn" */,
1 /* 175 - "dreturn" */,
1 /* 176 - "areturn" */,
1 /* 177 - "return" */,
3 /* 178 - "getstatic" */,
3 /* 179 - "putstatic" */,
3 /* 180 - "getfield" */,
3 /* 181 - "putfield" */,
3 /* 182 - "invokevirtual" */,
3 /* 183 - "invokespecial" */,
3 /* 184 - "invokestatic" */,
5 /* 185 - "invokeinterface" */,
-1 /* 186 - invalid */,
3 /* 187 - "new" */,
2 /* 188 - "newarray" */,
3 /* 189 - "anewarray" */,
1 /* 190 - "arraylength" */,
1 /* 191 - "athrow" */,
3 /* 192 - "checkcast" */,
3 /* 193 - "instanceof" */,
1 /* 194 - "monitorenter" */,
1 /* 195 - "monitorexit" */,
0 /* 196 - "wide" */,
4 /* 197 - "multinewarray" */,
3 /* 198 - "ifnull" */,
3 /* 199 - "ifnonnull" */,
5 /* 200 - "goto_w" */,
5 /* 201 - "jsr_w" */,
-1 /* 202 - "breakpoint" */,
-1 /* 203 - invalid */,
-1 /* 204 - invalid */,
-1 /* 205 - invalid */,
-1 /* 206 - invalid */,
-1 /* 207 - invalid */,
-1 /* 208 - invalid */,
-1 /* 209 - invalid */,
-1 /* 210 - invalid */,
-1 /* 211 - invalid */,
-1 /* 212 - invalid */,
-1 /* 213 - invalid */,
-1 /* 214 - invalid */,
-1 /* 215 - invalid */,
-1 /* 216 - invalid */,
-1 /* 217 - invalid */,
-1 /* 218 - invalid */,
-1 /* 219 - invalid */,
-1 /* 220 - invalid */,
-1 /* 221 - invalid */,
-1 /* 222 - invalid */,
-1 /* 223 - invalid */,
-1 /* 224 - invalid */,
-1 /* 225 - invalid */,
-1 /* 226 - invalid */,
-1 /* 227 - invalid */,
-1 /* 228 - invalid */,
-1 /* 229 - invalid */,
-1 /* 230 - invalid */,
-1 /* 231 - invalid */,
-1 /* 232 - invalid */,
-1 /* 233 - invalid */,
-1 /* 234 - invalid */,
-1 /* 235 - invalid */,
-1 /* 236 - invalid */,
-1 /* 237 - invalid */,
-1 /* 238 - invalid */,
-1 /* 239 - invalid */,
-1 /* 240 - invalid */,
-1 /* 241 - invalid */,
-1 /* 242 - invalid */,
-1 /* 243 - invalid */,
-1 /* 244 - invalid */,
-1 /* 245 - invalid */,
-1 /* 246 - invalid */,
-1 /* 247 - invalid */,
-1 /* 248 - invalid */,
-1 /* 249 - invalid */,
-1 /* 250 - invalid */,
-1 /* 251 - invalid */,
-1 /* 252 - invalid */,
-1 /* 253 - invalid */,
-1 /* 254 - invalid */,
-1 /* 255 - invalid */
};

/**
 * Perform the verification tests on the method bytecode as described
 * in the Java VM specifications.  While the Pass One and Pass Two validations
 * are mainly handled by the parsing and linking operations, this method
 * performs the algorithmically more complex Pass Three and Four verifications.
 *
 * Note: this method also performs a compaction on the class constant
 *       information, removing entries which exist for class definition and
 *       only leaving those required by the executing bytecode.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     classData - the partially resolved/linked class instance information
 *
 * Returns:
 *     JNI_OK - the class method bytecode verification was successful
 *     JNI_ERR - a verification error has occurred.  A VerifyError has been
 *               thrown in the current environment
 *     JNI_ENOMEM - a memory allocation has failed during the creation of the
 *                  verification tables and an OutOfMemory error has been
 *                  thrown in the current environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 *     VerifyError - the bytecode verification has failed
 */
jint JEM_VerifyClassByteCode(JNIEnv *env, JEM_ClassData *classData) {
    JEM_ParsedClassData *pData = classData->parseData;
    jubyte opCode, *opCodeMapTable;
    jubyte *byteCode, *bytePtr;
    jint tblCount, lowIndex, highIndex;
    int i, tag, pc, pcinc, len, passIdx, fieldIdx, methodIdx, cnstIdx, *iptr;
    int classIdx, constIdx, classRefCount, constantCount;
    JEM_ConstantPoolData *cpData;
    JEM_BCMethod *bcMethodPtr;

    /* All of linkage/verification operations occur method by method */
    classRefCount = constantCount = 0;
    for (passIdx = 0; passIdx < classData->localMethodCount; passIdx++) {
        bcMethodPtr = classData->localMethods[passIdx].method.bcMethod;
        if (bcMethodPtr == NULL) continue;

        /* Prepare the opcode mapping table and argument mappings */
        len = bcMethodPtr->codeLength;
        byteCode = bcMethodPtr->code;
        opCodeMapTable = (jubyte *) JEMCC_Malloc(env, len);
        if (opCodeMapTable == NULL) return JNI_ENOMEM;
        pc = 0;
        while (pc < len) {
            opCode = byteCode[pc];
            pcinc = (int) opInstLengths[(int) opCode];
            if (pcinc < 0) {
                JEMCC_ThrowStdThrowableIdx(env, JEMCC_Class_VerifyError, NULL,
                                           "Invalid opCode encountered.");
                JEMCC_Free(opCodeMapTable);
                return JNI_ERR;
            } else if (pcinc > 0) {
                /* Handle the constant/method/field entry mapping cases */
                switch (opCode) {
                    case 180: /* getfield */
                    case 178: /* getstatic */
                    case 181: /* putfield */
                    case 179: /* putstatic */
                        /* Validate and remap field reference index */
                        bytePtr = byteCode + pc + 1;
                        fieldIdx = (int) read_u2((const jubyte **) &bytePtr);
                        tag = pData->constantPool[fieldIdx - 1].generic.tag;
                        if ((tag != CONSTANT_ResolvedFieldref) &&
                            (tag != CONSTANT_ResolvedFieldrefErr)) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                             JEMCC_Class_VerifyError, NULL,
                                             "Field op on non-field constant");
                            JEMCC_Free(opCodeMapTable);
                            return JNI_ERR;
                        }
                        bytePtr = byteCode + pc + 1;
                        fieldIdx = pData->constantPool[fieldIdx - 
                                                    1].res_ref_info.tableIndex;
/*
                        if ((opCode == 180) || (opCode == 181)) {
XXXX
                            JEMCC_ThrowStdThrowableIdx(env, 
                                        JEMCC_Class_VerifyError, NULL,
                                        "Non-static field op on static field");
                        } else {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                        JEMCC_Class_VerifyError, NULL,
                                        "Static field op on non-static field");
                        }
*/
                        pack_u2((u2) fieldIdx, (jubyte **) &bytePtr);
                        break;
                    case 185: /* invokeinterface */
                        /* Validate and remap interface method ref index */
                        bytePtr = byteCode + pc + 1;
                        methodIdx = (int) read_u2((const jubyte **) &bytePtr);
                        tag = pData->constantPool[methodIdx - 1].generic.tag;
                        if ((tag != CONSTANT_ResolvedInterfaceMethodref) &&
                            (tag != CONSTANT_ResolvedInterfaceMethodrefErr)) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                     JEMCC_Class_VerifyError, NULL,
                                     "Interface op on non-if method constant");
                            JEMCC_Free(opCodeMapTable);
                            return JNI_ERR;
                        }
                        bytePtr = byteCode + pc + 1;
                        methodIdx = pData->constantPool[methodIdx - 
                                                    1].res_ref_info.tableIndex;
                        pack_u2((u2) methodIdx, (jubyte **) &bytePtr);
                        break;
                    case 183: /* invokespecial */
                    case 184: /* invokestatic */
                    case 182: /* invokevirtual */
                        /* Validate and remap class method reference index */
                        bytePtr = byteCode + pc + 1;
                        methodIdx = (int) read_u2((const jubyte **) &bytePtr);
                        tag = pData->constantPool[methodIdx - 1].generic.tag;
                        if ((tag != CONSTANT_ResolvedMethodref) &&
                            (tag != CONSTANT_ResolvedMethodrefErr)) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                           JEMCC_Class_VerifyError, NULL,
                                           "Method op on non-method constant");
                            return JNI_ERR;
                        }
                        bytePtr = byteCode + pc + 1;
                        methodIdx = pData->constantPool[methodIdx - 
                                                    1].res_ref_info.tableIndex;
                        pack_u2((u2) methodIdx, (jubyte **) &bytePtr);
                        break;
                    case 189: /* anewarray */
                    case 192: /* checkcast */
                    case 193: /* instanceof */
                    case 197: /* multianewarray */
                    case 187: /* new */
                        /* Validate and remap class ref (generated) index */
                        bytePtr = byteCode + pc + 1;
                        classIdx = (int) read_u2((const jubyte **) &bytePtr);
                        tag = pData->constantPool[classIdx - 1].generic.tag;
                        if ((tag != CONSTANT_ResolvedClass) &&
                            (tag != CONSTANT_ResolvedClassErr)) {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                             JEMCC_Class_VerifyError, NULL,
                                             "Class op on non-class constant");
                            JEMCC_Free(opCodeMapTable);
                            return JNI_ERR;
                        }
                        iptr = &(pData->constantPool[classIdx - 
                                                1].res_class_info.refTblIndex);
                        if (*iptr < 0) *iptr = classRefCount++;

                        bytePtr = byteCode + pc + 1;
                        pack_u2((u2) *(iptr), (jubyte **) &bytePtr);
                        break;
                    case 18: /* ldc */
                    case 19: /* ldc_w */
                        bytePtr = byteCode + pc + 1;
                        if (opCode == 18) {
                            cnstIdx = (int) read_u1((const jubyte **) &bytePtr);
                        } else {
                            cnstIdx = (int) read_u2((const jubyte **) &bytePtr);
                        }
                        tag = pData->constantPool[cnstIdx - 1].generic.tag;
                        iptr = NULL;
                        if (tag == CONSTANT_Integer) {
                            iptr = &(pData->constantPool[cnstIdx - 
                                                1].integer_info.constTblIndex);
                        } else if (tag == CONSTANT_Float) {
                            iptr = &(pData->constantPool[cnstIdx - 
                                                  1].float_info.constTblIndex);
                        } else if (tag == CONSTANT_ResolvedString) {
                            iptr = &(pData->constantPool[cnstIdx - 
                                             1].res_string_info.constTblIndex);
                        } else {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                       JEMCC_Class_VerifyError, NULL,
                                       "Constant load op on invalid constant");
                            JEMCC_Free(opCodeMapTable);
                            return JNI_ERR;
                        }
                        if (*iptr < 0) *iptr = constantCount++;

                        bytePtr = byteCode + pc + 1;
                        if (opCode == 18) {
                            pack_u1((u1) *(iptr), (jubyte **) &bytePtr);
                        } else {
                            pack_u2((u2) *(iptr), (jubyte **) &bytePtr);
                        }
                        break;
                    case 20: /* ldc2_w */
                        bytePtr = byteCode + pc + 1;
                        cnstIdx = (int) read_u2((const jubyte **) &bytePtr);
                        tag = pData->constantPool[cnstIdx - 1].generic.tag;
                        if (tag == CONSTANT_Long) {
                            iptr = &(pData->constantPool[cnstIdx - 
                                                1].long_info.constTblIndex);
                        } else if (tag == CONSTANT_Double) {
                            iptr = &(pData->constantPool[cnstIdx - 
                                                  1].double_info.constTblIndex);
                        } else {
                            JEMCC_ThrowStdThrowableIdx(env, 
                                  JEMCC_Class_VerifyError, NULL,
                                  "Wide constant load op on invalid constant");
                            JEMCC_Free(opCodeMapTable);
                            return JNI_ERR;
                        }
                        if (*iptr < 0) *iptr = constantCount++;

                        bytePtr = byteCode + pc + 1;
                        pack_u2((u2) *(iptr), (jubyte **) &bytePtr);
                        break;
                    default:
                        /* No modifications to the other opcodes */
                        break;
                }
                /* Mark the opcode and increment */
                opCodeMapTable[pc] = VALID_OPCODE_FLAG;
                pc += pcinc;
            } else {
                /* Complex test cases */
                opCodeMapTable[pc] = VALID_OPCODE_FLAG;
                if (opCode == 196) {
                    /* Wide cases */
                    opCode = byteCode[pc + 1];
                    switch (opCode) {
                        case 132: /* iinc */
                            pcinc = 6;
                            break;
                        case 25: /* aload */
                        case 58: /* astore */
                        case 24: /* dload */
                        case 57: /* dstore */
                        case 23: /* fload */
                        case 56: /* fstore */
                        case 21: /* iload */
                        case 54: /* istore */
                        case 22: /* lload */
                        case 55: /* lstore */
                        case 169: /* ret */
                            pcinc = 4;
                            break;
                        default:
                            /* XXX - Throw the invalid code exception */
                            JEMCC_Free(opCodeMapTable);
                            return JNI_ERR;
                            break;
                    }
                    pc += pcinc;
                } else if (opCode == 170) {
                    /* Table switch - round and jump table list */
                    pc = ((((++pc) + 3) >> 2) << 2) + 4;
                    bytePtr = byteCode + pc;
                    lowIndex = (jint) read_u4((const jubyte **) &bytePtr);
                    highIndex = (jint) read_u4((const jubyte **) &bytePtr);
                    pc += (highIndex - lowIndex + 1) * 4 + 8;
                } else if (opCode == 171) {
                    /* Lookup switch - round and jump table list */
                    pc = ((((++pc) + 3) >> 2) << 2) + 4;
                    bytePtr = byteCode + pc;
                    tblCount = (jint) read_u4((const jubyte **) &bytePtr);
                    pc += tblCount * 8 + 4;
                }
            }
        }
        /* Must be an exact fit */
        if (pc != len) {
            /* XXX - Throw the invalid code exception */
            JEMCC_Free(opCodeMapTable);
            return JNI_ERR;
        }

        /* XXX - insert complete verification algorithm here */

        /* Clean up working definition tables */
        JEMCC_Free(opCodeMapTable);
    }

    /* When passes complete, build compacted class reference table */
    if (classRefCount > 0) {
        classData->classRefs = (JEMCC_Class **) JEMCC_Malloc(env,
                                       classRefCount * sizeof(JEMCC_Class *));
        if (classData->classRefs == NULL) {
            /* XXX - what to clean up here? */
            return JNI_ERR;
        }
        for (i = 0; i < pData->constantPoolCount - 1; i++) {
            cpData = &(pData->constantPool[i]);
            tag = cpData->generic.tag;
            if ((tag != CONSTANT_ResolvedClass) &&
                (tag != CONSTANT_ResolvedClassErr)) continue;
            classIdx = cpData->res_class_info.refTblIndex;
            if (classIdx < 0) continue;
            classData->classRefs[classIdx] = 
                            (JEMCC_Class *) cpData->res_class_info.extClassRef;
        }
    }

    /* As well as the constants required by method code */
    if (constantCount > 0) {
        classData->localConstants = 
                    (JEM_ClassConstant *) JEMCC_Malloc(env,
                                  constantCount * sizeof(JEM_ClassConstant));
        if (classData->localConstants == NULL) {
            /* XXX - what to clean up here? */
            return JNI_ERR;
        }
        for (i = 0; i < pData->constantPoolCount - 1; i++) {
            cpData = &(pData->constantPool[i]);
            tag = cpData->generic.tag;
            if (tag == CONSTANT_Integer) {
                constIdx = cpData->integer_info.constTblIndex;
                if (constIdx < 0) continue;
                classData->localConstants[constIdx].integer_const.value = 
                                                    cpData->integer_info.value;
            } else if (tag == CONSTANT_Float) {
                constIdx = cpData->float_info.constTblIndex;
                if (constIdx < 0) continue;
                classData->localConstants[constIdx].float_const.value = 
                                                    cpData->float_info.value;
            } else if (tag == CONSTANT_Long) {
                constIdx = cpData->long_info.constTblIndex;
                if (constIdx < 0) continue;
                classData->localConstants[constIdx].long_const.value = 
                                                    cpData->long_info.value;
            } else if (tag == CONSTANT_Double) {
                constIdx = cpData->double_info.constTblIndex;
                if (constIdx < 0) continue;
                classData->localConstants[constIdx].double_const.value = 
                                                    cpData->double_info.value;
            } else if (tag == CONSTANT_ResolvedString) {
                constIdx = cpData->res_string_info.constTblIndex;
                if (constIdx < 0) continue;
                classData->localConstants[constIdx].string_const.stringRef =
                                              cpData->res_string_info.stringRef;
                tag = CONSTANT_String;
            } else {
                continue;
            }
            classData->localConstants[constIdx].generic.tag = tag;
        }
    }

    return JNI_OK;
}

/* For reference purposes, here is the opcode table */
/* 0 - "nop" */
/* 1 - "aconst_null" */
/* 2 - "iconst_m1" */
/* 3 - "iconst_0" */
/* 4 - "iconst_1" */
/* 5 - "iconst_2" */
/* 6 - "iconst_3" */
/* 7 - "iconst_4" */
/* 8 - "iconst_5" */
/* 9 - "lconst_0" */
/* 10 - "lconst_1" */
/* 11 - "fconst_0" */
/* 12 - "fconst_1" */
/* 13 - "fconst_2" */
/* 14 - "dconst_0" */
/* 15 - "dconst_1" */
/* 16 - "bipush" */
/* 17 - "sipush" */
/* 18 - "ldc" */
/* 19 - "ldc_w" */
/* 20 - "ldc2_w" */
/* 21 - "iload" */
/* 22 - "lload" */
/* 23 - "fload" */
/* 24 - "dload" */
/* 25 - "aload" */
/* 26 - "iload_0" */
/* 27 - "iload_1" */
/* 28 - "iload_2" */
/* 29 - "iload_3" */
/* 30 - "lload_0" */
/* 31 - "lload_1" */
/* 32 - "lload_2" */
/* 33 - "lload_3" */
/* 34 - "fload_0" */
/* 35 - "fload_1" */
/* 36 - "fload_2" */
/* 37 - "fload_3" */
/* 38 - "dload_0" */
/* 39 - "dload_1" */
/* 40 - "dload_2" */
/* 41 - "dload_3" */
/* 42 - "aload_0" */
/* 43 - "aload_1" */
/* 44 - "aload_2" */
/* 45 - "aload_3" */
/* 46 - "iaload" */
/* 47 - "laload" */
/* 48 - "faload" */
/* 49 - "daload" */
/* 50 - "aaload" */
/* 51 - "baload" */
/* 52 - "caload" */
/* 53 - "saload" */
/* 54 - "istore" */
/* 55 - "lstore" */
/* 56 - "fstore" */
/* 57 - "dstore" */
/* 58 - "astore" */
/* 59 - "istore_0" */
/* 60 - "istore_1" */
/* 61 - "istore_2" */
/* 62 - "istore_3" */
/* 63 - "lstore_0" */
/* 64 - "lstore_1" */
/* 65 - "lstore_2" */
/* 66 - "lstore_3" */
/* 67 - "fstore_0" */
/* 68 - "fstore_1" */
/* 69 - "fstore_2" */
/* 70 - "fstore_3" */
/* 71 - "dstore_0" */
/* 72 - "dstore_1" */
/* 73 - "dstore_2" */
/* 74 - "dstore_3" */
/* 75 - "astore_0" */
/* 76 - "astore_1" */
/* 77 - "astore_2" */
/* 78 - "astore_3" */
/* 79 - "iastore" */
/* 80 - "lastore" */
/* 81 - "fastore" */
/* 82 - "dastore" */
/* 83 - "aastore" */
/* 84 - "bastore" */
/* 85 - "castore" */
/* 86 - "sastore" */
/* 87 - "pop" */
/* 88 - "pop2" */
/* 89 - "dup" */
/* 90 - "dup_x1" */
/* 91 - "dup_x2" */
/* 92 - "dup2" */
/* 93 - "dup2_x1" */
/* 94 - "dup2_x2" */
/* 95 - "swap" */
/* 96 - "iadd" */
/* 97 - "ladd" */
/* 98 - "fadd" */
/* 99 - "dadd" */
/* 100 - "isub" */
/* 101 - "lsub" */
/* 102 - "fsub" */
/* 103 - "dsub" */
/* 104 - "imul" */
/* 105 - "lmul" */
/* 106 - "fmul" */
/* 107 - "dmul" */
/* 108 - "idiv" */
/* 109 - "ldiv" */
/* 110 - "fdiv" */
/* 111 - "ddiv" */
/* 112 - "irem" */
/* 113 - "lrem" */
/* 114 - "frem" */
/* 115 - "drem" */
/* 116 - "ineg" */
/* 117 - "lneg" */
/* 118 - "fneg" */
/* 119 - "dneg" */
/* 120 - "ishl" */
/* 121 - "lshl" */
/* 122 - "ishr" */
/* 123 - "lshr" */
/* 124 - "iushr" */
/* 125 - "lushr" */
/* 126 - "iand" */
/* 127 - "land" */
/* 128 - "ior" */
/* 129 - "lor" */
/* 130 - "ixor" */
/* 131 - "lxor" */
/* 132 - "iinc" */
/* 133 - "i2l" */
/* 134 - "i2f" */
/* 135 - "i2d" */
/* 136 - "l2i" */
/* 137 - "l2f" */
/* 138 - "l2d" */
/* 139 - "f2i" */
/* 140 - "f2l" */
/* 141 - "f2d" */
/* 142 - "d2i" */
/* 143 - "d2l" */
/* 144 - "d2f" */
/* 145 - "i2b" */
/* 146 - "i2c" */
/* 147 - "i2s" */
/* 148 - "lcmp" */
/* 149 - "fcmpl" */
/* 150 - "fcmpg" */
/* 151 - "dcmpl" */
/* 152 - "dcmpg" */
/* 153 - "ifeq" */
/* 154 - "ifne" */
/* 155 - "iflt" */
/* 156 - "ifge" */
/* 157 - "ifgt" */
/* 158 - "ifle" */
/* 159 - "if_icmpeq" */
/* 160 - "if_icmpne" */
/* 161 - "if_icmplt" */
/* 162 - "if_icmpge" */
/* 163 - "if_icmpgt" */
/* 164 - "if_icmple" */
/* 165 - "if_acmpeq" */
/* 166 - "if_acmpne" */
/* 167 - "goto" */
/* 168 - "jsr" */
/* 169 - "ret" */
/* 170 - "tableswitch" */
/* 171 - "lookupswitch" */
/* 172 - "ireturn" */
/* 173 - "lreturn" */
/* 174 - "freturn" */
/* 175 - "dreturn" */
/* 176 - "areturn" */
/* 177 - "return" */
/* 178 - "getstatic" */
/* 179 - "putstatic" */
/* 180 - "getfield" */
/* 181 - "putfield" */
/* 182 - "invokevirtual" */
/* 183 - "invokespecial" */
/* 184 - "invokestatic" */
/* 185 - "invokeinterface" */
/* 186 - invalid */
/* 187 - "new" */
/* 188 - "newarray" */
/* 189 - "anewarray" */
/* 190 - "arraylength" */
/* 191 - "athrow" */
/* 192 - "checkcast" */
/* 193 - "instanceof" */
/* 194 - "monitorenter" */
/* 195 - "monitorexit" */
/* 196 - "wide" */
/* 197 - "multinewarray" */
/* 198 - "ifnull" */
/* 199 - "ifnonnull" */
/* 200 - "goto_w" */
/* 201 - "jsr_w" */
/* 202 - "breakpoint" */
/* 203 - invalid */
/* 204 - invalid */
/* 205 - invalid */
/* 206 - invalid */
/* 207 - invalid */
/* 208 - invalid */
/* 209 - invalid */
/* 210 - invalid */
/* 211 - invalid */
/* 212 - invalid */
/* 213 - invalid */
/* 214 - invalid */
/* 215 - invalid */
/* 216 - invalid */
/* 217 - invalid */
/* 218 - invalid */
/* 219 - invalid */
/* 220 - invalid */
/* 221 - invalid */
/* 222 - invalid */
/* 223 - invalid */
/* 224 - invalid */
/* 225 - invalid */
/* 226 - invalid */
/* 227 - invalid */
/* 228 - invalid */
/* 229 - invalid */
/* 230 - invalid */
/* 231 - invalid */
/* 232 - invalid */
/* 233 - invalid */
/* 234 - invalid */
/* 235 - invalid */
/* 236 - invalid */
/* 237 - invalid */
/* 238 - invalid */
/* 239 - invalid */
/* 240 - invalid */
/* 241 - invalid */
/* 242 - invalid */
/* 243 - invalid */
/* 244 - invalid */
/* 245 - invalid */
/* 246 - invalid */
/* 247 - invalid */
/* 248 - invalid */
/* 249 - invalid */
/* 250 - invalid */
/* 251 - invalid */
/* 252 - invalid */
/* 253 - invalid */
/* 254 - invalid */
/* 255 - invalid */
