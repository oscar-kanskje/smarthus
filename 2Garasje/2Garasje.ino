/*
Bruk RFID-leseren og en servomotor. 
Servoen skal åpne garasjeporten om riktig kort blir lest. 
Om feil kort blir lest skal en rød Led tennes
*/


/*Led (Red)*/
// OUTPUT : 7


/*Servo*/
// GND (black)      : GND
// VCC (red)        : 5V
// Signal (orange)  : 8


/*RFID leser*/
// VCC  : 3.3V 
// RST  : A1
// GND  : GND
// MISO : 50
// MOSI : 51
// SCK  : 52
// SDA  : 53


String validUID;
const int Led = 7;

#include <Servo.h>
Servo myServo;

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53 // The Arduino Mega pin connected to the SS pin of RC522 module
#define RST_PIN A1 // The Arduino Mega pin connected to the RST pin of RC522 module

MFRC522 rfid(SS_PIN, RST_PIN);


String check_card() {
  String UID;
  Serial.print("Tap Card: ");
  while (1) {
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      //Serial.print("RFID/NFC Tag Type: ");
      //Serial.println(rfid.PICC_GetTypeName(piccType));

      // print NUID in Serial Monitor in the hex format
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        UID += rfid.uid.uidByte[i];
      }

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
      Serial.println();
      return UID;
    }
  } 
  } 
} 

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  pinMode(Led, OUTPUT);
  myServo.attach(8);

  Serial.println();
  Serial.println("Save new card");
  validUID = check_card();
}


int pos = 0;

void loop() {
  String newUID = check_card();
  if (newUID == validUID) {
    Serial.println("Correct");
    if (pos == 0) {
      pos = 90;
    } else {
      pos = 0;
    }

// move the servo
  Serial.println(pos);
  myServo.write(pos);
// Led show when new card is not the same
  } else {
    Serial.println("Incorrect");
    for (int i = 0; i < 3; i++) {
      digitalWrite(Led, HIGH);
      delay(200);
      digitalWrite(Led, LOW);
      delay(200);
    }
  }

}


