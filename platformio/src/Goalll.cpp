/*
 * Goalll
 * Keeps the score of a foosball game.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include "ScoreKeeper.h"

// Set LED_BUILTIN if it is not defined by Arduino framework
#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

#define PIN_GOAL_HOME    4
#define PIN_GOAL_AWAY    5
#define PIN_BUTTON_RESET 9
#define PIN_DISPLAY_CS   7
#define PIN_DISPLAY_DC   12
#define PIN_DISPLAY_RST  13

enum class GoalSensorState { Waiting, Low, High };
enum class ButtonState { Waiting, Pressed, Released };

// Reset button state machine
unsigned long resetButtonStartTime = 0;
unsigned long resetButtonEndTime = 0;
ButtonState resetButtonState = ButtonState::Waiting;

// Home goal state machine
unsigned long goalHomeSensorStartTime = 0;
unsigned long goalHomeSensorEndTime = 0;
GoalSensorState homeGoalState = GoalSensorState::Waiting;

// Away goal state machine
unsigned long goalAwaySensorStartTime = 0;
unsigned long goalAwaySensorEndTime = 0;
GoalSensorState awayGoalState = GoalSensorState::Waiting;

// Maximum score of 10
ScoreKeeper* scoreKeeper;

// OLED Display
// Using HW SPI
U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ PIN_DISPLAY_CS, /* dc=*/ PIN_DISPLAY_DC, /* reset=*/ PIN_DISPLAY_RST);

static const char* home_team_name = "HOME";
static const char* away_team_name = "AWAY";

