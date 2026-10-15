#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

// ---------------- PIN DEFINITIONS ----------------
#define SERVO_PIN 10
#define SERVO_LOCK_POS 20
#define SERVO_UNLOCK_POS 90
#define BUZZER_PIN 12

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- SERVO ----------------
Servo lockServo;

// ---------------- KEYPAD ----------------
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;

byte rowPins[KEYPAD_ROWS] = {5, 4, 3, 2};
byte colPins[KEYPAD_COLS] = {9, 8, 7, 6};

char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  KEYPAD_ROWS,
  KEYPAD_COLS
);

// ---------------- EEPROM ----------------
// Addresses 0-3: four PIN digits
// Address 4: PIN initialization marker
// Address 5: lock state (1 = locked, 0 = unlocked)

const int PIN_MARKER_ADDR = 4;
const int LOCK_STATE_ADDR = 5;
const byte PIN_MARKER = 0xA5;

bool doorLocked = true;

// ---------------- PIN STORAGE ----------------

bool hasCode() {
  return EEPROM.read(PIN_MARKER_ADDR) == PIN_MARKER;
}

String getStoredCode() {
  String code = "";

  for (int i = 0; i < 4; i++) {
    code += char(EEPROM.read(i));
  }

  return code;
}

void saveCode(String code) {
  for (int i = 0; i < 4; i++) {
    EEPROM.update(i, code[i]);
  }

  // Write marker last, after the PIN has been saved.
  EEPROM.update(PIN_MARKER_ADDR, PIN_MARKER);
}

void saveLockState(bool locked) {
  doorLocked = locked;
  EEPROM.update(LOCK_STATE_ADDR, locked ? 1 : 0);
}

// ---------------- LOCKING ----------------

void lockDoor() {
  lockServo.write(SERVO_LOCK_POS);
  saveLockState(true);
}

void unlockDoor() {
  lockServo.write(SERVO_UNLOCK_POS);
  saveLockState(false);
}

// ---------------- KEYPAD INPUT ----------------

String inputCode(String prompt) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(prompt);
  lcd.setCursor(0, 1);
  lcd.print(" ");

  String code = "";

  while (code.length() < 4) {
    char key = keypad.getKey();

    if (key >= '0' && key <= '9') {
      code += key;
      lcd.print('*');
    }

    delay(20);
  }

  return code;
}

// ---------------- CHANGE / CREATE PIN ----------------

bool setNewCode() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter new PIN");

  String newCode = inputCode("New PIN:");

  String confirmCode = inputCode("Confirm PIN:");

  if (newCode == confirmCode) {
    saveCode(newCode);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PIN saved!");
    delay(1500);

    return true;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PIN mismatch!");
  lcd.setCursor(0, 1);
  lcd.print("Try again");
  delay(1500);

  return false;
}

// ---------------- BUZZER ----------------

void soundAlarm() {
  tone(BUZZER_PIN, 1000);
  delay(700);
  noTone(BUZZER_PIN);
}

// ---------------- LOCKED SCREEN ----------------

void safeLockedLogic() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[LOCKED]");
  lcd.setCursor(0, 1);
  lcd.print("Enter PIN:");

  String enteredCode = inputCode("Enter PIN:");

  if (enteredCode == getStoredCode()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access granted");
    delay(1000);

    unlockDoor();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door unlocked");
    lcd.setCursor(0, 1);
    lcd.print("# = lock");
    delay(1200);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access denied!");
    soundAlarm();
    delay(1000);
  }
}

// ---------------- UNLOCKED SCREEN ----------------

void safeUnlockedLogic() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[UNLOCKED]");
  lcd.setCursor(0, 1);
  lcd.print("#=Lock A=PIN");

  while (!doorLocked) {
    char key = keypad.getKey();

    if (key == '#') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Locking door...");
      delay(700);

      lockDoor();
      return;
    }

    if (key == 'A') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Verify old PIN");

      String oldCode = inputCode("Old PIN:");

      if (oldCode == getStoredCode()) {
        bool updated = false;

        while (!updated) {
          updated = setNewCode();
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PIN updated");
        delay(1000);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Wrong old PIN");
        soundAlarm();
        delay(1000);
      }

      // Return to the unlocked screen.
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("[UNLOCKED]");
      lcd.setCursor(0, 1);
      lcd.print("#=Lock A=PIN");
    }

    delay(20);
  }
}

// ---------------- SETUP ----------------

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(BUZZER_PIN, OUTPUT);

  lockServo.attach(SERVO_PIN);

  lcd.setCursor(0, 0);
  lcd.print("Welcome!");
  lcd.setCursor(0, 1);
  lcd.print("Arduino Safe");
  delay(1500);

  // On first use, create a PIN.
  if (!hasCode()) {
    bool created = false;

    while (!created) {
      created = setNewCode();
    }

    lockDoor();
  } else {
    // Restore the saved lock state.
    byte storedState = EEPROM.read(LOCK_STATE_ADDR);

    // If the state is invalid, default to locked.
    if (storedState != 0 && storedState != 1) {
      lockDoor();
    } else if (storedState == 1) {
      doorLocked = true;
      lockServo.write(SERVO_LOCK_POS);
    } else {
      doorLocked = false;
      lockServo.write(SERVO_UNLOCK_POS);
    }
  }

  Serial.println("Arduino Safe Ready");
}

// ---------------- MAIN LOOP ----------------

void loop() {
  if (doorLocked) {
    safeLockedLogic();
  } else {
    safeUnlockedLogic();
  }
}