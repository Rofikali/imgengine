#ifndef IMGENGINE_OPCODES_H
#define IMGENGINE_OPCODES_H

/*
 * 🔥 STABLE OPCODES (DO NOT CHANGE ORDER)
 */
typedef enum
{
    OP_RESIZE = 1,
    OP_CROP = 2,
    OP_GRAYSCALE = 3,

    OP_MAX

} img_opcode_t;

#endif

