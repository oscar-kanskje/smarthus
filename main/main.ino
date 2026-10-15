#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

// LCD screen
// VCC: 5 volt
// GND: GND
// SDA: SDA (20) used directly by library
// SCL: SCL (21) used directly by library

// Bluetooth module
// VCC: 5 volt
// GND: GND
// RXD: TX1
// TXD: RX1

// RFID module
// VCC : 3.3 volt
// RST : 6
// GND : GND
// MISO: 50
// MOSI: 51
// SCK : 52
// SDA : 53

// Servo motor
// yellow: 7
// red   : 5 volt
// brown : GND

// Stepper motor
// VCC: 5 volt
// GND: GND
// IN1: 8
// IN2: 9
// IN3: 10
// IN4: 11

// Buzzer
// in: 12

// Photoresistor
// in : 5 volt
// out: A1

// Error led
// in: 41

// Light led
// in: 40



// these PWM values were found by testing
#define SERVO_OPEN   1
#define SERVO_CLOSED 130

#define STEPPER_STEPS 2048

// card cannot be read faster than this
#define RFID_TIMEOUT 1000

// how long before the lcd screen updates
#define LCD_TIMEOUT 50

// light pins
const uint8_t pin_light = 40;
const uint8_t pin_photoresistor = A1;

// garage/rfid pins
const uint8_t pin_servo = 7;
const uint8_t pin_err_led = 41;
const uint8_t pin_rfid_rst = 5;
const uint8_t pin_rfid_ss = 53; // sda
const uint8_t pin_rfid_mosi = 51;
const uint8_t pin_rfid_miso = 50;
const uint8_t pin_rfid_sck = 52; 

// door/keypad pins
const uint8_t pin_stepper_in1 = 8;
const uint8_t pin_stepper_in2 = 9;
const uint8_t pin_stepper_in3 = 10;
const uint8_t pin_stepper_in4 = 11;
const uint8_t pin_buzzer = 12;


struct HomeState {
  bool use_photoresistor;
  bool lights_on;
  bool garage_open;
  bool doorlock_open;
  bool card_error;
  int16_t current_lock_step;
  uint32_t rfid_timeout_until;
  uint32_t lcd_timeout_until;
};

//------------ Global variables
// this is 'volatile' since it is written from
// the stepper motor timer interrupt
volatile HomeState home_state = {
  .use_photoresistor = true,
  .lights_on = false,
  .garage_open = false,
  .doorlock_open = false,
  .card_error = false,
  .current_lock_step = 0,
  .rfid_timeout_until = 0,
  .lcd_timeout_until = 0,
};

// First argument is address??
// address of what??
LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial &Bluetooth = Serial1;

MFRC522 rfid(pin_rfid_ss, pin_rfid_rst);

MFRC522::Uid valid_uid = {.size = 0};

bool bt_get_state = false;

//----------- End of global variables


// This is a timer interrupt handler
// used for moving the stepper motor
ISR(TIMER1_COMPA_vect) {
  if (home_state.doorlock_open) {
    if (home_state.current_lock_step < STEPPER_STEPS / 4) {
      home_state.current_lock_step += 1;
    } else {
      return;
    }
  } else {
    if (home_state.current_lock_step > 0) {
      home_state.current_lock_step -= 1;
    } else {
      return;
    }
  }
  // I have changed the write sequence from
  // 1010 0110 0101 1001 (in the Stepper library)
  // to
  // 1100 0110 0011 1001
  // like swapping in2 and in3
  // it can move backwards now
  switch (home_state.current_lock_step % 4) {
    case 0:  // 1100
      digitalWrite(pin_stepper_in1, HIGH);
      digitalWrite(pin_stepper_in2, HIGH);
      digitalWrite(pin_stepper_in3, LOW);
      digitalWrite(pin_stepper_in4, LOW);
      break;
    case 1:  // 0110
      digitalWrite(pin_stepper_in1, LOW);
      digitalWrite(pin_stepper_in2, HIGH);
      digitalWrite(pin_stepper_in3, HIGH);
      digitalWrite(pin_stepper_in4, LOW);
      break;
    case 2:  // 0011
      digitalWrite(pin_stepper_in1, LOW);
      digitalWrite(pin_stepper_in2, LOW);
      digitalWrite(pin_stepper_in3, HIGH);
      digitalWrite(pin_stepper_in4, HIGH);
      break;
    case 3: // 1001
      digitalWrite(pin_stepper_in1, HIGH);
      digitalWrite(pin_stepper_in2, LOW);
      digitalWrite(pin_stepper_in3, LOW);
      digitalWrite(pin_stepper_in4, HIGH);
      break;
  }
}

