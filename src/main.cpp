/*
 * This ESP8266 NodeMCU code was developed by newbiely.com
 *
 * This ESP8266 NodeMCU code is made available for public use without any restriction
 *
 * For comprehensive instructions and wiring diagrams, please visit:
 * https://newbiely.com/tutorials/esp8266/esp8266-led-matrix
 */

#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> 
#include <WiFiClient.h> 

#include <NTPClient.h> 
#include <WiFiUdp.h> 
#include <WiFiManager.h> 

#include <Arduino_JSON.h>


#include <Adafruit_Sensor.h>
#include "DHT.h"


#include "OneButton.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4 // 4 blocks
#define CS_PIN D6

#define DHTTYPE DHT11 // DHT 11
//float temperature, humidity, pressure, altitude;


const int DHTPin = D4;
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

#define DEBUG 1

#if DEBUG
  #define PRINT(x) Serial.print(x);
  #define PRINTLN(x) Serial.println(x);
#else
  #define PRINT(x) 
  #define PRINTLN(x)
#endif

// create an instance of the MD_Parola class
MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);




WiFiUDP ntpUDP;


WiFiManager wifiManager;

unsigned long lastSecond=0;
 String Hours;
  String Mins;
  String Seconds;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
bool pointState=false;

uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C

unsigned long currentStateStartTime = 0;
const unsigned long stateDuration = 5000; // Тривалість кожного стану у мілісекундах

enum State {
  STATE_DATE,
  STATE_TEMP,
  STATE_CLOCK
};

const unsigned long stateDurations[3] = {
  10000, // STATE_DATE: 10 секунди
  10000, // STATE_TEMP: 10 секунд
  15000 // STATE_CLOCK: 15 секунд
};

State currentState = STATE_CLOCK;

bool printedState1 = false;
bool printedState2 = false;
bool printedState3 = false;

char ipChar[16];
String Time="";
String Date="";
String inTemp,outTemp;
String inHum,outHum;
int temp_corr=-2;

String inTempHum="";
bool showClock = false;

int buttonPin = D1;
int button2Pin = D2;
int ledPin = D0;
bool ledState = false;

OneButton button(buttonPin, true);
OneButton button2(button2Pin, true);

static const char ntpServerName[] = "time.nist.gov";
const long utcOffsetInSeconds = 3600;
NTPClient timeClient(ntpUDP, ntpServerName, utcOffsetInSeconds);
unsigned long lastSync=0; //Last NTP sync
bool ntpSyncStatus=false;

int repTime = 0; //First logo repeat times


String jsonBuffer;

String city="London";
#String openWeatherMapApiKey = "7e04cc3aaf4976b9ffd56f77d83b1fa4";
String temperature_unit = "metric";


void toggleLED() {
  ledState = !ledState; // Змінюємо стан на протилежний
  digitalWrite(ledPin, ledState ? HIGH : LOW); // Вмикаємо або вимикаємо світлодіод
  Serial.print("LED is now ");
  Serial.println(ledState ? "ON" : "OFF");
}

 void doubleClick()  {
  PRINTLN("DoubleClick() detected.");
  showClock =!showClock;
}

void singleClick()  {
  PRINTLN("SingleClick() detected.");
  toggleLED();
 
}
void getIndoor()
{
  inHum=  "h="+String(dht.readHumidity(),0)+"% ";
  inTemp= " t="+String(dht.readTemperature()+temp_corr,0)+"$ ";
  inTempHum=inTemp+inHum;
}


int getTime(){

  unsigned long currentClockMillis = millis();
  time_t epochTime;
  int monthDay =0;
  int currentMonth =0;
  int currentYear =0;

 
  int hours,mins,seconds;
 

  if (currentClockMillis - lastSecond >= 500) 
  {
    lastSecond = currentClockMillis;
    epochTime = timeClient.getEpochTime();
    
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    
    monthDay = ptm->tm_mday;
    currentMonth = ptm->tm_mon+1;
    currentYear = ptm->tm_year+1900;
    
    Date = String(monthDay)+ "/" + String(currentMonth)+ "/"+String(currentYear);

    hours = timeClient.getHours();
    mins = timeClient.getMinutes();
    seconds = timeClient.getSeconds();
 
    Hours = String(hours);
    Mins  = String(mins);
    Seconds  = String(seconds); 
     if (hours<10) Hours = " "+Hours;
    if (mins<10) Mins = "0"+Mins;
  Date = String(daysOfTheWeek[timeClient.getDay()]) +" "+ Date+" ";
  pointState = !pointState;
  //PRINTLN(Date)
  return 1;
  
  
  } return 0;
}



String httpGETRequest(const char* serverName) 
{
  WiFiClient client;
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();   
  return payload;
}


