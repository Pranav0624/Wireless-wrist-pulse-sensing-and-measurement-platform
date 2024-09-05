#include <Wire.h>
#include "MAX30105.h"
#include "WiFi.h"
#include <HTTPClient.h>
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Fsa"
#define WIFI_PASSWORD "akkk7644"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCRkaIiVscuPCdonw34YXLb0fk6bGMXSi0"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://pulse-sensor-1ff24-default-rtdb.asia-southeast1.firebasedatabase.app/"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
int user_count = 1;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
int count = 1;

// including libraries and variables for getting time from ntp server
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int  daylightOffset_sec = 0;

#define Multiplexer_address 0x70  
// Initializing the 3 sensors using MAX30105 library
MAX30105 sensor_1;
MAX30105 sensor_2;
MAX30105 sensor_3;

// Initializing variables for storing IR values
long vattaVal = 0;
long pittaVal = 0;
long kaphaVal = 0;
long vattaVal_adjusted, pittaVal_adjusted, kaphaVal_adjusted;

//ID of the deployed Google script
String GoogleScriptID = "AKfycbwJi2o4chogCeULvRMIiLgvwZeXxKCZRe3MyLZuxJvnGvig8bs-Ogz7-HS1r8M_5LgS";

// Wifi credentials
// const char* WiFi_ssid = "Fsa";
// const char* WiFi_password = "akkk7644";

// Settings for initializing the 3 sensors 
byte led_brightness = 0x1F;  //Options: 0=Off to 255=50mA
byte sample_avg = 32;    //Options: 1, 2, 4, 8, 16, 32
byte LED_mode = 3;           //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
int sample_rate = 1600;       //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulse_width = 215;       //Options: 69, 118, 215, 411
int ADC_range = 8192;        //Options: 2048, 4096, 8192, 16384

xQueueHandle xQueue;

typedef struct{
  int vattaVal;
  int pittaVal;
  int kaphaVal;
}Data;


void gettingCore(void* parameters){
  Data data;
  BaseType_t status;
  HTTPClient http;
  while(1){
    status = xQueueReceive(xQueue, &data, 0);
    if(status == pdPASS){
       //Sending time data and IR values to Google Sheet
  
      // struct tm timeinfo;
      // char timeStringBuff[50]; //50 chars should be enough
      // strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
      // String asString(timeStringBuff);
      // asString.replace(" ", "-");
     String GoogleSheetURL = "https://script.google.com/macros/s/"+GoogleScriptID+ "/exec?"+ "Time=" + String(1) + "&Vatta=" + String (data.vattaVal) + "&Pitta=" + String (data.pittaVal) + "&Kapha=" + String (data.kaphaVal);
   http.begin(GoogleSheetURL.c_str());
      http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
      int httpCode = http.GET(); 
    // count++;
  }
      
      
    
  }
  http.end();

}
void setup() {
  Wire.begin(); // Initialize the ESP32's I2C bus
  Serial.begin(115200);
 xQueue = xQueueCreate(15, sizeof(Data));

  //Connecting to the WiFi network
  Serial.println();
  Serial.print("Connecting to wifi network: ");
  Serial.print(WIFI_SSID);
  Serial.flush();
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay (500);
    Serial.println(".");
  }
  // Getting time data
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

 // Checking whether the sensors are connected and are being detected properly ..

  // Select channel 2 on the multiplexer IC for sensor 1
  initializeMuxChannel(2);
  // Communicate with device on channel 2
  if(!sensor_1.begin()){
    Serial.println("Sensor connected to channel 2 not detected...");
    while(1);
  }else{
    Serial.println("Sensor connected to channel 2 detected!!");
  }

  initializeMuxChannel(5);
  if(!sensor_2.begin()){
    Serial.println("Sensor connected to channel 5 not detected...");
    while(1);
  }
  else{
    Serial.println("Sensor connected to channel 5  detected!!");
  }

  initializeMuxChannel(7);
  // Communicate with device connected to channel 6
  if(!sensor_3.begin()){
    Serial.println("Sensor connected to channel 7 not detected...");
    while(1);
  }
  else
    Serial.println("Sensor connected to channel 7 detected!!");


 // Pre-populating the plotter so that the Y scale is close to IR values
  initializeMuxChannel(2);
  sensor_1.setup(led_brightness, sample_avg, LED_mode, sample_rate, pulse_width, ADC_range); //Configure sensor with these settings


  initializeMuxChannel(5);
  // Communicate with a device connected to channel 4
  sensor_2.setup(led_brightness, sample_avg, LED_mode, sample_rate, pulse_width, ADC_range); //Configure sensor with these settings


  initializeMuxChannel(7);
  sensor_3.setup(led_brightness, sample_avg, LED_mode, sample_rate, pulse_width, ADC_range); //Configure sensor with these settings

  // config.api_key = API_KEY;

  // /* Assign the RTDB URL (required) */
  // config.database_url = DATABASE_URL;

  // /* Sign up */
  // if (Firebase.signUp(&config, &auth, "", "")) {
  //   Serial.println("ok");
  //   signupOK = true;
  // } else {
  //   Serial.printf("%s\n", config.signer.signupError.message.c_str());
  // }

  // /* Assign the callback function for the long running token generation task */
  // config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  // Firebase.begin(&config, &auth);
  // Firebase.reconnectWiFi(true);

  xTaskCreatePinnedToCore(
    gettingCore,
    "gettingCore",
    10000,
    NULL,
    1,
    NULL,
    1
  );
    
  
}



void loop() {
  if(!(WiFi.status() == WL_CONNECTED)){
    Serial.println("WiFi connection lost ...");
    return;
  }
  // if (WiFi.status() == WL_CONNECTED) {
  //   static bool flag = false;
  //   struct tm timeinfo;
  //   if (!getLocalTime(&timeinfo)) {
  //     Serial.println("Failed to obtain time");
  //     return;
  //   }
  // }
   // Obtaining date and time data and organizing it into a string 
  // Serial.println("\n");
  // struct tm timeinfo;
  // char timeStringBuff[50]; //50 chars should be enough
  // strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  // String asString(timeStringBuff);
  // asString.replace(" ", "-");

  initializeMuxChannel(2);
  vattaVal = sensor_1.getIR();
  // Adjusting for DC noise
  // vattaVal_adjusted = ((vattaVal > 50000 ) ? (vattaVal-50000):(0));
  Serial.print(vattaVal);
  Serial.print(",");

  initializeMuxChannel(5);
  pittaVal = sensor_2.getIR();
  // Adjusting for DC noise
  // pittaVal_adjusted = ((pittaVal > 50000) ? (pittaVal-50000):(0));
  Serial.print(pittaVal);
  Serial.print(",");

  initializeMuxChannel(7);
  kaphaVal = sensor_3.getIR();
  // Adjusting for DC noise
  // kaphaVal_adjusted = ((kaphaVal > 50000 ) ? (kaphaVal-50000):(0));
  Serial.println(kaphaVal);
  //8Serial.println("");

  Data data;
  BaseType_t status; 
  data.vattaVal = vattaVal;
  data.pittaVal = pittaVal;
  data.kaphaVal = kaphaVal;

  status = xQueueSendToFront(xQueue, &data, 0);
    
}

void initializeMuxChannel(byte channel) {
  // Send the channel selection command to the multiplexer
  Wire.beginTransmission(Multiplexer_address);
  Wire.write(1 << channel);
  Wire.endTransmission();
  delay(1); // Delay to allow the switch to settle
}
// https://pulse-sensor-1ff24-default-rtdb.asia-southeast1.firebasedatabase.app/
