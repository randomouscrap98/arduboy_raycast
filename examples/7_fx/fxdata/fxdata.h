#pragma once

using uint24_t = __uint24;

// Generated with ardugotools on 2024-06-13T14:33:22-04:00

// Image info for "tilesheet"
// NOTE: raycast tiles stored as mipmaps, 32 8 byte strips per tile
constexpr uint24_t tilesheet       = 0x000000;
constexpr uint8_t  tilesheetFrames = 4;
constexpr uint16_t tilesheetWidth  = 32;
constexpr uint16_t tilesheetHeight = 32;
// Image info for "spritesheet"
// NOTE: raycast tiles stored as mipmaps, 32 8 byte strips per tile
constexpr uint24_t spritesheet       = 0x000400;
constexpr uint24_t spritesheetMask   = 0x000800;
constexpr uint8_t  spritesheetFrames = 4;
constexpr uint16_t spritesheetWidth  = 32;
constexpr uint16_t spritesheetHeight = 32;

// FX addresses (only really used for initialization)
constexpr uint16_t FX_DATA_PAGE = 0xFFF4;
constexpr uint24_t FX_DATA_BYTES = 3072;

// Helper macro to initialize fx, call in setup()
#define FX_INIT() FX::begin(FX_DATA_PAGE)
