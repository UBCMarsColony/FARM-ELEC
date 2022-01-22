// ----------------------------------------------------------------------------
// Sample Sensor Code for use with Data Streamer Excel add-in
// more info available from Microsoft Education Workshop at 
// http://aka.ms/hackingSTEM 
// 
// This project uses an Arduino UNO microcontroller board. More information can
// be found by visiting the Arduino website: 
// https://www.arduino.cc/en/main/arduinoBoardUno 
//  
// This code reads in a generic analog sensor on Arduino pin 0 and prints 
// it to serial.
// 
// Comments, contributions, suggestions, bug reports, and feature requests 
// are welcome! For source code and bug reports see: 
// http://github.com/[TODO github path to Hacking STEM] 
// 
// Copyright 2019, Jen Fox Microsoft EDU Workshop - HackingSTEM 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE. 
// ----------------------------------------------------------------------------
// Program variables ----------------------------------------------------------
#include <Wire.h>

// CO2 K30
//#include "kSeries.h"
#include <SoftwareSerial.h>

// DHT22
#include <DHT.h>

// BME280
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Constants
#define Rx1 8                           // Rx pin for CO2 (control)
#define Tx1 9                           // Tx pin for CO2 (control)
#define Rx2 12                          // Rx pin for CO2 (lpgc)
#define Tx2 13                          // Tx pin for CO2 (lpgc)
#define CO2_LOWER_THRESHOLD 1100        // Threshold below which to enable pump
#define CO2_UPPER_THRESHOLD 1300        // Threshold above which to enable pump
#define PUMP 1                          // Pump pin connection
#define DHTPIN 7                        // What pin we're connected to
#define DHTTYPE DHT22                   // DHT 22  (AM2302)
#define SEALEVELPRESSURE_HPA (1013.25)  // Constant sea level pressure

// Initializations
SoftwareSerial portOne(Rx1, Tx1);    // Initialize CO2 sensor for kSeries k30 (control)
SoftwareSerial portTwo(Rx2, Tx2);    // Initialize CO2 sensor for kSeries k30 (lpgc)

DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor for normal 16mhz Arduino
Adafruit_BME280 bme;      // Initialize BME280 for I2C (Adafruit code)

// Variables
float co2_control; //Stores CO2 value (control)
float co2_lpgc;    //Stores CO2 value (lpgc)
float hum;  //Stores humidity value
float temp; //Stores temperature value
float pres; //Stores pressure value
float hum_dht;  //Stores humidity value from DHT
float temp_dht; //Stores temperature value from DHT

unsigned long delayTime = 5000;  //Current delay time between readings
int bmeAddr = 0x76;               //BME280 sensor address
int bmeAddr2 = 0x77;              //BME280 sensor address for second BME sensor
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25}; //Command packet to read Co2 (see app note) 
byte response[] = {0,0,0,0,0,0,0}; //create an array to store the response 

// Serial data variables ------------------------------------------------------
//Incoming Serial Data Array
const byte kNumberOfChannelsFromExcel = 6; 
// Comma delimiter to separate consecutive data if using more than 1 sensor
const char kDelimiter = '|';    
// Interval between serial writes
const int kSerialInterval = 50;   
// Timestamp to track serial interval
unsigned long serialPreviousTime; 
char* arr[kNumberOfChannelsFromExcel];

// SETUP ----------------------------------------------------------------------
void setup() {
  // Initialize Serial Communication
  Serial.begin(9600);  
  // Start each software serial port
  portOne.begin(9600);
  portTwo.begin(9600);
  dht.begin(); // DHT backup
  bme.begin(bmeAddr, &Wire);
}

// START OF MAIN LOOP --------------------------------------------------------- 
void loop()
{
  // Gather and process sensor data
  processSensors();
  // Read Excel variables from serial port (Data Streamer)
  processIncomingSerial();
  // Process and send data to Excel via serial port (Data Streamer)
  processOutgoingSerial();
}

// SENSOR INPUT CODE-----------------------------------------------------------
void processSensors() 
{
  // Read sensor inputs
  hum_dht = dht.readHumidity();                 // DHT backup
  temp_dht = dht.readTemperature();             // DHT backup
//  co2_control = k30_control.getCO2('p');        // Read CO2 (K30 control)
//  co2_lpgc = k30_lpgc.getCO2('p');              // Read CO2 (K30 lpgc)
  hum = bme.readHumidity();                     // Read humidity (BME)
  temp = bme.readTemperature();                 // Read temperature (BME)
  pres = bme.readPressure() / 100.0F;           // Read pressure (BME)

  portOne.listen();
  sendRequestA(readCO2); 
  co2_control = getValueA(response); 

  // Now listen on the second port
  portTwo.listen();
  sendRequestB(readCO2); 
  co2_lpgc = getValueB(response);   

  //CO2PumpLoop(co2_lpgc);                             // Loop to pump CO2
  
  delay(delayTime);                             // Delay between signal reads
}

