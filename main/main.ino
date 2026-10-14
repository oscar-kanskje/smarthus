#include <SPI.h>
#include <MFRC522.h>

// Bluetooth module
// VCC: 5 volt
// GND: GND
// RXD: TX1
// TXD: RX1

// RFID module
// VCC : 3.3 volt
// RST : 5
// GND : GND
// MISO: 50
// MOSI: 51
// SCK : 52
// SDA : 53

// Servo motor
// yellow: 7
// red   : 5 volt
// brown : GND

// Photoresistor
// in : 5 volt
// out: A1

// Error led
// in: 41

// Light led
// in: 40

const uint8_t pin_light = 40;
const uint8_t pin_photoresistor = A1;

const uint8_t pin_servo = 7;
const uint8_t pin_err_led = 41;

const uint8_t pin_rfid_rst = 5;
const uint8_t pin_rfid_ss = 53; // sda
const uint8_t pin_rfid_mosi = 51;
const uint8_t pin_rfid_miso = 50;
const uint8_t pin_rfid_sck = 52; 

// these PWM values were found by testing
const uint16_t SERVO_OPEN = 1;
const uint16_t SERVO_CLOSED = 130;

const uint16_t RFID_TIMEOUT = 1000; // ms

struct HomeState {
  bool use_photoresistor;
  bool lights_on;
  bool garage_open;
  bool door_open;
  bool card_error;
  uint32_t rfid_timeout_until;
};

HomeState home_state = {
  .use_photoresistor = true,
  .lights_on = false,
  .garage_open = false,
  .door_open = false,
  .card_error = false,
  .rfid_timeout_until = 0,
};

HardwareSerial &Bluetooth = Serial1;

MFRC522 rfid(pin_rfid_ss, pin_rfid_rst);

MFRC522::Uid valid_uid = {.size = 0};

void setup() {
  Serial.begin(9600);
  Bluetooth.begin(9600);

  pinMode(pin_light, OUTPUT);
  pinMode(pin_photoresistor, INPUT);

  pinMode(pin_servo, OUTPUT);
  pinMode(pin_err_led, OUTPUT);

  SPI.begin();
  rfid.PCD_Init();
  if (rfid.PCD_PerformSelfTest()) {
    Serial.println("(Info) Rfid self check succeeded");
    Serial.println("(Info) Rfid reader initialized");
  }
}

bool uidsEqual(MFRC522::Uid *uid_a,MFRC522::Uid *uid_b) {
  if (uid_a->size != uid_b->size) return false;
  return 0 == memcmp(&uid_a->uidByte, &uid_b->uidByte, uid_a->size);
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
      case '2': home_state.door_open = !home_state.door_open; break;
      case '3': home_state.use_photoresistor = !home_state.use_photoresistor; break;
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
  // digitalWrite(pin_, home_state.door_open ? HIGH : LOW);
  // TODO: write new state to lcd display
}
