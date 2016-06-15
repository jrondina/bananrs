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

#define runInterval 5

//setting up DHT22

#define DHTTYPE DHT22
DHT dht(tempPin, DHTTYPE);

#define disconnectThreshold 1010
#define dryThreshold 950

int lastRun;
int dryness;
float humidity;
float temperature;
boolean waterLevel;

void setup() {
  //let modules power up, otherwise you might get NaN on initial results
  delay(500);
  
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
    humidity = dht.readHumidity();
    temperature = dht.readTemperature(true);

    //read soil dryness
    dryness = analogRead(moistPin);

    //check water level
    waterLevel = digitalRead(lowlevelPin);

    //run pump or shut pump off depending on conditions in pump_state
    digitalWrite(pumpPin, pump_state());

    //give light value depending on time of day
    analogWrite(lightPin, light_level());

    //send data over serial
    serial_send(); 

    lastRun = now.unixtime();
  }

}

boolean pump_state() {
  if ((dryness > 950) && (waterLevel == HIGH))
    {
      return HIGH;
    }
  else 
    {
      return LOW;
    }
}

int light_level(){
  //parabolic equation to specify maximum light output (8-bit) at noon, more or less model intensity of sun
  //y = a(x-h)^2 + k
  //y = light intensity
  //x = time (in hours)
  //h = x value of vertex (time when light is most intense, in this case 12 or noon)
  //k = y value of vertex (max value, in this case 255 due to 8-bits)
  //but don't forget, you must dimension variables as floats, otherwise they will default to doubles
  //afterward, cast to int to truncate the decimal
  DateTime now = RTC.now();
  float a = -4.0;
  float x = now.hour() + (now.minute() / 60.0);
  float h = 12.0;
  float k = 255.0;
  float y = (a * (pow((x - h), 2)) + k);
  //cast y from float to int
  int lightLevel = (int) y;
  //int y = (-4(pow(((now.hour() + mins) - 12), 2) + 255);
  //make sure negative values aren't passed to analogWrite(), otherwise it will rollover
  if (lightLevel < 0){
    lightLevel = 0;
  }
  return lightLevel;
}

void serial_send() {
     //send data over serial
    DateTime now = RTC.now();
    
    Serial.print(now.year(), DEC);
    Serial.print("/");
    Serial.print(now.month(), DEC);
    Serial.print("/");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.print(now.second(), DEC);
    Serial.println(" ");
    Serial.print("Current Conditions: ");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("*F ");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%\t");
    Serial.print("Soil: ");
    Serial.print(dryness);
    Serial.print(" Water Level: ");
    if (waterLevel == HIGH) {
      Serial.print("OK");
    }
    else {
      Serial.print("Low");
    }
    Serial.println(" ");
    Serial.print("Light Output: ");
    Serial.print(light_level());
    Serial.print(" ");
    Serial.print("Pump Status: ");
    if (pump_state() == HIGH) {
      Serial.print("On ");
    }
    else {
      Serial.print("Off ");
    }
    
    Serial.println(" ");
}