void serialPrintScore() {
  Serial.print("Home: ");
  Serial.print(scoreKeeper->getHomeScore());
  Serial.print(" Away: ");
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

void resetGame() {
  scoreKeeper = new ScoreKeeper(10);
  goalHomeSensorStartTime = 0;
  goalHomeSensorEndTime = 0;
  homeGoalState = GoalSensorState::Waiting;
  goalAwaySensorStartTime = 0;
  goalAwaySensorEndTime = 0;
  awayGoalState = GoalSensorState::Waiting;
  resetButtonStartTime = 0;
  resetButtonEndTime = 0;
  resetButtonState = ButtonState::Waiting;

  Serial.println("Game Reset");
}


void drawScoreBoard() {
  u8g2.clearBuffer();

  const char* home_score_formatted = scoreKeeper->getHomeScoreFormatted();
  const char* away_score_formatted = scoreKeeper->getAwayScoreFormatted();
 
  // Write Score
  u8g2.setFont(u8g2_font_logisoso58_tr);
  u8g2.setFontDirection(0); // Left to Right
  u8g2.setFontPosBaseline();
  //u8g2.drawButtonUTF8(home_score_start_pos_x, score_baseline_pos_y, U8G2_BTN_INV,0, 0, 0, scoreKeeper->getHomeScoreFormatted());
  
  u8g2_uint_t display_height_half = u8g2.getDisplayHeight() / 2;
  u8g2_uint_t display_width_half = u8g2.getDisplayWidth() / 2;

  // Calculate Position for Score #s
  u8g2_uint_t home_score_start_pos_x = (display_width_half - u8g2.getStrWidth(home_score_formatted)) / 2;
  u8g2_uint_t away_score_start_pos_x = display_width_half + (display_width_half - u8g2.getStrWidth(away_score_formatted)) / 2;
  u8g2_uint_t score_baseline_pos_y = u8g2.getDisplayHeight() - ((u8g2.getDisplayHeight() - u8g2.getFontAscent()) / 2);
  
  // Draw the Score #s
  u8g2.drawStr(home_score_start_pos_x, score_baseline_pos_y, home_score_formatted);
  u8g2.drawStr(away_score_start_pos_x, score_baseline_pos_y, away_score_formatted);

  // Draw Home Team Name
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setFontDirection(1); // Top to down
  u8g2_uint_t home_team_name_start_y = (u8g2.getDisplayHeight() - u8g2.getStrWidth(home_team_name)) / 2;
  u8g2.drawStr(0,home_team_name_start_y, home_team_name);
  
  // Draw Away Team Name
  //u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setFontDirection(3); // Down to Top
  u8g2_uint_t away_team_name_start_y = u8g2.getDisplayHeight() - (u8g2.getDisplayHeight() - u8g2.getStrWidth(away_team_name)) / 2;
  u8g2_uint_t away_team_name_start_x = u8g2.getDisplayWidth() - u8g2.getFontAscent() + 5;
  u8g2.drawStr(away_team_name_start_x,away_team_name_start_y, away_team_name);

  u8g2.sendBuffer();
}

void goalHomeScored(unsigned long goalTime) {
  scoreKeeper->scoreHomeGoal(goalTime);
  Serial.println("Home Scored");
  serialPrintScore();
}

void goalAwayScored(unsigned long goalTime) {
  scoreKeeper->scoreAwayGoal(goalTime);
  Serial.println("Away Scored");
  serialPrintScore();
}

void undoLastGoal() {
  scoreKeeper->removeLastGoal();
  Serial.println("Undid Last Goal.");
  serialPrintScore();
}

void setup() {
  // Configure the home/away goal sensor digital pin.
  pinMode(PIN_GOAL_HOME, INPUT);
  pinMode(PIN_GOAL_AWAY, INPUT);
  pinMode(PIN_BUTTON_RESET, INPUT_PULLDOWN);
  pinMode(PIN_DISPLAY_CS, OUTPUT);
  pinMode(PIN_DISPLAY_DC, OUTPUT);
  pinMode(PIN_DISPLAY_RST, OUTPUT);

  delay(2000);

  // Setup Serial Comms
  Serial.begin(115200);
  Serial.println("setup()");

  // Setup the display
  u8g2.begin();

  // Reset Game
  resetGame();
}


// Main loop.
void loop() {

  drawScoreBoard();

  // Read the value of our reset button
  int resetButtonPinValue = digitalRead(PIN_BUTTON_RESET);
  // Read the value of our home/away goal sensors
  int homeGoalPinValue = digitalRead(PIN_GOAL_HOME);
  int awayGoalPinValue = digitalRead(PIN_GOAL_AWAY);
  // Read the monotonous clock
  long loopTime = millis();

  // State machine for the reset button.
  switch (resetButtonState) {
    case ButtonState::Waiting:
      if (resetButtonPinValue == LOW) {
        // Save start of pulse time.
        resetButtonStartTime = loopTime;
        resetButtonState = ButtonState::Pressed;
      }
      break;
    case ButtonState::Pressed:
      if (resetButtonPinValue == HIGH) {
        // Save end of pulse time
        resetButtonEndTime = loopTime;
        resetButtonState = ButtonState::Released;
      }
      break;
    case ButtonState::Released: 
      {
        // Calculate pulse length
        long resetButtonPulseLength = resetButtonEndTime - resetButtonStartTime;
        // Reset timers
        resetButtonStartTime = 0;
        resetButtonEndTime = 0;

        Serial.print("Reset Button Pulse Length: ");
        Serial.println(resetButtonPulseLength);

        if (resetButtonPulseLength > 50 && resetButtonPulseLength < 200) {
          // Short push Remove last goal
          undoLastGoal();
        } else if (resetButtonPulseLength > 1000 && resetButtonPulseLength < 3000) {
          // Long push Reset game.
          resetGame();
        }
      }
    default:
      resetButtonState = ButtonState::Waiting;
  }

  // State machine for the home goal sensor.
  switch (homeGoalState) {
    case GoalSensorState::Waiting:
      if (homeGoalPinValue == LOW) {
        // Save start of pulse time.
        goalHomeSensorStartTime = loopTime;
        homeGoalState = GoalSensorState::Low;
      }
      break;
    case GoalSensorState::Low:
      if (homeGoalPinValue == HIGH) {
        // Save end of pulse time
        goalHomeSensorEndTime = loopTime;
        homeGoalState = GoalSensorState::High;
      }
      break;
    case GoalSensorState::High: 
      {
        // Calculate pulse length
        long goalHomeSensorPulseLength = goalHomeSensorEndTime - goalHomeSensorStartTime;

        Serial.print("Goal Home Pulse Length: ");
        Serial.println(goalHomeSensorPulseLength);

        if (goalHomeSensorPulseLength > 10 && goalHomeSensorPulseLength < 100) {
          // It's a goal!!!
          goalHomeScored(goalHomeSensorStartTime);
        }

        // Reset timers
        goalHomeSensorStartTime = 0;
        goalHomeSensorEndTime = 0;
      }
    default:
      homeGoalState = GoalSensorState::Waiting;
  }

  // State machine for the away goal sensor.
  switch (awayGoalState) {
    case GoalSensorState::Waiting:
      if (awayGoalPinValue == LOW) {
        // Save start of pulse time.
        goalAwaySensorStartTime = loopTime;
        awayGoalState = GoalSensorState::Low;
      }
      break;
    case GoalSensorState::Low:
      if (awayGoalPinValue == HIGH) {
        // Save end of pulse time
        goalAwaySensorEndTime = loopTime;
        awayGoalState = GoalSensorState::High;
      }
      break;
    case GoalSensorState::High: 
      {
        // Calculate pulse length
        long goalAwaySensorPulseLength = goalAwaySensorEndTime - goalAwaySensorStartTime;

        Serial.print("Goal Away Pulse Length: ");
        Serial.println(goalAwaySensorPulseLength);

        if (goalAwaySensorPulseLength > 10 && goalAwaySensorPulseLength < 100) {
          // It's a goal!!!
          goalAwayScored(goalAwaySensorStartTime);
        }

        // Reset timers
        goalAwaySensorStartTime = 0;
        goalAwaySensorEndTime = 0;
      }
    default:
      awayGoalState = GoalSensorState::Waiting;
  }    

}