#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Default LCD address (will be updated after scan)
uint8_t lcdAddress = 0x27; 
LiquidCrystal_I2C lcd(lcdAddress, 16, 2); // 16x2 LCD

// Function to scan for I2C devices
uint8_t scanI2CAddress() {
  Serial.println("Scanning for I2C devices...");
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at 0x");
      Serial.println(address, HEX);
      return address; // Return first found address
    }
  }
  Serial.println("No I2C devices found!");
  return 0;
}

// lcd found address
void lcd_found() {
  // Scan and update LCD address
  uint8_t foundAddress = scanI2CAddress();
  if (foundAddress != 0) {
    lcdAddress = foundAddress;
    lcd = LiquidCrystal_I2C(lcdAddress, 16, 2);
    lcd.init();       // Initialize LCD
    lcd.backlight();  // Turn on backlight

    lcd.setCursor(0, 0);
    lcd.print("LCD Test OK!");
    lcd.setCursor(0, 1);
    lcd.print("Addr: 0x");
    lcd.print(lcdAddress, HEX);
  } else {
    Serial.println("Connect LCD and reset.");
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  Wire.begin(); // Mega uses SDA=20, SCL=21 automatically
  lcd_found();
}

void loop() {
  // Blink backlight to show it's alive
/*  if (lcdAddress != 0) {
    lcd.noBacklight();
    delay(500);
    lcd.backlight();
    delay(500);
  }
*/
 lcd.setCursor(0, 0);
  lcd.print("Vi presenterer    ");
  lcd.setCursor(0, 1);
  lcd.print("Oscar smarthus!");

}

