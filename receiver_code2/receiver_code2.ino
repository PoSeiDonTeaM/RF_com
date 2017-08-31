#include <RH_ASK.h>
#include <LiquidCrystal.h>
#include <SPI.h> // Not actualy used but needed to compile

RH_ASK driver;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup()
{
    Serial.begin(9600); // Debugging only
    lcd.begin(16,2);
    lcd.setCursor(1,0);
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
    lcd.setCursor(1,0);
    uint8_t buf[12];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      int i;
      // Message with a good checksum received, dump it.
      Serial.print("Message: ");
      Serial.println((char*)buf);
      lcd.print((char*)buf);
              
    }
}
