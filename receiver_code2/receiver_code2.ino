#include <RH_ASK.h>
#include <LiquidCrystal.h>
#include <SPI.h> // Not actualy used but needed to compile

RH_ASK driver;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

byte customChar[8] = {
  0b00111,
  0b00101,
  0b00111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

void setup()
{
    Serial.begin(9600); // Debugging only
    lcd.begin(16,2);
    lcd.createChar(0, customChar);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Waiting for");
    lcd.setCursor(0,1);
    lcd.print("signal...");
    Serial.println("wait signal...");

    
    if (!driver.init())
         Serial.println("init failed");  
}

int page = 0;

void loop()
{
    static int oldpage = page;
    static bool dataexists = false;
    page = ( (millis()/1000 / 8) % 2 ) ? 0 : 1;
  
    uint8_t buf[64];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) { // Non-blocking
      // Message with a good checksum received, dump it.
      
      int8_t * realbuf = (int8_t*) buf;
      float r_temperature = (realbuf[1] + realbuf[2]/128.0);
      float r_level = realbuf[4] + realbuf[5]/128.0;
      float r_battery = realbuf[6] + realbuf[7]/128.0;

      if (oldpage != page) { lcd.clear(); oldpage = page; }
      // serial message format:
      // temperature humidity level battery
      Serial.print((float) r_temperature, 1); // 1 decimal accuracy
      Serial.print(" ");
      Serial.print((int) (realbuf[3]));
      Serial.print(" ");
      Serial.print((float) r_level, 2);
      Serial.print(" ");
      Serial.print((float) r_battery, 2);
      Serial.print(" ");
      Serial.print((int) 0);
		
	    Serial.println("");

      if (page == 0) {
        lcd.setCursor(0,0);
        lcd.print("Temperature:");
        lcd.print((int)(realbuf[1]));
        lcd.write((uint8_t)0);
        lcd.print("C");
  
        lcd.setCursor(0,1);
        lcd.print("Humidity:");
        lcd.print((int)(realbuf[3]));
        lcd.print("%");     
    } else if (page == 1) {
        lcd.setCursor(0,0);
        lcd.print("Panels: ");
        lcd.print(r_level,1);
        lcd.write((uint8_t)0);  

        //lcd.setCursor(0,1);
        //lcd.print("Battery: ");
        //lcd.print((int)(r_battery));
        //lcd.print("%");
      }
      dataexists = true;
    } else { dataexists = false; }
}

