
// VCC: 5 volt
// GND: ... GND
// RXD: TX1
// TXD: RX1

HardwareSerial &Bluetooth = Serial1;

enum Command {
  CMD_LIGHTS_ON    = '0',
  CMD_LIGHTS_OFF   = '1',
  CMD_UNLOCK_DOOR  = '2',
  CMD_LOCK_DOOR    = '3',
  CMD_OPEN_GARAGE  = '4',
  CMD_CLOSE_GARAGE = '5',
};


void setup() {
  Serial.begin(9600);
  Bluetooth.begin(9600);
  Serial.println("bluetooth running...");
}

void loop() {
  if (Bluetooth.available()) {
    const char byte = Bluetooth.read();
    switch (byte) {
      case CMD_LIGHTS_ON:
        Serial.println("lights on");
        break;
      case CMD_LIGHTS_OFF:
        Serial.println("lights off");
        break;
      case CMD_UNLOCK_DOOR:
        Serial.println("door unlocked");
        break;
      case CMD_LOCK_DOOR:
        Serial.println("door locked");
        break;
      case CMD_OPEN_GARAGE:
        Serial.println("garage opened");
        break;
      case CMD_CLOSE_GARAGE:
        Serial.println("garage closed");
        break;
      default:
        Serial.print("Unknown command '");
        Serial.print(byte);
        Serial.println("'");
    }
  }
}
