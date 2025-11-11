/*
 * Goalll
 * Keeps the score of a foosball game.
 */

#include <Arduino.h>
#include <WiFi.h>
//#include <lvgl.h>
//#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_idf_version.h>


#include <SPI.h>
#include <Wire.h>
#include "ScoreKeeper.h"
#include "Pins.h"
#include "UARTProtocol.h"


enum class GoalSensorState { Waiting, Low, High };
enum class ButtonState { Waiting, Pressed, Released };

// Maximum score of 10
ScoreKeeper* scoreKeeper;
UARTProtocol* uartProto = nullptr;

// ESP32-S3 RGB Panel wiring
const int BACKLIGHT_PIN = 2;
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  /* de, vsync, hsync, pclk */
  41, 40, 39, 0,
  /* R0..R4 */
  14, 21, 47, 48, 45,
  /* G0..G5 */
   9, 46,  3,  8, 16,  1,
  /* B0..B4 */
  15,  7,  6,  5,  4,
  /* hsync_polarity, hfp, hpw, hbp */
   0, 40, 48, 40,
  /* vsync_polarity, vfp, vpw, vbp */
   0,  1, 31, 13,
  /* pclk_active_neg, prefer_speed (Hz), useBigEndian, de_idle_high, pclk_idle_high, bounce_buffer_size_px */
   1, 15000000, false, 0, 0, 0
);
Arduino_RGB_Display *gfx = new Arduino_RGB_Display(800, 480, rgbpanel, 0 /* rotation */, true /* auto_flush */);

static const char* home_team_name = "HOME";
static const char* away_team_name = "AWAY";

void serialPrintScore() {
  Serial.print("Home: ");
  Serial.println(scoreKeeper->getHomeScore());
  Serial.print("Away: ");
  Serial.println(scoreKeeper->getAwayScore());
  Serial.print("Winning Team: ");
  if (scoreKeeper->isHomeWinning()) {
    Serial.println("Home");
  } else if (scoreKeeper->isAwayWinning()) {
    Serial.println("Away");
  } else {
    Serial.println("Tied");
  }
  Serial.print("Game Over? ");
  Serial.println(scoreKeeper->isGameOver());
}