bool uidsEqual(MFRC522::Uid *uid_a,MFRC522::Uid *uid_b) {
  if (uid_a->size != uid_b->size) return false;
  return 0 == memcmp(&uid_a->uidByte, &uid_b->uidByte, uid_a->size);
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  Serial.println("(Info) Lcd started");

  lcd.setCursor(0,0);
  lcd.print("SYSTEM");
  lcd.setCursor(0,1);
  lcd.print("STARTING...");

  Bluetooth.begin(9600);

  pinMode(pin_light, OUTPUT);
  pinMode(pin_photoresistor, INPUT);

  pinMode(pin_servo, OUTPUT);
  pinMode(pin_err_led, OUTPUT);

  pinMode(pin_stepper_in1, OUTPUT);
  pinMode(pin_stepper_in2, OUTPUT);
  pinMode(pin_stepper_in3, OUTPUT);
  pinMode(pin_stepper_in4, OUTPUT);

  // Setup timer interrupt for stepper motor
  TCCR1A = 0;
  TCCR1B = bit(WGM12) | bit(CS10);
  // increasing this decreases stepper speed (it is almost max speen now) 
  OCR1A  = 30000;
  TIMSK1 = bit(OCIE1A);

  SPI.begin();
  rfid.PCD_Init();
  if (rfid.PCD_PerformSelfTest()) {
    Serial.println("(Info) Rfid self check succeeded");
    Serial.println("(Info) Rfid reader initialized");
  }
}


void loop() {

  if (Bluetooth.available()) {
    const char byte = Bluetooth.read();
    Serial.print("(Info) Received bluetooth command '");
    Serial.print(byte);
    Serial.println("'");
    switch (byte) {
      case '0': home_state.lights_on = !home_state.lights_on; break;
      case '1': home_state.garage_open = !home_state.garage_open; break;
      case '2': home_state.doorlock_open = !home_state.doorlock_open; break;
      case '3': home_state.use_photoresistor = !home_state.use_photoresistor; break;
      case '4': bt_get_state = true; break;
      default: Serial.println("(Info) Last bluetooth command unrecognized");
    }
  }

  if (rfid.PICC_IsNewCardPresent() && home_state.rfid_timeout_until < millis()) {
    if (rfid.PICC_ReadCardSerial()) {
      if (valid_uid.size == 0) {
        valid_uid = rfid.uid;
        Serial.println("(Info) Registered vaild rfid card");
      } else if (uidsEqual(&rfid.uid, &valid_uid)) {
        home_state.garage_open = !home_state.garage_open;
        Serial.println("(Info) Rfid uid valid");
      } else {
        home_state.card_error = true;
        Serial.println("(Info) Rfid uid incorrect");
      }
      home_state.rfid_timeout_until = millis() + RFID_TIMEOUT;
    }
  }

  if (home_state.use_photoresistor) {
    if (analogRead(pin_photoresistor) < 800) {
      home_state.lights_on = true;
    } else {
      home_state.lights_on = false;
    }
  }

  //// ---- Send signals to components based on home_state ------
   
  if (home_state.card_error) {
    if (home_state.rfid_timeout_until > millis()) {
      digitalWrite(pin_err_led, HIGH);
    } else {
      digitalWrite(pin_err_led, LOW);
      home_state.card_error = false;
    }
  }

  digitalWrite(pin_light, home_state.lights_on ? HIGH : LOW);
  analogWrite(pin_servo, home_state.garage_open ? SERVO_OPEN : SERVO_CLOSED);

  // this condition makes sure the lcd isn't refreshed all the time
  if (millis() > home_state.lcd_timeout_until) {
    // maybe use our own symbols instead of esoteric codes
    char statestr1[16] = "PR:x L:x G:x D:x";
    char statestr2[16] = "CR:x";
    statestr1[3]  = home_state.use_photoresistor ? 't' : 'f';
    statestr1[7]  = home_state.lights_on ? 't' : 'f';
    statestr1[11] = home_state.garage_open ? 'o' : 'c';
    statestr1[15] = home_state.doorlock_open ? 'o' : 'c';
    statestr2[3]  = home_state.card_error ? 't' : 'f';
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(statestr1);
    lcd.setCursor(0,1);
    lcd.print(statestr2);

    // maybe write nicer state messages over bluetooth
    if (bt_get_state) {
      Bluetooth.print(statestr1);
      Bluetooth.print(" ");
      Bluetooth.print(statestr2);
      Bluetooth.print("\n");
      bt_get_state = false;
    }

    home_state.lcd_timeout_until = millis() + LCD_TIMEOUT;
  }
}
