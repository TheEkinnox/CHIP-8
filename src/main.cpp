#if defined(_DEBUG) && defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <iostream>

#include "chip8.h"
#include "utility.h"

#include "raylib.h"

int main(int argc, char** argv)
{
#if defined(_DEBUG) && defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    ChangeDirectory(GetApplicationDirectory());

    Chip8 chip8;

    if (argc > 1)
    {
        startROM(chip8, argv[1]);
        return 0;
    }

    char romPath[MAX_OSPATH] = "";

    std::cout << "ROM Path: " << std::flush;
    std::cin >> romPath;

    startROM(chip8, romPath);

    return 0;
}
