
// Pin definitions
//#define PIN_BUTTON_RESET  3
#define PIN_GOAL_HOME     4
#define PIN_GOAL_AWAY     5

// State machine definitions
enum class GoalSensorState { Waiting, Low, High };
//enum class ButtonState { Waiting, Pressed, Released };

// Reset button state machine
// unsigned long resetButtonStartTime = 0;
// unsigned long resetButtonEndTime = 0;
// ButtonState resetButtonState = ButtonState::Waiting;

// Home goal state machine
unsigned long goalHomeSensorStartTime = 0;
unsigned long goalHomeSensorEndTime = 0;
GoalSensorState homeGoalState = GoalSensorState::Waiting;

// Away goal state machine
unsigned long goalAwaySensorStartTime = 0;
unsigned long goalAwaySensorEndTime = 0;
GoalSensorState awayGoalState = GoalSensorState::Waiting;

/*
------------------------------------------------------------------------------
 UART Text Protocol Specification
------------------------------------------------------------------------------

Format:
  ^<TYPE> [<FIELDS>]*<CS>\n

  ^       : Start-of-frame marker  
  <TYPE>  : Two-letter message type  
  <FIELDS>: Space-separated parameters (optional)  
  *<CS>   : 2-digit hexadecimal checksum (LRC of all bytes between '^' and '*')  
  \n      : Line terminator (use '\r\n' if required by host)

------------------------------------------------------------------------------
 Message Types
------------------------------------------------------------------------------

  HG <t>   Home goal scored at time <t> (unsigned long)
  AG <t>   Away goal scored at time <t>
  RS       Reset button short press
  RL       Reset button long press

------------------------------------------------------------------------------
 Examples
------------------------------------------------------------------------------

  ^HG 123456*9A\n   → Home goal at clock 123456, checksum 0x9A  
  ^AG 223789*B2\n   → Away goal at clock 223789, checksum 0xB2  
  ^RS*8F\n          → Reset button short press  
  ^RL*73\n          → Reset button long press  

------------------------------------------------------------------------------
 Debug and Non-Protocol Output
------------------------------------------------------------------------------

  Any line that does not start with '^' or fails checksum validation
  is considered free-form debug output.  
  Example:
      Sensor online at startup
      Clock resync OK

  Receivers may log or ignore such lines.

------------------------------------------------------------------------------
 Notes
------------------------------------------------------------------------------

 - All structured messages include checksums for integrity.  
 - Newline-terminated frames allow simple parsing with readStringUntil('\n').  
 - LRC is the unsigned 8-bit sum of bytes between '^' and '*' inclusive.  
 - ASCII only; no embedded nulls or binary data.  
 - Recommended baud rate: 9600.

------------------------------------------------------------------------------
*/

/*
------------------------------------------------------------------------------
 Function: sendFramedCommand
 Purpose : Build and transmit a framed UART message with checksum.
------------------------------------------------------------------------------

 Arguments:
   cmd  - Command string (the content between '^' and '*')
           Example: "HG 123456" or "RS"

 Description:
   This function computes the LRC checksum of the command string, appends it
   as a two-digit hexadecimal value following a '*', prefixes the message with
   '^', and terminates it with '\n'. It then sends the complete frame using
   Serial.println().

   Example:
     sendFramedCommand("HG 123456");
     → Transmitted: ^HG 123456*9A

 Checksum:
   LRC = (sum of all bytes in cmd) modulo 256
   Output as two uppercase hexadecimal digits.

 Dependencies:
   Requires Serial to be initialized via Serial.begin().

------------------------------------------------------------------------------
*/

void sendFramedCommand(const char *cmd) {
  // Compute 8-bit LRC of command string
  uint8_t lrc = 0;
  for (const char *p = cmd; *p; ++p)
    lrc += static_cast<uint8_t>(*p);

  // Convert LRC to 2-digit uppercase hexadecimal
  char hex[3];
  const char hexmap[] = "0123456789ABCDEF";
  hex[0] = hexmap[(lrc >> 4) & 0xF];
  hex[1] = hexmap[lrc & 0xF];
  hex[2] = '\0';

  // Build final framed message: ^<cmd>*<hex>
  String frame = "^";
  frame += cmd;
  frame += "*";
  frame += hex;

  // Transmit via UART with newline termination
  Serial.println(frame);
}

// Called when a goal is scored for the home team.
void goalHomeScored(unsigned long goalTime) {
  char buf[32];
  snprintf(buf, sizeof(buf), "HG %lu", goalTime);
  sendFramedCommand(buf);
}

// Called when a goal is scored for the away team.
void goalAwayScored(unsigned long goalTime) {
  char buf[32];
  snprintf(buf, sizeof(buf), "AG %lu", goalTime);
  sendFramedCommand(buf);
}

// Called when the game reset button is short pressed.
void resetButtonShortPressed() {
  sendFramedCommand("RS");
}

// Called when the game reset button is long pressed.
void resetButtonLongPressed() {
  sendFramedCommand("RL");
}

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_GOAL_HOME, INPUT);
  pinMode(PIN_GOAL_AWAY, INPUT);
  //pinMode(PIN_BUTTON_RESET, INPUT);

  Serial.begin(9600);
  Serial.println("BOOT");
}

void loop() {
  // Read the value of our reset button
  //int resetButtonPinValue = digitalRead(PIN_BUTTON_RESET);
  // Read the value of our home/away goal sensors
  int homeGoalPinValue = digitalRead(PIN_GOAL_HOME);
  int awayGoalPinValue = digitalRead(PIN_GOAL_AWAY);

  // Read the monotonous clock
  long loopTime = millis();

  // State machine for the reset button.
  // switch (resetButtonState) {
  //   case ButtonState::Waiting:
  //     if (resetButtonPinValue == LOW) {
  //       // Save start of pulse time.
  //       resetButtonStartTime = loopTime;
  //       resetButtonState = ButtonState::Pressed;
  //     }
  //     break;
  //   case ButtonState::Pressed:
  //     if (resetButtonPinValue == HIGH) {
  //       // Save end of pulse time
  //       resetButtonEndTime = loopTime;
  //       resetButtonState = ButtonState::Released;
  //     }
  //     break;
  //   case ButtonState::Released: 
  //     {
  //       // Calculate pulse length
  //       long resetButtonPulseLength = resetButtonEndTime - resetButtonStartTime;
  //       // Reset timers
  //       resetButtonStartTime = 0;
  //       resetButtonEndTime = 0;

  //       Serial.print("Reset Button Pulse Length: ");
  //       Serial.println(resetButtonPulseLength);

  //       if (resetButtonPulseLength > 50 && resetButtonPulseLength < 200) {
  //         // short press
  //         resetButtonShortPressed();
  //       } else if (resetButtonPulseLength > 1000 && resetButtonPulseLength < 3000) {
  //         // long press
  //         resetButtonLongPressed();
  //       }
  //     }
  //   default:
  //     resetButtonState = ButtonState::Waiting;
  // }

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
