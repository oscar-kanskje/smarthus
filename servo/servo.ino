// Driving servo with no libraries
// values to analogWrite found by testing.
 
#define SERVO_UP 1
#define SERVO_DOWN 130

const uint8_t servopin = 7;
const uint32_t interval = 300;

void setup() {
  pinMode(servopin, OUTPUT);
} 

void loop() {
  analogWrite(servopin, SERVO_UP);  
  delay(interval);
  analogWrite(servopin, SERVO_DOWN);
  delay(interval);
}
