/*
 * Goalll
 * Keeps the score of a foosball game.
 */

#include <Arduino.h>

// Set LED_BUILTIN if it is not defined by Arduino framework
#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

#define PIN_GOAL_HOME    3
#define PIN_GOAL_AWAY    4
#define PIN_BUTTON_RESET 5

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

// Team Scores
unsigned int homeScore = 0;
unsigned int awayScore = 0;

void setup() {
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Configure the home/away goal sensor digital pin.
  pinMode(PIN_GOAL_HOME, INPUT);
  pinMode(PIN_GOAL_AWAY, INPUT);
}

void resetGame() {
  homeScore = 0;
  awayScore = 0;
  goalHomeSensorStartTime = 0;
  goalHomeSensorEndTime = 0;
  homeGoalState = GoalSensorState::Waiting;
  goalAwaySensorStartTime = 0;
  goalAwaySensorEndTime = 0;
  awayGoalState = GoalSensorState::Waiting;
  resetButtonStartTime = 0;
  resetButtonEndTime = 0;
  resetButtonState = ButtonState::Waiting;
}

void goalHomeScored() {
    homeScore = homeScore + 1;
}

void goalAwayScored() {
    awayScore = awayScore + 1;
}

void undoLastGoal() {
  // TODO
}

// Main loop.
void loop() {

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

        if (resetButtonPulseLength > 10 && resetButtonPulseLength < 100) {
          // Short push Remove last goal
          undoLastGoal();
        } else if (resetButtonPulseLength > 100 && resetButtonPulseLength < 2000) {
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
        // Reset timers
        goalHomeSensorStartTime = 0;
        goalHomeSensorEndTime = 0;

        if (goalHomeSensorPulseLength > 10 && goalHomeSensorPulseLength < 100) {
          // It's a goal!!!
          goalHomeScored();
        }
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
        // Reset timers
        goalAwaySensorStartTime = 0;
        goalAwaySensorEndTime = 0;

        if (goalAwaySensorPulseLength > 10 && goalAwaySensorPulseLength < 100) {
          // It's a goal!!!
          goalAwayScored();
        }
      }
    default:
      awayGoalState = GoalSensorState::Waiting;
  }    

}