// Add any specialized methods and processing code below

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void sendDataToSerial()
{
  // Send data out separated by a comma (kDelimiter)
  // Repeat next 2 lines of code for each variable sent:
  Serial.print(co2_control);
  Serial.print(kDelimiter);
  
  Serial.print(co2_lpgc);
  Serial.print(kDelimiter);
  
  Serial.print(hum);
  Serial.print(kDelimiter);

  Serial.print(temp);
  Serial.print(kDelimiter);

  Serial.print(pres);
  Serial.print(kDelimiter);

  Serial.print(hum_dht);
  Serial.print(kDelimiter);

  Serial.print(temp_dht);
  Serial.print(kDelimiter);
  
  Serial.println(); // Add final line ending character only once
}

// CO2 LOOP CODE--------------------------------------------------------------
void CO2PumpLoop(float co2)
{
  //! Actual code needs to continuously read co2 till it reaches CO2_UPPER_THRESHOLD
  //! Need to check if we can do interrupts, or test with different delayTime for most efficient opening/closing of pump
  if(co2 < CO2_LOWER_THRESHOLD)
    digitalWrite(PUMP, 1);
  else if(co2 >= CO2_UPPER_THRESHOLD)
    digitalWrite(PUMP, 0);
}







//-----------------------------------------------------------------------------
// DO NOT EDIT ANYTHING BELOW THIS LINE
//-----------------------------------------------------------------------------
// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void processOutgoingSerial()
{
   // Enter into this only when serial interval has elapsed
  if((millis() - serialPreviousTime) > kSerialInterval) 
  {
    // Reset serial interval timestamp
    serialPreviousTime = millis(); 
    sendDataToSerial(); 
  }
}
// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
void processIncomingSerial()
{
  if(Serial.available()){
    parseData(GetSerialData());
  }
}
// Gathers bytes from serial port to build inputString
char* GetSerialData()
{
  static char inputString[64]; // Create a char array to store incoming data
  memset(inputString, 0, sizeof(inputString)); // Clear the memory from a pervious reading
  while (Serial.available()){
    Serial.readBytesUntil('\n', inputString, 64); //Read every byte in Serial buffer until line end or 64 bytes
  }
  return inputString;
}
// Seperate the data at each delimeter
void parseData(char data[])
{
    char *token = strtok(data, ","); // Find the first delimeter and return the token before it
    int index = 0; // Index to track storage in the array
    while (token != NULL){ // Char* strings terminate w/ a Null character. We'll keep running the command until we hit it
      arr[index] = token; // Assign the token to an array
      token = strtok(NULL, ","); // Conintue to the next delimeter
      index++; // incremenet index to store next value
    }
}

//*************************************************************************
void sendRequestA(byte packet[]) 
{ 
 while(!portOne.available()) //keep sending request until we start to get a response 
 { 
 portOne.write(readCO2,7); 

 delay(50); 

 } 
 int timeout=0; //set a timeoute counter 
 while(portOne.available() < 7 ) //Wait to get a 7 byte response 
 { 
 timeout++; 
 if(timeout > 10) //if it takes to long there was probably an error 
 { 
 while(portOne.available()) //flush whatever we have 
 portOne.read(); 
 break; //exit and try again 
 } 
 delay(50); 
 } 
 for (int i=0; i < 7; i++) 
 { 
 response[i] = portOne.read(); 
 } 
} 


unsigned long getValueA(byte packet[]) 
{ 
 int high = packet[3]; //high byte for value is 4th byte in packet in the packet 
 int low = packet[4]; //low byte for value is 5th byte in the packet 
 unsigned long val = high*256 + low; //Combine high byte and low byte with this formula to get value 
 return val; 

}



//*************************************************************************
void sendRequestB(byte packet[]) 
{ 
 while(!portTwo.available()) //keep sending request until we start to get a response 
 { 
 portTwo.write(readCO2,7); 

 delay(50); 

 } 
 int timeout=0; //set a timeoute counter 
 while(portTwo.available() < 7 ) //Wait to get a 7 byte response 
 { 
 timeout++; 
 if(timeout > 10) //if it takes to long there was probably an error 
 { 
 while(portTwo.available()) //flush whatever we have 
 portTwo.read(); 
 break; //exit and try again 
 } 

 delay(50); 
 
 } 
 for (int i=0; i < 7; i++) 
 {
 response[i] = portTwo.read(); 
 } 

} 


unsigned long getValueB(byte packet[]) 
{ 
 int high = packet[3]; //high byte for value is 4th byte in packet in the packet 
 int low = packet[4]; //low byte for value is 5th byte in the packet 
 unsigned long val = high*256 + low; //Combine high byte and low byte with this formula to get value 
 return val; 

}
