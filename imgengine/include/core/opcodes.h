/* include/core/opcodes.h */

#ifndef IMGENGINE_OPCODES_H
#define IMGENGINE_OPCODES_H

#include <stdint.h>

/*
 * 🔥 BUILTIN OPCODES (DENSE, FAST)
 */
typedef enum
{
    OP_NOP = 0,

    OP_RESIZE = 1,
    OP_CROP,
    OP_GRAYSCALE,
    OP_BRIGHTNESS,

    OP_MAX_BUILTIN // last builtin
} img_opcode_t;

/*
 * 🔥 CUSTOM OPCODE RANGE (EXTENSIBLE ABI)
 *
 * Anything >= this is plugin / user-defined
 */
#define OP_CUSTOM_BASE 0x1000

/*
 * 🔥 HARD LIMIT (SAFETY)
 */
#define OP_MAX_LIMIT 0xFFFF

#endif