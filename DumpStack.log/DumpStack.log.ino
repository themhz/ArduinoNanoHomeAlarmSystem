#include <Keypad.h>

const int switchPin = 2;  // Pin connected to the magnetic contact switch
const int ledPin = 3;     // Pin connected to the LED
const int buzzerPin = 4;  // Pin connected to the transistor base

// Keypad setup
const byte ROWS = 4;  // Four rows
const byte COLS = 4;  // Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {5, 6, 7, 8};  // Connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 10, 11, 12};  // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const String correctCode = "526996";  // Set your correct code here
String enteredCode = "";
unsigned long codeEntryStart = 0;
const unsigned long codeEntryTimeout = 20000;  // 20 seconds
const unsigned long armDelay = 20000;  // 20 seconds to leave the room after arming
bool doorOpened = false;  // Flag to track door state
bool codeEnteredCorrectly = false;  // Flag to track if the correct code was entered
bool buzzerActive = false;  // Flag to track if the buzzer should continue to sound
bool alarmArmed = false;  // Flag to track if the alarm is armed
bool leavingRoom = false;  // Flag to track if we are leaving the room

void setup() {
  pinMode(switchPin, INPUT_PULLUP);  // Set switchPin as input with internal pull-up resistor
  pinMode(ledPin, OUTPUT);           // Set ledPin as output
  pinMode(buzzerPin, OUTPUT);        // Set buzzerPin as output
  digitalWrite(ledPin, LOW);         // Initialize LED as off
  digitalWrite(buzzerPin, LOW);      // Initialize buzzer as off
  Serial.begin(115200);
  Serial.println("System initialized.");
}

void loop() {
  int switchState = digitalRead(switchPin);  // Read the state of the switch
  Serial.print("Switch state: ");
  Serial.println(switchState);

  if (alarmArmed) {
    if (leavingRoom) {
      if (millis() - codeEntryStart >= armDelay) {
        leavingRoom = false;  // Finish the leaving process
        Serial.println("Alarm is fully armed now.");
      }
    } else {
      if (switchState == LOW) {  // Switch closed (door closed)
        digitalWrite(ledPin, HIGH);  // Turn LED on
      } else {  // Switch open (door open)
        digitalWrite(ledPin, LOW);   // Turn LED off
        if (!doorOpened) {  // If door was just opened
          doorOpened = true;
          codeEntryStart = millis();  // Start code entry timer
          Serial.println("Door opened.");
        }
      }

      handleCodeEntry();  // Handle code entry process

      if (buzzerActive) {
        playPoliceSiren();  // Play police siren sound if buzzer is active
      } else if (doorOpened && !codeEnteredCorrectly && (millis() - codeEntryStart >= codeEntryTimeout)) {
        buzzerActive = true;  // Activate buzzer
        Serial.println("Buzzer activated.");
      }
    }
  } else {
    noTone(buzzerPin);  // Ensure buzzer is off if alarm is not armed
    digitalWrite(ledPin, LOW);   // Turn LED off
  }

  checkArmAlarm();  // Check if the alarm should be armed

  delay(50);  // Small delay for debouncing
}

void handleCodeEntry() {
  char key = keypad.getKey();
  
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);  // Print the key that was pressed
    enteredCode += key;
    Serial.print("Entered code: ");
    Serial.println(enteredCode);
    if (enteredCode.length() == correctCode.length()) {
      if (enteredCode == correctCode) {
        Serial.println("Correct code entered!");
        enteredCode = "";  // Clear entered code
        codeEntryStart = 0;  // Reset code entry timer
        noTone(buzzerPin);  // Stop buzzer
        codeEnteredCorrectly = true;  // Set code entered flag
        buzzerActive = false;  // Deactivate buzzer
        alarmArmed = false;  // Disarm the alarm
        Serial.println("Alarm disarmed.");
      } else {
        Serial.println("Incorrect code. Try again.");
        enteredCode = "";  // Clear entered code
        codeEntryStart = millis();  // Restart code entry timer
      }
    }
  }
}

void checkArmAlarm() {
  char key = keypad.getKey();

  if (key == 'A') {
    alarmArmed = true;
    leavingRoom = true;
    codeEntryStart = millis();  // Start the delay for leaving the room
    doorOpened = false;
    codeEnteredCorrectly = false;
    buzzerActive = false;
    enteredCode = "";
    Serial.println("Alarm armed. You have 20 seconds to leave the room.");
    digitalWrite(ledPin, HIGH);  // Turn on LED during the 20 seconds leaving period
  }
}

void playPoliceSiren() {
  static unsigned long lastChangeTime = 0;
  static int frequency = 2000;
  static bool increasing = true;

  if (millis() - lastChangeTime > 50) {  // Change frequency every 50ms
    lastChangeTime = millis();
    if (increasing) {
      frequency += 20;
      if (frequency >= 2200) {
        increasing = false;
      }
    } else {
      frequency -= 20;
      if (frequency <= 2000) {
        increasing = true;
      }
    }
    tone(buzzerPin, frequency);
  }
}
