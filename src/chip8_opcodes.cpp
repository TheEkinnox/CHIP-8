#define CHIP8_DECLARE_OPCODES
#include "chip8_opcodes.h"
#undef CHIP8_DECLARE_OPCODES

#include <cstdlib>

#include "utility.h"

byte readX(const uint16_t opcode)
{
    return opcode >> 8 & 0x0f;
}

byte& getVx(Chip8& context, const uint16_t opcode)
{
    return context.V[readX(opcode)];
}

byte readY(const uint16_t opcode)
{
    return opcode >> 4 & 0x0f;
}

byte& getVy(Chip8& context, const uint16_t opcode)
{
    return context.V[readY(opcode)];
}

byte& getVf(Chip8& context)
{
    return context.V[0x0f];
}

byte readN(const uint16_t opcode)
{
    return opcode & 0x0f;
}

byte readNN(const uint16_t opcode)
{
    return opcode & 0xff;
}

uint16_t readNNN(const uint16_t opcode)
{
    return opcode & 0x0fff;
}

CHIP8_OPCODE_SIG(op_clearDisplay)
{
    memset(context.screenMask, 0, sizeof(context.screenMask));
    context.shouldDraw = true;
}

CHIP8_OPCODE_SIG(op_return)
{
    --context.stackPointer;
    context.PC = context.stack[context.stackPointer];
}

CHIP8_OPCODE_SIG(op_jmp)
{
    context.PC = readNNN(opcode);
}

CHIP8_OPCODE_SIG(op_call)
{
    context.stack[context.stackPointer] = context.PC;
    ++context.stackPointer;
    op_jmp(context, opcode);
}

CHIP8_OPCODE_SIG(op_skipIfVxEqN)
{
    if (getVx(context, opcode) == readNN(opcode))
        context.PC += sizeof(opcode);
}

CHIP8_OPCODE_SIG(op_skipIfVxNeqN)
{
    if (getVx(context, opcode) != readNN(opcode))
        context.PC += sizeof(opcode);
}

CHIP8_OPCODE_SIG(op_skipIfVxEqVy)
{
    if (getVx(context, opcode) == getVy(context, opcode))
        context.PC += sizeof(opcode);
}

CHIP8_OPCODE_SIG(op_setVxToN)
{
    getVx(context, opcode) = readNN(opcode);
}

CHIP8_OPCODE_SIG(op_addNToVx)
{
    getVx(context, opcode) += readNN(opcode);
}


CHIP8_OPCODE_SIG(op_setVxToVy)
{
    byte& vx = getVx(context, opcode);
    vx = getVy(context, opcode);
}

CHIP8_OPCODE_SIG(op_vxOrEqVy)
{
    byte& vx = getVx(context, opcode);
    vx |= getVy(context, opcode);
    getVf(context) = 0;
}

CHIP8_OPCODE_SIG(op_vxAndEqVy)
{
    byte& vx = getVx(context, opcode);
    vx &= getVy(context, opcode);
    getVf(context) = 0;
}

CHIP8_OPCODE_SIG(op_vxXorEqVy)
{
    byte& vx = getVx(context, opcode);
    vx ^= getVy(context, opcode);
    getVf(context) = 0;
}

CHIP8_OPCODE_SIG(op_addVyToVx)
{
    byte& vx = getVx(context, opcode);
    const byte baseVx = vx;
    const byte vy = getVy(context, opcode);

    vx += vy;
    getVf(context) = UINT8_MAX - baseVx < vy ? 1 : 0;
}

CHIP8_OPCODE_SIG(op_subVyFromVx)
{
    byte& vx = getVx(context, opcode);
    const byte baseVx = vx;
    const byte vy = getVy(context, opcode);

    vx -= vy;
    getVf(context) = vy > baseVx ? 0 : 1;
}

CHIP8_OPCODE_SIG(op_rshiftVyInVx)
{
    byte& vx = getVx(context, opcode);
    vx = getVy(context, opcode);
    const byte baseVx = vx;

    vx >>= 1;
    getVf(context) = baseVx & 0b1;
}

CHIP8_OPCODE_SIG(op_setVxToVyMinVx)
{
    byte& vx = getVx(context, opcode);
    const byte baseVx = vx;
    const byte vy = getVy(context, opcode);

    vx = vy - vx;
    getVf(context) = baseVx > vy ? 0 : 1;
}

