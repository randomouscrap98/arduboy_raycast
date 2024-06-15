#pragma once

using uint24_t = __uint24;

// Generated with ardugotools on 2024-06-14T23:14:49-04:00

// Image info for "tilesheet"
// Raycast frame bytes: 172, mipmap widths: 32,16,8,4
constexpr uint24_t tilesheet       = 0x000000;
constexpr uint8_t  tilesheetFrames = 4;
// Image info for "spritesheet"
// Raycast frame bytes: 172, mipmap widths: 32,16,8,4
constexpr uint24_t spritesheet       = 0x0002B0;
constexpr uint24_t spritesheetMask   = 0x000560;
constexpr uint8_t  spritesheetFrames = 4;

// FX addresses (only really used for initialization)
constexpr uint16_t FX_DATA_PAGE = 0xFFF7;
constexpr uint24_t FX_DATA_BYTES = 2064;

// Helper macro to initialize fx, call in setup()
#define FX_INIT() FX::begin(FX_DATA_PAGE)
