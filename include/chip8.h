#pragma once
#include <climits>
#include <cstdint>

#include "utility.h"

using byte = uint8_t;

inline constexpr uint16_t FONT_OFFSET = 0x50;
inline constexpr uint16_t ROM_OFFSET = 512;
inline constexpr uint16_t MEMORY_SIZE = 4096;
inline constexpr uint16_t MAX_ROM_SIZE = MEMORY_SIZE - ROM_OFFSET;
inline constexpr byte STACK_DEPTH = 16;
inline constexpr uint16_t CLOCK_SPEED = 540;
inline constexpr uint16_t FRAME_RATE = 60;

struct Chip8
{
    // Memory
    byte memory[MEMORY_SIZE]{};

    // Registers
    byte V[16]{};
    uint16_t I = 0;
    uint16_t PC = ROM_OFFSET;

    // Stack
    uint16_t stack[STACK_DEPTH]{};
    byte stackPointer = 0;

    // Timers
    byte delayTimer = 0;
    byte soundTimer = 0;

    // Input
    uint16_t prevKeyMask = 0;
    uint16_t keyMask = 0;

    // Graphics
    uint64_t screenMask[32]{};
    bool shouldDraw = false;
};

inline constexpr byte SCREEN_WIDTH = sizeof(*Chip8::screenMask) * CHAR_BIT;
inline constexpr byte SCREEN_HEIGHT = ARRAY_COUNT(Chip8::screenMask);

void startROM(Chip8 &context, const char *path);