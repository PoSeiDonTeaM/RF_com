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
  
    String msg = "TEMP:       ";
    msg += temp;
    msg += "oC";
    msg += "HUM:         ";
    msg += hum;
    msg += "%";
    
    // Convert Arduino's fancy string to something that the driver can understand
    char buffer[64];
    msg.toCharArray(buffer, 64);
    //const char * buffer = "I like toast";
    
    driver.send((uint8_t *)buffer, strlen(buffer));
    driver.waitPacketSent();
    delay(1000);
}