void getCityCode() {
String URL = "http://ip-api.com/json/?fields=city,lat,lon";

jsonBuffer = httpGETRequest(URL.c_str());

JSONVar myObject = JSON.parse(jsonBuffer);
  
  if (JSON.typeof(myObject) == "undefined") 
  {
    Serial.println("Parsing input failed!");
    return;
  }
  city = String(myObject["city"]);
  PRINTLN(city)
}
 
 void getCityTime()
 {
  String URL = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=" + temperature_unit  + "&appid=" + openWeatherMapApiKey;
  jsonBuffer = httpGETRequest(URL.c_str());
  JSONVar myObject = JSON.parse(jsonBuffer);
   if (JSON.typeof(myObject) == "undefined") 
  {
    Serial.println("Parsing input failed!");
    return;
  }
  PRINTLN(myObject["timezone"])
  timeClient.setTimeOffset(myObject["timezone"]);
 }


void setup() {

  String startLogo="";
  Serial.begin(115200);
  dht.begin(); 

  ledMatrix.begin();         // initialize the object
  ledMatrix.setIntensity(0); // set the brightness of the LED matrix display (from 0 to 15)
  ledMatrix.displayClear();  // clear led matrix display
  ledMatrix.addChar('$', degC);

   if ( digitalRead(button2Pin) == LOW) { 
    wifiManager.resetSettings();
   }

  wifiManager.autoConnect("MiniWeather");
  //delay(2000);

  Serial.print("Connecting to WiFi");
 //WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
  Serial.println(WiFi.localIP());
  startLogo=WiFi.localIP().toString();
  Serial.println(startLogo);
  IPAddress localIP = WiFi.localIP();
  String ipString = localIP.toString();
   Serial.println("IP Address: " + ipString);

  // IPv4: максимум 15 символів + 1 для '\0'
   ipString.toCharArray(ipChar,ipString.length()+1);

  Serial.println(ipChar);
  // Display IP address on LED matrix
  getCityCode();
  getCityTime();
  ledMatrix.displayText(ipChar, PA_CENTER, 100, 0, PA_SCROLL_LEFT);

  

  if (timeClient.update()) {
      Serial.println("Synchronized with NTP.");
      ntpSyncStatus=true;
    } else {
      Serial.println("Error synchronizing with NTP.");
    }

button.attachDoubleClick(doubleClick);
button2.attachClick(singleClick);

pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
   
}

void loop() {

   if(repTime<1) {
      if (ledMatrix.displayAnimate()) {
        ledMatrix.displayReset();
        Serial.println(repTime);
        repTime++;
      }
    return;
    }

  button.tick();
  button2.tick();

  if ((millis() - lastSync > 300000) || (!ntpSyncStatus)) 
  {
    lastSync = millis();
    timeClient.begin();
    if (timeClient.update()) {
      Serial.println("Synchronized with NTP (in loop).");
      ntpSyncStatus=true;
      
    } else {
      Serial.println("Error synchronizing with NTP (in loop).");
      ntpSyncStatus=false;
    }
  }
  
  unsigned long currentMillis = millis();

    if (currentMillis - currentStateStartTime >= stateDurations[currentState]) {
      currentStateStartTime = currentMillis; 
      printedState1 = false;
      printedState2 = false;
      printedState3 = false;
    
   

     switch (currentState) 
     {
      case STATE_DATE:
        currentState = STATE_TEMP;
        break;
      case STATE_TEMP:
        currentState = STATE_CLOCK;
        break;
      case STATE_CLOCK:
        currentState = STATE_DATE;
        break;
     }
    }

     if(showClock) 
    {
      currentState = STATE_CLOCK;
    }

    if (ledMatrix.displayAnimate()) {
    ledMatrix.displayReset();
    }
    
    switch (currentState) {
    case STATE_DATE:
      if (!printedState1) {
        PRINTLN("Виконуються дії для Стану 1");
        //ledMatrix.print("21/01/24");
        
            //PRINTLN("Data")
            ledMatrix.displayClear();
            ledMatrix.displayText(Date.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT);
            
        printedState1 = true;
      }
      break;
    case STATE_TEMP:
     
      if (!printedState2) {
        PRINTLN("Виконуються дії для Стану 2");
        ledMatrix.displayClear();
        getIndoor();
        ledMatrix.displayText(inTempHum.c_str(), PA_CENTER, 100, 0, PA_SCROLL_LEFT);
        
        printedState2 = true;
      }
      break;
    case STATE_CLOCK:
      
      if (!printedState3) {
        //PRINTLN("Виконуються дії для Стану 3");
       // displayIndoor();
         if(getTime()) {
          if(pointState)
            Time = Hours+":"+Mins;
           else
            Time = Hours+" "+Mins;
         //Serial.print(Time);
         //ledMatrix.print(Time.c_str());
         ledMatrix.displayText(Time.c_str(), PA_CENTER, 100, 0, PA_PRINT);
  
          }
        printedState3 = false;
      }
      break;
  }
  

}
