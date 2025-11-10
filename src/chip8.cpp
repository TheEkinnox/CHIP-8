#include "chip8.h"

#include <cstring>
#include <format>
#include <fstream>
#include <iostream>

#include "raylib.h"
#include "utility.h"

#include "chip8_opcodes.h"

inline constexpr byte font[]
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

//   CHIP-8  |  QWERTY
//  1 2 3 C  |  1 2 3 4
//  4 5 6 D  |  Q W E R
//  7 8 9 E  |  A S D F
//  A 0 B F  |  Z X C V
inline constexpr uint8_t keyMap[16] = {
    KEY_X, KEY_ONE, KEY_TWO, KEY_THREE,
    KEY_Q, KEY_W, KEY_E, KEY_A,
    KEY_S, KEY_D, KEY_Z, KEY_C,
    KEY_FOUR, KEY_R, KEY_F, KEY_V
};

inline constexpr byte PIXEL_SIZE = 16;

static Sound beepSound{};

namespace
{
    struct OpCodeInfo
    {
        uint16_t mask;
        uint16_t opcode;
        Chip8OpCodeFunc func;
    };
}

inline static OpCodeInfo opCodeTable[] = {
#define CHIP8_REGISTER_OPCODES
    #include "chip8_opcodes.h"
#undef CHIP8_REGISTER_OPCODES
};

static void init(Chip8 &context)
{
    context = {};
    memcpy(context.memory + FONT_OFFSET, font, sizeof(font));
    srand(static_cast<int>(time(nullptr)));
}

static bool loadROM(Chip8 &context, const char *filename)
{
    std::cout << "Loading ROM @ " << filename << std::endl;

    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to load ROM - Unable to open file" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size > MAX_ROM_SIZE)
    {
        std::cerr << "Failed to load ROM - file is too large (" << size << "/" << MAX_ROM_SIZE << ")" << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(context.memory + ROM_OFFSET), size);

    std::cout << "Successfully loaded " << size << " bytes ROM" << std::endl;
    return true;
}

static void loadBeep()
{
    if (beepSound.frameCount > 0)
        return;

    constexpr int SAMPLE_RATE = 44100;
    constexpr int FRAME_COUNT = 4410;
    constexpr float FREQUENCY = 440;

    int8_t data[FRAME_COUNT]{};
    for (int i = 0; i < FRAME_COUNT; i++) {
        data[i] = static_cast<int8_t>(sinf(2 * PI * FREQUENCY * static_cast<float>(i) / SAMPLE_RATE) * 127);
    }

    const Wave beepWave{FRAME_COUNT, SAMPLE_RATE, sizeof(*data) * CHAR_BIT, 1, data};
    beepSound = LoadSoundFromWave(beepWave);
}

static void unloadBeep()
{
    if (beepSound.frameCount == 0)
        return;

    UnloadSound(beepSound);
}

static uint16_t readOpCode(const Chip8 &context)
{
    if (context.PC >= MEMORY_SIZE)
        return 0xffff;

    return context.memory[context.PC] << 8 | context.memory[context.PC + 1];
}

static void execute(Chip8 &context, uint16_t opcode)
{
    for (const auto& [mask, expectedOpcode, func] : opCodeTable)
    {
        if ((opcode & mask) == expectedOpcode)
        {
            func(context, opcode);
            return;
        }
    }

    std::cerr << "Unknown opcode " << std::format("0x{:04x}", opcode) << std::endl;
    context.PC = MEMORY_SIZE; // Crash interpreter on unknown opcode
}

static void draw(const Chip8 &context)
{
    for (byte y = 0; y < SCREEN_HEIGHT; ++y)
    {
        const auto& line = context.screenMask[y];
        for (byte x = 0; x < SCREEN_WIDTH; ++x)
        {
            const int posX = x * PIXEL_SIZE;
            const int posY = y * PIXEL_SIZE;
            DrawRectangle(posX, posY, PIXEL_SIZE, PIXEL_SIZE, (line >> (SCREEN_WIDTH - x - 1) & 0b01) ? WHITE : BLACK);
        }
    }
}

static void updateTimers(Chip8 &context)
{
    if (context.delayTimer > 0)
        --context.delayTimer;

    if (context.soundTimer > 0)
    {
        if (!IsSoundPlaying(beepSound))
            PlaySound(beepSound);

        --context.soundTimer;
    }
    else if (IsSoundPlaying(beepSound))
    {
        StopSound(beepSound);
    }
}

static void updateInputs(Chip8 &context)
{
    context.prevKeyMask = context.keyMask;
    for (byte i = 0; i < 16; ++i)
    {
        context.keyMask = com_setBits(context.keyMask, i, 1, IsKeyDown(keyMap[i]));
    }
}

static void frame(Chip8 &context)
{
    for (uint16_t i = 0; i < CLOCK_SPEED / FRAME_RATE; ++i)
    {
        const uint16_t opcode = readOpCode(context);
        context.PC += 2;

        execute(context, opcode);
    }

    if (context.shouldDraw)
    {
        BeginDrawing();
        draw(context);
        EndDrawing();
        context.shouldDraw = false;
    }
    else
    {
        PollInputEvents();
    }

    updateTimers(context);
    updateInputs(context);
}

void startROM(Chip8 &context, const char *path)
{
    init(context);

    if (!loadROM(context, path))
        return;

    constexpr char prefix[] = "CHIP-8 | ";
    char displayName[MAX_OSPATH + sizeof(prefix)];

    com_strcpy(displayName, prefix);
    com_strcpy(displayName + sizeof(prefix) - 1, sizeof(displayName) - sizeof(prefix), com_getFilename(path));

    constexpr int WINDOW_WIDTH = SCREEN_WIDTH * PIXEL_SIZE;
    constexpr int WINDOW_HEIGHT = SCREEN_HEIGHT * PIXEL_SIZE;

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, displayName);
    InitAudioDevice();
    loadBeep();

    SetWindowFocused();
    SetTargetFPS(FRAME_RATE);

    bool shouldClose = WindowShouldClose();
    while (!shouldClose)
    {
        frame(context);
        shouldClose = WindowShouldClose() || context.PC >= MEMORY_SIZE;
    }

    unloadBeep();
    CloseAudioDevice();
    CloseWindow();
}