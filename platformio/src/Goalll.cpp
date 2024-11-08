/*
 * Goalll
 * Keeps the score of a foosball game.
 */

#include <Arduino.h>

// Set LED_BUILTIN if it is not defined by Arduino framework
#ifndef LED_BUILTIN
    #define LED_BUILTIN 2
#endif

#define PIN_GOAL_HOME 3
#define PIN_GOAL_AWAY 4

#define GOAL_STATE_WAITING 0
#define GOAL_STATE_LOW 1
#define GOAL_STATE_HIGH 2

// Home goal state machine
unsigned long goalHomeSensorStartTime = 0;
unsigned long goalHomeSensorEndTime = 0;
unsigned int homeGoalState = GOAL_STATE_WAITING;

// Away goal state machine
unsigned long goalAwaySensorStartTime = 0;
unsigned long goalAwaySensorEndTime = 0;
unsigned int awayGoalState = GOAL_STATE_WAITING;

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

void goalHomeScored() {
    homeScore = homeScore + 1;
}

void goalAwayScored() {
    awayScore = awayScore + 1;
}

// Main loop.
void loop() {

  // Read the value of our home/away goal sensors
  int homeGoalPinValue = digitalRead(PIN_GOAL_HOME);
  int awayGoalPinValue = digitalRead(PIN_GOAL_AWAY);
  // Read the monotonous clock
  long loopTime = millis();

  // State machine for the home goal sensor.
  switch (homeGoalState) {
    case GOAL_STATE_WAITING:
      if (homeGoalPinValue == LOW) {
        // Save start of pulse time.
        goalHomeSensorStartTime = loopTime;
        homeGoalState = GOAL_STATE_LOW;
      }
      break;
    case GOAL_STATE_LOW:
      if (homeGoalPinValue == HIGH) {
        // Save end of pulse time
        goalHomeSensorEndTime = loopTime;
        homeGoalState = GOAL_STATE_HIGH;
      }
      break;
    case GOAL_STATE_HIGH: 
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
      homeGoalState = GOAL_STATE_WAITING;
  }

  // State machine for the away goal sensor.
  switch (awayGoalState) {
    case GOAL_STATE_WAITING:
      if (awayGoalPinValue == LOW) {
        // Save start of pulse time.
        goalAwaySensorStartTime = loopTime;
        awayGoalState = GOAL_STATE_LOW;
      }
      break;
    case GOAL_STATE_LOW:
      if (awayGoalPinValue == HIGH) {
        // Save end of pulse time
        goalAwaySensorEndTime = loopTime;
        awayGoalState = GOAL_STATE_HIGH;
      }
      break;
    case GOAL_STATE_HIGH: 
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
      awayGoalState = GOAL_STATE_WAITING;
  }    

}