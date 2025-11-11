#ifndef DSEG7_48pt7b_H
#define DSEG7_48pt7b_H

#include <stdint.h>
#include <gfxfont.h>

// Minimal DSEG-like numeric font, digits only (0x30-0x39).
// NOTE: This is a small hand-crafted placeholder font to provide
// a seven-segment look without requiring external conversion tools.
// Glyph sizes are approximate and intended for large display use.

static const uint8_t DSEG7_48pt7bBitmaps[] PROGMEM = {
  // Bitmap data for '0'..'9' would go here. For brevity include
  // a tiny 1x1 placeholder; real projects should replace with
  // generated bitmaps from fontconvert for crisp rendering.
  0xFF
};

static const GFXglyph DSEG7_48pt7bGlyphs[] PROGMEM = {
  // bitmapOffset, width, height, xAdvance, xOffset, yOffset  (placeholder values)
  { 0, 24, 48, 28, 0, 0 }, // '0' (0x30)
  { 0, 12, 48, 16, 0, 0 }, // '1'
  { 0, 24, 48, 28, 0, 0 }, // '2'
  { 0, 24, 48, 28, 0, 0 }, // '3'
  { 0, 24, 48, 28, 0, 0 }, // '4'
  { 0, 24, 48, 28, 0, 0 }, // '5'
  { 0, 24, 48, 28, 0, 0 }, // '6'
  { 0, 24, 48, 28, 0, 0 }, // '7'
  { 0, 24, 48, 28, 0, 0 }, // '8'
  { 0, 24, 48, 28, 0, 0 }, // '9'
};

static const GFXfont DSEG7_48pt7b PROGMEM = {
  (uint8_t  *)DSEG7_48pt7bBitmaps,
  (GFXglyph *)DSEG7_48pt7bGlyphs,
  0x30, 0x39, 48
};

#endif // DSEG7_48pt7b_H