void drawScoreBoard() {
  // Use gfx (Arduino_GFX) to draw a simple left/right scoreboard
  if (!gfx) return;

  int w = gfx->width();
  int h = gfx->height();

  // Clear background
  gfx->fillScreen(BLACK);

  // Draw dividing line in center
  int midX = w / 2;
  gfx->drawLine(midX, 0, midX, h, WHITE);

  // Area sizes
  int sideW = midX;
  int sideH = h;

  // Team names
  const char* homeName = home_team_name;
  const char* awayName = away_team_name;

  // Scores
  int homeScore = scoreKeeper ? scoreKeeper->getHomeScore() : 0;
  int awayScore = scoreKeeper ? scoreKeeper->getAwayScore() : 0;

  char homeBuf[4];
  char awayBuf[4];
  snprintf(homeBuf, sizeof(homeBuf), "%d", homeScore);
  snprintf(awayBuf, sizeof(awayBuf), "%d", awayScore);

  // Reserve bottom area for team name (approx 10% of height)
  int nameAreaH = max(24, h / 10);

  // Draw team names centered at bottom of each half
  gfx->setTextColor(WHITE, BLACK);
  gfx->setTextSize(2);

  // Helper to draw centered text using getTextBounds
  auto drawCenteredText = [&](const char* str, int areaX, int areaY, int areaW, int areaH, int textSize) {
    gfx->setTextSize(textSize);
    int16_t bx, by; uint16_t bw, bh;
    gfx->getTextBounds(str, 0, 0, &bx, &by, &bw, &bh);
    int cx = areaX + (areaW - (int)bw) / 2;
  // place baseline a bit above the bottom so text isn't clipped
  const int namePaddingBottom = 32; // pixels to lift label above bottom edge (increased to avoid clipping)
  int cy = areaY + areaH - namePaddingBottom;
    // getTextBounds expects x,y as the cursor position; for Adafruit_GFX compatibility
    gfx->setCursor(cx, cy);
    gfx->print(str);
    return bh;
  };

  // Draw HOME name on left (at bottom of the half)
  drawCenteredText(homeName, 0, h - nameAreaH, sideW, nameAreaH, 2);
  // Draw AWAY name on right (at bottom of the half)
  drawCenteredText(awayName, midX, h - nameAreaH, sideW, nameAreaH, 2);

  // We'll render the big score using a scalable 7-segment style digit renderer
  auto drawSevenSeg = [&](int digit, int x, int y, int wbox, int hbox, uint16_t color) {
    // Seven segments: a,b,c,d,e,f,g (clockwise, g is middle)
    static const bool segmap[10][7] = {
      {1,1,1,1,1,1,0}, //0
      {0,1,1,0,0,0,0}, //1
      {1,1,0,1,1,0,1}, //2
      {1,1,1,1,0,0,1}, //3
      {0,1,1,0,0,1,1}, //4
      {1,0,1,1,0,1,1}, //5
      {1,0,1,1,1,1,1}, //6
      {1,1,1,0,0,0,0}, //7
      {1,1,1,1,1,1,1}, //8
      {1,1,1,1,0,1,1}  //9
    };

    int t = max(6, min(wbox, hbox) / 12); // segment thickness
    int pad = t; // padding from edges

    // segment rectangles
    int ax = x + pad;
    int ay = y;
    int aw = wbox - 2 * pad;
    int ah = t;

    int bx = x + wbox - t;
    int by = y + pad;
    int bw = t;
    int bh = (hbox / 2) - pad - t/2;

    int cx = bx;
    int cy = y + hbox/2 + t/2;
    int cw = bw;
    int ch = bh;

    int dx = ax;
    int dy = y + hbox - ah;
    int dw = aw;
    int dh = ah;

    int ex = x;
    int ey = cy;
    int ew = t;
    int eh = ch;

    int fx = x;
    int fy = by;
    int fw = t;
    int fh = bh;

    int gx = ax;
    int gy = y + hbox/2 - ah/2;
    int gw = aw;
    int gh = ah;

    const bool* seg = segmap[digit >=0 && digit <=9 ? digit : 8];

    // draw segments if enabled (use rounded rects for a nicer look)
    if (seg[0]) gfx->fillRoundRect(ax, ay, aw, ah, t/2, color); // a
    if (seg[1]) gfx->fillRoundRect(bx, by, bw, bh, t/2, color); // b
    if (seg[2]) gfx->fillRoundRect(cx, cy, cw, ch, t/2, color); // c
    if (seg[3]) gfx->fillRoundRect(dx, dy, dw, dh, t/2, color); // d
    if (seg[4]) gfx->fillRoundRect(ex, ey, ew, eh, t/2, color); // e
    if (seg[5]) gfx->fillRoundRect(fx, fy, fw, fh, t/2, color); // f
    if (seg[6]) gfx->fillRoundRect(gx, gy, gw, gh, t/2, color); // g
  };

  int scoreAreaH = h - nameAreaH; // area above name

  // function to render a score (handles 0..10)
  auto renderScoreInHalf = [&](int score, int areaX) {
    // If 10, render '1' and '0' side by side; otherwise center single digit
    if (score == 10) {
      // split area into two digit boxes with small gap
      int gap = max(8, sideW / 20);
      int digitW = (sideW - gap) / 2;
      int digitH = scoreAreaH - 8;
      int d1x = areaX + (sideW - (2*digitW + gap))/2;
      int d2x = d1x + digitW + gap;
      int dy = (scoreAreaH - digitH) / 2;
      drawSevenSeg(1, d1x, dy, digitW, digitH, WHITE);
      drawSevenSeg(0, d2x, dy, digitW, digitH, WHITE);
    } else {
      int digit = constrain(score, 0, 9);
      int digitW = min(sideW * 3 / 4, scoreAreaH * 3 / 5);
      int digitH = scoreAreaH - 8;
      if (digitW > sideW - 20) digitW = sideW - 20;
      int dx = areaX + (sideW - digitW) / 2;
      int dy = (scoreAreaH - digitH) / 2;
      drawSevenSeg(digit, dx, dy, digitW, digitH, WHITE);
    }
  };

  // Render home and away scores
  renderScoreInHalf(homeScore, 0);
  renderScoreInHalf(awayScore, midX);

  // Draw reset button in center of screen: circular button with circular-arrow icon
  auto drawResetButton = [&](int cx, int cy, int radius) {
    // button fill and outline
    uint16_t fillColor = gfx->color565(30, 30, 30); // dark grey
    uint16_t outline = WHITE;
    gfx->fillCircle(cx, cy, radius, fillColor);
    gfx->drawCircle(cx, cy, radius, outline);

    // Draw a single 7/8ths arc (315 degrees) and an arrowhead at the end
    // Start angle: -45 degrees, sweep to 270 degrees (-45 + 315 = 270)
    int startDeg = -45;
    int sweep = 315;
    int endDeg = startDeg + sweep;
    const int step = 4;
    int lastX = 0, lastY = 0;
    bool first = true;
    float r = radius * 0.50f;
    for (int a = startDeg; a <= endDeg; a += step) {
      float rad = a * PI / 180.0f;
      int x = cx + (int)(r * cosf(rad));
      int y = cy + (int)(r * sinf(rad));
      if (!first) gfx->drawLine(lastX, lastY, x, y, WHITE);
      lastX = x; lastY = y; first = false;
    }

    // Draw arrowhead at final angle (endDeg)
    float a = endDeg * PI / 180.0f;
    float tipXf = cx + r * cosf(a);
    float tipYf = cy + r * sinf(a);
    // direction vector
    float dx = cosf(a);
    float dy = sinf(a);
    float px = -dy; // perp
    float py = dx;
    float arrowLen = radius * 0.26f;
    float arrowWid = radius * 0.16f;
    int tipX = (int)tipXf;
    int tipY = (int)tipYf;
    int b1x = (int)(tipXf - dx * arrowLen + px * arrowWid);
    int b1y = (int)(tipYf - dy * arrowLen + py * arrowWid);
    int b2x = (int)(tipXf - dx * arrowLen - px * arrowWid);
    int b2y = (int)(tipYf - dy * arrowLen - py * arrowWid);
    gfx->fillTriangle(tipX, tipY, b1x, b1y, b2x, b2y, WHITE);
  };

  // Place the reset button in the absolute center with smaller radius
  //drawResetButton(midX, h / 2, 48);

  // Force flush if using auto_flush=false; safe to call anyway
  // gfx->display() is not necessary for auto_flush=true
}

