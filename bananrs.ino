#include <RTClib.h>
#include <DHT.h>
#include <Wire.h>
RTC_DS1307 RTC;

//setting pin constants

#define powerPin D0
#define moistPin A0
#define tempPin D3
#define pumpPin D4
#define lightPin D6
#define lowlevelPin D8

//define polling interval in seconds (NOT MILLISECONDS!), choose a reasonable value so that you are
//checking sensors at a useful rate, but not overpolling and drawing too much power for no good reason

#define runInterval 120

//setting up DHT22

#define DHTTYPE DHT22
DHT dht(tempPin, DHTTYPE);
int lastRun;

void setup() {

  //setting pin modes

  pinMode(powerPin, OUTPUT);
  pinMode(tempPin, INPUT_PULLUP);
  pinMode(moistPin, INPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(lowlevelPin, INPUT_PULLDOWN_16);

  digitalWrite(pumpPin, LOW);

  //opening up serial and initializing serial functions
  Serial.begin(9600);
  Wire.begin();
  dht.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

}

void loop() {

  //set delay between measurements in ms
  //do not set below 50 so that EPS8266 has an opportunity to run background processes, without enough time, the board will crash
  delay(100);

  DateTime now = RTC.now();

  if (now.unixtime() > lastRun + runInterval)
  {

    //read temperature and humidity
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(true);

    //read soil moisture
    int moisture = analogRead(moistPin);

    //check water level
    boolean waterlevel = digitalRead(lowlevelPin);

    //decide whether to run pump or not, if soil is dry and water is low, run pump for 5 seconds
    if ((moisture > 700) && (waterlevel == HIGH))
    {
      digitalWrite(pumpPin, HIGH);
      delay(3000);
      digitalWrite(pumpPin, LOW);
    }

    //send data over serial
    DateTime now = RTC.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println(" ");
    Serial.print("Current Conditions: ");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%\t ");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("*F ");
    Serial.print("Soil: ");
    Serial.print(moisture);
    Serial.print(" Water Level: ");
      if (waterlevel == HIGH) {
        Serial.print("OK");
      }
      else {
        Serial.print("Low");
      }
    Serial.println();
    
    lastRun = now.unixtime();
  }

}

