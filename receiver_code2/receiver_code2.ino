#include <RH_ASK.h>
#include <LiquidCrystal.h>
#include <SPI.h> // Not actualy used but needed to compile

RH_ASK driver;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup()
{
    Serial.begin(9600); // Debugging only
    lcd.begin(16,2);
    
    if (!driver.init())
         Serial.println("init failed");
}

void loop()
{
  
    uint8_t buf[64];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      
      // Message with a good checksum received, dump it.
      
      Serial.print("Message:  ");
        
      int8_t * realbuf = (int8_t*) buf;
      
      Serial.print("-127: ");
      Serial.print((int) (realbuf[0]));
		
	  Serial.print(", Temp: ");
      Serial.print((int) (realbuf[1]));
		
	  Serial.print("oC, Humidity: ");
      Serial.print((int) (realbuf[2]));
		
	  Serial.println("%");
      lcd.setCursor(0,1);
      lcd.print("Not Ready Yet");
      
     
     
      
    }
}