CHIP8_OPCODE_SIG(op_lshiftVyInVx)
{
    byte& vx = getVx(context, opcode);
    vx = getVy(context, opcode);
    const byte baseVx = vx;

    vx <<= 1;
    getVf(context) = baseVx >> 7 & 0b1;
}

CHIP8_OPCODE_SIG(op_skipIfVxNeqVy)
{
    if (getVx(context, opcode) != getVy(context, opcode))
        context.PC += sizeof(opcode);
}

CHIP8_OPCODE_SIG(op_setIToN)
{
    context.I = readNNN(opcode);
}

CHIP8_OPCODE_SIG(op_jmpFromV0)
{
    context.PC = static_cast<uint16_t>(context.V[0]) + readNNN(opcode);
}

CHIP8_OPCODE_SIG(op_rand)
{
    const byte n = readNN(opcode);
    getVx(context, opcode) = rand() & n;
}

CHIP8_OPCODE_SIG(op_draw)
{
    const byte vx = getVx(context, opcode) % SCREEN_WIDTH;
    const byte vy = getVy(context, opcode) % SCREEN_HEIGHT;
    const byte n = readN(opcode);

    constexpr byte MAX_SPRITE_HEIGHT = 0xf;
    byte sprite[MAX_SPRITE_HEIGHT]{};
    memcpy(sprite, context.memory + context.I, n);

    bool anyDisabled = false;
    for (byte y = 0; y < n && vy + y < SCREEN_HEIGHT; ++y)
    {
        auto& line = context.screenMask[vy + y];
        const auto& spriteRow = sprite[y];
        for (byte x = 0; x < 8 && vx + x < SCREEN_WIDTH; ++x)
        {
            const byte screenShift = SCREEN_WIDTH - 1 - (vx + x);
            const byte prevState = line >> screenShift & 0b1;
            const byte newState = prevState ^ (spriteRow >> (7 - x) & 0b1);
            anyDisabled |= prevState && !newState;

            line = com_setBits(line, screenShift, 1, newState);
        }
    }

    getVf(context) = anyDisabled ? 1 : 0;
    context.shouldDraw = true;
}

CHIP8_OPCODE_SIG(op_skipIfKeyDown)
{
    if (context.keyMask & (1 << (getVx(context, opcode) & 0xf)))
        context.PC += sizeof(opcode);
}

CHIP8_OPCODE_SIG(op_skipIfKeyUp)
{
    if (!(context.keyMask & (1 << (getVx(context, opcode) & 0xf))))
        context.PC += sizeof(opcode);
}

CHIP8_OPCODE_SIG(op_setVxToDelay)
{
    getVx(context, opcode) = context.delayTimer;
}

CHIP8_OPCODE_SIG(op_waitForKey)
{
    if (context.prevKeyMask <= context.keyMask)
    {
        context.PC -= sizeof(opcode);
        return;
    }

    byte pressedKey;
    for (pressedKey = 0; pressedKey < static_cast<byte>(sizeof(context.keyMask) * CHAR_BIT); ++pressedKey)
    {
        if ((context.keyMask >> pressedKey & 0b1) != (context.prevKeyMask >> pressedKey & 0b1))
            break;
    }

    getVx(context, opcode) = pressedKey;
}

CHIP8_OPCODE_SIG(op_setDelayToVx)
{
    context.delayTimer = getVx(context, opcode);
}

CHIP8_OPCODE_SIG(op_setSoundToVx)
{
    context.soundTimer = getVx(context, opcode);
}

CHIP8_OPCODE_SIG(op_addVxToI)
{
    context.I += getVx(context, opcode);
}

CHIP8_OPCODE_SIG(op_setIToCharAtVx)
{
    context.I = FONT_OFFSET + getVx(context, opcode) & 0x0f;
}

CHIP8_OPCODE_SIG(op_setIToVxBCD)
{
    byte vx = getVx(context, opcode);
    byte scale = 100;

    byte *out = context.memory + context.I;
    for (byte i = 0; i < 3; ++i)
    {
        const byte digit = vx / scale;
        vx -= digit * scale;
        scale /= 10;
        *out = digit;
        ++out;
    }
}

CHIP8_OPCODE_SIG(op_dumpXRegisters)
{
    const byte count = readX(opcode) + 1;
    memcpy(context.memory + context.I, context.V, count);
    context.I += count;
}

CHIP8_OPCODE_SIG(op_loadXRegisters)
{
    const byte count = readX(opcode) + 1;
    memcpy(context.V, context.memory + context.I, count);
    context.I += count;
}
