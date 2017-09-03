#include <DHT.h>;
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

int chk;
int hum;  //Stores humidity value
int temp; //Stores temperature value

void setup()
{
    Serial.begin(9600);   // Debugging only
    dht.begin();
    if (!driver.init())
         Serial.println("init failed");
}
  
void loop()
{
    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    
    const int8_t buffer[3] = {
        -127, // an identifying value that will not show up in the data
              // so that we know when the transmission starts
        (int8_t) temp, // this assumes temperature is between -126 and 127
        (int8_t) hum // humidity is always between 0 and 100
    };
     
    driver.send((uint8_t *)buffer, 3);
    driver.waitPacketSent();
    delay(1000);
}

