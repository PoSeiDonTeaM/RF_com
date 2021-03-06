#include <RH_ASK.h>
#include <LiquidCrystal.h>
#include <SPI.h> // Not actualy used but needed to compile

RH_ASK driver;
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

/**
 * Constants
 */
const float transmissions_per_second = 10;
const float signal_refresh_per_second = 5;
const float screen_refresh_per_second = 3;
#define MOVING_AVERAGE_COUNT 10
const bool demo = false;

/**
 * An array to store measured signal strength for small time intervals
 * These intervals are then averaged out
 */
float signalReceived[MOVING_AVERAGE_COUNT] = { 0 };
int signalReceivedPoint = 0; // The last value added
float signalAverage = 0;

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

    // Set LED pin for debugging
    pinMode(13, OUTPUT);
    pinMode(12, OUTPUT);
    digitalWrite(13, HIGH);

    if (demo) {
      lcd.setCursor(0,0);
      lcd.print("DEMO MODE     ");
      lcd.setCursor(0,1);
      lcd.print("Starting in ");
      for (int i = 5; i >= 0; i--) {
        lcd.setCursor(12,1);
        lcd.print(i);
        delay(500);
      }
    }
    
    if (!driver.init())
         Serial.println("init failed");  
}

int page = 0;

long lastTime = 0;
long lastScreenTime = 0;
unsigned int lastCount = 0;
float signalStrength;

float r_temperature, r_magx, r_magy, r_magz, r_pressure, r_battery, r_level;
float magfield;

void loop()
{
    static int oldpage = page;
    static bool dataExists = false;

    // COMMENT THIS LINE TO STAY ON THE FIRST PAGE
    // FOREVER
    page = ( (millis()/1000 / 5) % 2 ) ? 0 : 1;

    if (millis() - lastTime >= (1000.0)/signal_refresh_per_second) {
      lastTime = millis();
      signalStrength = (float) lastCount * signal_refresh_per_second / transmissions_per_second;
      lastCount = 0;

      // Calculate the average of the last MOVING_AVERAGE_COUNT signal strengths
      signalAverage += (signalStrength - signalReceived[signalReceivedPoint])/( (float) MOVING_AVERAGE_COUNT);
      // Add the new signal strength to our list
      signalReceived[signalReceivedPoint] = signalStrength;
      signalReceivedPoint = (signalReceivedPoint + 1) % MOVING_AVERAGE_COUNT;
    }
  
    uint8_t buf[64];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen) || demo) { // Non-blocking
      lastCount++;
      dataExists = true;
      
      // Message with a good checksum received, dump it.
      digitalWrite(13, HIGH);
      digitalWrite(12, HIGH);
      
      int8_t * realbuf = (int8_t*) buf;
      r_temperature = (realbuf[1] + realbuf[2]/128.0);
      r_magx = realbuf[3] + realbuf[4]/128.0;
      r_magy = realbuf[5] + realbuf[6]/128.0;
      r_magz = realbuf[7] + realbuf[8]/128.0;
      r_pressure = realbuf[9] + realbuf[10]/128.0;
      //r_battery = realbuf[11] + realbuf[12]/128.0;
      r_battery = r_temperature;

      if(demo) {
        r_magy = analogRead(A1);
        r_magz = analogRead(A2);
        r_battery = min(r_battery,5.5) + random(0,100)/1000.0;
      }

      magfield = sqrt( pow(r_magx, 2) + pow(r_magy, 2) + pow(r_magz, 2));

      if (oldpage != page) { lcd.clear(); oldpage = page; }
      // serial message format:
      // temperature humidity level battery
      Serial.print((float) r_temperature, 1); // 1 decimal accuracy
      Serial.print(" ");
      Serial.print((float) r_magx, 2);
      Serial.print(" ");
      Serial.print((float) r_magy, 2);
      Serial.print(" ");
      Serial.print((float) r_magz, 2);
      Serial.print(" ");
      Serial.print((float) r_pressure, 2);
      Serial.print(" ");
      Serial.print((float) r_battery, 2);
      Serial.print(" ");
      Serial.print((float) signalAverage * 100, 1);
		
	    Serial.println("");

      digitalWrite(13, LOW);
      digitalWrite(12, LOW);

      if (demo) delay(random(1,150));
    }

    
    if (dataExists && millis() - lastScreenTime >= (1000.0)/screen_refresh_per_second) {
      lastScreenTime = millis();
      if (page == 0) {
        lcd.setCursor(0,0);
        lcd.print(demo ? "d" : "[");
        int bars = signalAverage * 13;
        for (int i = 0; i <= 13; i++) {
          if (i < bars) {
            lcd.print("|");
          } else {
            lcd.print(" ");
          }
        }
        lcd.print(demo ? "d" : "]");
  
        lcd.setCursor(0,1);
        lcd.print("Magn (uT): ");
        lcd.print((float)(magfield));
    } else if (page == 1) {
        lcd.setCursor(0,0);
        lcd.print("Voltage: ");
        lcd.print(r_battery,1);
        lcd.write(" V ");
  
        lcd.setCursor(0,1);
        lcd.print("Magn (uT): ");
        lcd.print((float)(magfield));
      }
    }

    delay(5);
}

