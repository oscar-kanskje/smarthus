// bruk 3 Volt
const uint8_t photoresistor_pin = A1;
const int Led = 7;

// 1. photoresistor. When dark, turn led on.
// 2. RFID to open garage with servo, red led if wrong card
// 3. keypad. If combination is correct, step motor open lock. else buzz
// 4. lcd info..
// 5. control everything over bluetooth, with app.

void setup() {
  Serial.begin(9600);
  pinMode(Led, OUTPUT);
  pinMode(photoresistor_pin, INPUT);
}

void loop() {
  Serial.println(analogRead(photoresistor_pin));
  if (analogRead(photoresistor_pin) < 550) {
    digitalWrite(Led, HIGH);
  } else {
    digitalWrite(Led, LOW);
  }
}