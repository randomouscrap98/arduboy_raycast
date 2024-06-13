#pragma once

using uint24_t = __uint24;

// Generated with ardugotools on 2024-06-13T01:19:39-04:00

// Image info for "tilesheet"
// NOTE: offsets for raycast stripes are: 4, 2, 2, 1, 1, 1, 1, 1,  => 13
constexpr uint24_t tilesheet       = 0x000000;
constexpr uint24_t tilesheetMask   = 0x000680;
constexpr uint8_t  tilesheetFrames = 4;
constexpr uint16_t tilesheetWidth  = 32;
constexpr uint16_t tilesheetHeight = 32;

// FX addresses (only really used for initialization)
constexpr uint16_t FX_DATA_PAGE = 0xFFF3;
constexpr uint24_t FX_DATA_BYTES = 3328;

// Helper macro to initialize fx, call in setup()
#define FX_INIT() FX::begin(FX_DATA_PAGE)
