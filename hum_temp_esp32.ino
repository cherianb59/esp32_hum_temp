// Example testing sketch for various DHT humidity/temperature sensors written by ladyada
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor


#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <WiFiManager.h>

//WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
WiFiManager wm;

const char* ssid = "";
const char* password = "";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://api.dsssmble.cyou/kk";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

unsigned long POSTtimerDelay = 60000;
unsigned long lastPOSTTime = -POSTtimerDelay;

#define DHTPIN 4     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11      // DHT 22  (AM2302), AM2321

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

float h1 = 0 ; 
float t1 = 0 ; 

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

struct Button {
	const uint8_t PIN;
	uint32_t numberKeyPresses;
	bool pressed;
};

Button button1 = {18, 0, false};

//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

void IRAM_ATTR isr() {
  button_time = millis();
  if (button_time - last_button_time > 250)
  {
          button1.numberKeyPresses++;
          button1.pressed = true;
        last_button_time = button_time;
  }
}


void setup() {
 
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Starting Up");

  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));

  dht.begin();
  lcd.setCursor(0, 1);
  lcd.print("DHT ");

  WiFi.begin(ssid, password);
  Serial.println("WiFi connecting");
  lcd.setCursor(0, 1);
  lcd.print("WiFi ");

  int wifi_count = 0  ; 
  const int wifi_count_max = 10  ; 
  while( (WiFi.status() != WL_CONNECTED) &&  (wifi_count<wifi_count_max) ) {
    delay(500);
    wifi_count = wifi_count + 1 ; 
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.setCursor(1, 0);
  lcd.print("WiFi Connected");
  lcd.clear();

	pinMode(button1.PIN, INPUT_PULLUP);
	attachInterrupt(button1.PIN, isr, FALLING);
}

void loop() {

	
  if (button1.pressed) {
		Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
		button1.pressed = false;
	}

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    // set cursor to first column, first row
    lcd.setCursor(0, 0);
    // print message
    lcd.print("Error");
    lcd.setCursor(0, 1);
    // print message
    lcd.print("Error");

    delay(2000);
    return;
  }

  // only print if there is a change
  if (h1 != h || t1 != t){
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("C "));
    Serial.println(" ");

    //lcd.clear();
    // set cursor to first column, first row
    lcd.setCursor(0, 0);
    // print message
    lcd.print("Temp:");
    lcd.setCursor(6, 0);
    lcd.print(t);
    lcd.setCursor(0, 1);
    // print message
    lcd.print("Hum: ");
    lcd.setCursor(6, 1);
    lcd.print(h);

    // update prev reading
    h1 = h ; 
    t1 = t ; 

  }

  //Send an HTTP POST request every 1 minute
  if ((millis() - lastPOSTTime) > POSTtimerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
      
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "api_key=a&humidity=";
      httpRequestData += String(h)      ;
      httpRequestData += "&temp=" ; 
      httpRequestData += String(t) ; 

      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      
      Serial.println(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      String response = http.getString(); //Get the response to the request
      Serial.println(response);   //Print return code        

      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastPOSTTime = millis();

  }
  delay(2000);


}