void goalHomeScored(unsigned long goalTime) {
  scoreKeeper->scoreHomeGoal(goalTime);
  Serial.println("Home Scored");
  serialPrintScore();
  drawScoreBoard();
}

void goalAwayScored(unsigned long goalTime) {
  scoreKeeper->scoreAwayGoal(goalTime);
  Serial.println("Away Scored");
  serialPrintScore();
  drawScoreBoard();
}

void undoLastGoal() {
  scoreKeeper->removeLastGoal();
  Serial.println("Undid Last Goal.");
  serialPrintScore();
  drawScoreBoard();
}

void resetGame() {
  scoreKeeper = new ScoreKeeper(10);

  Serial.println("Game Reset");
  drawScoreBoard();
}


void setup() {
  delay(2000);

  // Setup Serial Comms (protocol and logging use 9600)
  Serial.begin(9600);
  Serial.println("setup()");

  //Serial1.begin(9600, SERIAL_8N1, 36, 37); 
  //Serial1.begin(9600);

  // UART protocol on the (single) Serial port — Serial is initialized above.
  // Parser will not reconfigure the Serial port; it expects the stream to be
  // initialized by the application.
  uartProto = new UARTProtocol(Serial);
  uartProto->setHomeGoalCallback(goalHomeScored);
  uartProto->setAwayGoalCallback(goalAwayScored);
  uartProto->setResetShortCallback(resetGame);
  uartProto->setResetLongCallback(resetGame);

  // Setup the display
  // u8g2.begin();
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN, HIGH); 
  gfx->begin();
  gfx->fillScreen(BLACK);

  // Initialize game state and draw initial scoreboard
  resetGame();
}


// Main loop.
void loop() {
  ////Serial.println("loop()");

  // Read the monotonous clock
  //long loopTime = millis();
  // Poll UART protocol parser (non-blocking)
  if (uartProto) uartProto->poll();
}