#if defined(CHIP8_REGISTER_OPCODES)
#define CHIP8_OPCODE( mask, code, func) { mask, code, func },
#else
#include <type_traits>
#include "chip8.h"
#define CHIP8_OPCODE_SIG( func ) void func( Chip8& context, [[maybe_unused]] uint16_t opcode )
#define CHIP8_OPCODE( mask, code, func ) CHIP8_OPCODE_SIG( func );
using Chip8OpCodeFunc = CHIP8_OPCODE_SIG((*));
#endif

CHIP8_OPCODE( 0xffff, 0x00E0, op_clearDisplay )
CHIP8_OPCODE( 0xffff, 0x00EE, op_return )
CHIP8_OPCODE( 0xf000, 0x1000, op_jmp )
CHIP8_OPCODE( 0xf000, 0x2000, op_call )
CHIP8_OPCODE( 0xf000, 0x3000, op_skipIfVxEqN )
CHIP8_OPCODE( 0xf000, 0x4000, op_skipIfVxNeqN )
CHIP8_OPCODE( 0xf00f, 0x5000, op_skipIfVxEqVy )
CHIP8_OPCODE( 0xf000, 0x6000, op_setVxToN )
CHIP8_OPCODE( 0xf000, 0x7000, op_addNToVx )
CHIP8_OPCODE( 0xf00f, 0x8000, op_setVxToVy )
CHIP8_OPCODE( 0xf00f, 0x8001, op_vxOrEqVy )
CHIP8_OPCODE( 0xf00f, 0x8002, op_vxAndEqVy )
CHIP8_OPCODE( 0xf00f, 0x8003, op_vxXorEqVy )
CHIP8_OPCODE( 0xf00f, 0x8004, op_addVyToVx )
CHIP8_OPCODE( 0xf00f, 0x8005, op_subVyFromVx )
CHIP8_OPCODE( 0xf00f, 0x8006, op_rshiftVyInVx )
CHIP8_OPCODE( 0xf00f, 0x8007, op_setVxToVyMinVx )
CHIP8_OPCODE( 0xf00f, 0x800E, op_lshiftVyInVx )
CHIP8_OPCODE( 0xf00f, 0x9000, op_skipIfVxNeqVy )
CHIP8_OPCODE( 0xf000, 0xA000, op_setIToN )
CHIP8_OPCODE( 0xf000, 0xB000, op_jmpFromV0 )
CHIP8_OPCODE( 0xf000, 0xC000, op_rand )
CHIP8_OPCODE( 0xf000, 0xD000, op_draw )
CHIP8_OPCODE( 0xf0ff, 0xE09E, op_skipIfKeyDown )
CHIP8_OPCODE( 0xf0ff, 0xE0A1, op_skipIfKeyUp )
CHIP8_OPCODE( 0xf0ff, 0xF007, op_setVxToDelay )
CHIP8_OPCODE( 0xf0ff, 0xF00A, op_waitForKey )
CHIP8_OPCODE( 0xf0ff, 0xF015, op_setDelayToVx )
CHIP8_OPCODE( 0xf0ff, 0xF018, op_setSoundToVx )
CHIP8_OPCODE( 0xf0ff, 0xF01E, op_addVxToI )
CHIP8_OPCODE( 0xf0ff, 0xF029, op_setIToCharAtVx )
CHIP8_OPCODE( 0xf0ff, 0xF033, op_setIToVxBCD )
CHIP8_OPCODE( 0xf0ff, 0xF055, op_dumpXRegisters )
CHIP8_OPCODE( 0xf0ff, 0xF065, op_loadXRegisters )

#undef CHIP8_OPCODE