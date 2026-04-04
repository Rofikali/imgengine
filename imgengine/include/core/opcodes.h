/* include/core/opcodes.h */

#ifndef IMGENGINE_OPCODES_H
#define IMGENGINE_OPCODES_H

/*
 * 🔥 GLOBAL OPCODE CONTRACT
 * (ABI-STABLE — NEVER BREAK)
 */

/* Base operations */
#define OP_RESIZE 0x01
#define OP_CROP 0x02
#define OP_GRAYSCALE 0x03

/*
 * 🔥 INTERNAL (SEPARABLE / HOT PATH)
 * NOT exposed to API layer
 */
#define OP_RESIZE_H 0x10
#define OP_RESIZE_V 0x11

/*
 * 🔥 FUTURE SAFE RANGE (L8 foresight)
 */
#define OP_CUSTOM_BASE 0x1000

#endif