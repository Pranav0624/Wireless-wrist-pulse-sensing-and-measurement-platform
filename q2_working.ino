#include <Wire.h>
#include "MAX30105.h"
#include "WiFi.h"
#include <HTTPClient.h>


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

// Settings for initializing the 3 sensors 
byte led_brightness = 0x1F;  //Options: 0=Off to 255=50mA
byte sample_avg = 32;    //Options: 1, 2, 4, 8, 16, 32
byte LED_mode = 3;           //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
int sample_rate = 1600;       //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulse_width = 215;       //Options: 69, 118, 215, 411
int ADC_range = 8192;        //Options: 2048, 4096, 8192, 16384

const byte baseValue1=0;
const byte baseValue2=0;
const byte baseValue3=0;

void setup() {
  Wire.begin(); // Initialize the ESP32's I2C bus
  Serial.begin(115200);

 // Checking whether the sensors are connected and are being detected properly ..

  // Select channel 2 on the multiplexer IC for sensor 1
  initializeMuxChannel(2);
  // Communicate with device on channel 2
  if(!sensor_1.begin()){
    Serial.println("Sensor connected to channel 2 not detected...");
    while(1);
  }
  else{
    Serial.println("Sensor connected to channel 2 detected!!");
  }

  initializeMuxChannel(5);
  if(!sensor_2.begin()){
    Serial.println("Sensor connected to channel 3 not detected...");
    while(1);
  }
  else{
    Serial.println("Sensor connected to channel 3  detected!!");
  }

  initializeMuxChannel(7);
  // Communicate with device connected to channel 6
  if(!sensor_3.begin()){
    Serial.println("Sensor connected to channel 6 not detected...");
    while(1);
  }
  else
    Serial.println("Sensor connected to channel 6 detected!!");


 // Pre-populating the plotter so that the Y scale is close to IR values
  initializeMuxChannel(2);
  sensor_1.setup(led_brightness, sample_avg, LED_mode, sample_rate, pulse_width, ADC_range); //Configure sensor with these settings
  const byte avgAmount1 = 64;
  long baseValue1 = 0;
  for (byte x = 0 ; x < avgAmount1 ; x++)
  {
    baseValue1 += sensor_1.getIR(); //Read the IR value
  }
  baseValue1 /= avgAmount1;

  for (int x = 0 ; x < 500 ; x++);
  //   Serial.println(baseValue1);

  initializeMuxChannel(5);
  // Communicate with a device connected to channel 4
  sensor_2.setup(led_brightness, sample_avg, LED_mode, sample_rate, pulse_width, ADC_range); //Configure sensor with these settings
  const byte avgAmount2 = 64;
  long baseValue2 = 0;
  for (byte x = 0 ; x < avgAmount2 ; x++)
  {
    baseValue2 += sensor_2.getIR(); //Read the IR value
  }
  baseValue2 /= avgAmount2;

  for (int x = 0 ; x < 500 ; x++);
  //   Serial.println(baseValue2);


  initializeMuxChannel(7);
  sensor_3.setup(led_brightness, sample_avg, LED_mode, sample_rate, pulse_width, ADC_range); //Configure sensor with these settings
  const byte avgAmount3 = 64;
  long baseValue3 = 0;
  for (byte x = 0 ; x < avgAmount3 ; x++)
  {
    baseValue3 += sensor_3.getIR(); //Read the IR value
  }
  baseValue3 /= avgAmount3;

  for (int x = 0 ; x < 500 ; x++);
  //   Serial.println(baseValue3);

}



void loop() {

  initializeMuxChannel(2);
  vattaVal = sensor_1.getIR();
  // Adjusting for DC noise
  vattaVal_adjusted = ((vattaVal > 45000) ? (vattaVal-45000):(0));
 // Serial.print(vattaVal_adjusted);
 Serial.print(vattaVal - baseValue1);
  Serial.print(",");

  initializeMuxChannel(5);
  pittaVal = sensor_2.getIR();
  // Adjusting for DC noise
  pittaVal_adjusted = ((pittaVal > 50000) ? (pittaVal-50000):(0));
  // Serial.print(pittaVal_adjusted);
  Serial.print(pittaVal - baseValue2);
  Serial.print(",");

  initializeMuxChannel(7);
  kaphaVal = sensor_3.getIR();
  // Adjusting for DC noise
  kaphaVal_adjusted = ((kaphaVal > 50000) ? (kaphaVal-50000):(0));
  // Serial.println(kaphaVal_adjusted);
  Serial.println(kaphaVal - baseValue3);

}

void initializeMuxChannel(byte channel) {
  // Send the channel selection command to the multiplexer
  Wire.beginTransmission(Multiplexer_address);
  Wire.write(1 << channel);
  Wire.endTransmission();
  delay(1); // Delay to allow the switch to settle
}