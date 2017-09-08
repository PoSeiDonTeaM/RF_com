#include <DHT.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile

RH_ASK driver;

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino

// DHT sensor has a 0.5 Hz sampling rate,
// but we are transmitting data faster to
// account for possible packet loss

// DHT22 typical ranges:
// -40 ~ 125 C temperature
//   0 ~ 100 % humidity

int chk;
int hum;  //Stores humidity value
float temp; //Stores temperature value
float bat; // Stores battery value
float lvl; // Stores level value

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

    // Convert to integer and fractional parts
    double tempInt;
    float tempFrac = modf(temp, &tempInt);
    double batInt;
    float batFrac = modf(temp, &batInt);
    double lvlInt;
    float lvlFrac = modf(temp,&lvlInt);

    Serial.print(tempInt); Serial.print("."); Serial.println(tempFrac*256);
    
    const int8_t buffer[8] = {
        -127, // an identifying value that will not show up in the data
              // so that we know when the transmission starts
        (int8_t) tempInt, // this assumes temperature is between -126 and 127
        (int8_t) (tempFrac*127), // convert the fractional part to an integer -
                                 // the receiver will have to decode this
        (int8_t) hum, // humidity is always between 0 and 100
        (int8_t) batInt, // battery is between 0 and 127
        (int8_t) (batFrac*127),
        (int8_t) lvlInt, // level is between 0 and 90
        (int8_t) (lvlFrac*127),
    };
     
    driver.send((uint8_t *)buffer, 4);
    driver.waitPacketSent();
    delay(500);
}

