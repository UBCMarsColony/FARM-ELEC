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
// Permission is hereby granted, free of charge, to any person obtaining a copym
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
#include <SoftwareSerial.h>

// BME280
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Constants
#define RX_C 8                   // rx pin for CO2 (control)
#define TX_C 9                   // tx pin for CO2 (control)
#define RX_L 12                  // rx pin for CO2 (lpgc)
#define TX_L 13                  // tx pin for CO2 (lpgc)
#define CO2_LOWER_THRESHOLD 1100 // threshold below which to enable pump
#define CO2_UPPER_THRESHOLD 1300 // threshold above which to enable pump
#define RELAY_C A0               // pump pin connection (control)
#define RELAY_L A1               // pump pin connection (lpgc)

// Initializations
SoftwareSerial portOne(RX_C, TX_C); // initialize CO2 sensor for kSeries k30 (control)
SoftwareSerial portTwo(RX_L, TX_L); // initialize CO2 sensor for kSeries k30 (lpgc)

Adafruit_BME280 bme_control; // initialize BME280 for I2C (control)
Adafruit_BME280 bme_lpgc;    // initialize BME280 for I2C (lpgc)

// Variables
float co2_control;  // stores CO2 value (control)
float co2_lpgc;     // stores CO2 value (lpgc)
float hum_control;  // stores humidity value (control)
float hum_lpgc;     // stores humidity value (lpgc)
float temp_control; // stores temperature value (control)
float temp_lpgc;    // stores temperature value (lpgc)
float pres_control; // stores pressure value (control)
float pres_lpgc;    // stores pressure value (lpgc)

unsigned long delayTime = 5000;                              // current delay time between readings
int bmeAddr_control = 0x76;                                  // BME280 sensor address for control chamber
int bmeAddr_lpgc = 0x77;                                     // BME280 sensor address for lpgc
byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25}; // command packet to read CO2 (see app note)
byte response[] = {0, 0, 0, 0, 0, 0, 0};                     // create an array to store the response

// Serial data variables ------------------------------------------------------
const byte kNumberOfChannelsFromExcel = 6; // incoming serial data array
const char kDelimiter = ',';               // comma delimiter to separate consecutive data if using more than 1 sensor
const int kSerialInterval = 50;            // interval between serial writes
unsigned long serialPreviousTime;          // timestamp to track serial interval
char *arr[kNumberOfChannelsFromExcel];     // array to parse data

int tmpbool = 0; // tmp var to switch pump on/off

// SETUP ----------------------------------------------------------------------
void setup()
{
  // Initialize Serial Communication
  Serial.begin(9600);
  // Start each software serial port
  portOne.begin(9600);
  portTwo.begin(9600);
  // Start BMEs
  bme_control.begin(0x76, &Wire);
  bme_lpgc.begin(0x77, &Wire);

  // Set pump relay pin mode
  pinMode(RELAY_C, OUTPUT);
  pinMode(RELAY_L, OUTPUT);
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

  // Control BME
  hum_control = bme_control.readHumidity();           // read humidity
  temp_control = bme_control.readTemperature();       // read temperature
  pres_control = bme_control.readPressure() / 100.0F; // read pressure

  // LPGC BME
  hum_lpgc = bme_lpgc.readHumidity();           // read humidity
  temp_lpgc = bme_lpgc.readTemperature();       // read temperature
  pres_lpgc = bme_lpgc.readPressure() / 100.0F; // read pressure

  // Listen to first CO2 port
  portOne.listen();
  sendRequestA(readCO2);
  co2_control = getValueA(response);

  // Listen to second CO2 port
  portTwo.listen();
  sendRequestB(readCO2);
  co2_lpgc = getValueB(response);

  // CO2 Pump Loops
  tmpbool = ~tmpbool;
  CO2PumpLoop(RELAY_C, tmpbool); // loop to control pump
  CO2PumpLoop(RELAY_L, tmpbool); // loop to lpgc pump

  delay(delayTime); // delay between signal reads
}

// Add any specialized methods and processing code below

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void sendDataToSerial()
{
  // Send data out separated by a comma (kDelimiter)
  // Repeat next 2 lines of code for each variable sent:
  Serial.print(co2_control);
  Serial.print(kDelimiter);

  Serial.print(hum_control);
  Serial.print(kDelimiter);

  Serial.print(temp_control);
  Serial.print(kDelimiter);

  Serial.print(pres_control);
  Serial.print(kDelimiter);

  Serial.print(co2_lpgc);
  Serial.print(kDelimiter);

  Serial.print(hum_lpgc);
  Serial.print(kDelimiter);

  Serial.print(temp_lpgc);
  Serial.print(kDelimiter);

  Serial.print(pres_lpgc);
  Serial.print(kDelimiter);

  Serial.println(); // add final line ending character only once
}

// CO2 LOOP CODE--------------------------------------------------------------
void CO2PumpLoop(uint8_t relay, int co2)
{
  //! Actual code needs to continuously read co2 till it reaches CO2_UPPER_THRESHOLD
  //! Need to check if we can do interrupts, or test with different delayTime for most efficient opening/closing of pump
  if (co2 == 1)
    digitalWrite(relay, HIGH);
  else
    digitalWrite(relay, LOW);

  // if (co2 < CO2_LOWER_THRESHOLD)
  //   digitalWrite(RELAY_PUMP, HIGH);
  // else if (co2 >= CO2_UPPER_THRESHOLD)
  //   digitalWrite(RELAY_PUMP, LOW);
}

//-----------------------------------------------------------------------------
// DO NOT EDIT ANYTHING BELOW THIS LINE
//-----------------------------------------------------------------------------
// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void processOutgoingSerial()
{
  // Enter into this only when serial interval has elapsed
  if ((millis() - serialPreviousTime) > kSerialInterval)
  {
    // Reset serial interval timestamp
    serialPreviousTime = millis();
    sendDataToSerial();
  }
}
// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
void processIncomingSerial()
{
  if (Serial.available())
  {
    parseData(GetSerialData());
  }
}
// Gathers bytes from serial port to build inputString
char *GetSerialData()
{
  static char inputString[64];                 // Create a char array to store incoming data
  memset(inputString, 0, sizeof(inputString)); // Clear the memory from a pervious reading
  while (Serial.available())
  {
    Serial.readBytesUntil('\n', inputString, 64); // Read every byte in Serial buffer until line end or 64 bytes
  }
  return inputString;
}
// Seperate the data at each delimeter
void parseData(char data[])
{
  char *token = strtok(data, ","); // Find the first delimeter and return the token before it
  int index = 0;                   // Index to track storage in the array
  while (token != NULL)
  {                            // Char* strings terminate w/ a Null character. We'll keep running the command until we hit it
    arr[index] = token;        // Assign the token to an array
    token = strtok(NULL, ","); // Conintue to the next delimeter
    index++;                   // incremenet index to store next value
  }
}

//*************************************************************************
void sendRequestA(byte packet[])
{
  while (!portOne.available()) // keep sending request until we start to get a response
  {
    portOne.write(readCO2, 7);

    delay(50);
  }
  int timeout = 0;                // set a timeoute counter
  while (portOne.available() < 7) // Wait to get a 7 byte response
  {
    timeout++;
    if (timeout > 10) // if it takes to long there was probably an error
    {
      while (portOne.available()) // flush whatever we have
        portOne.read();
      break; // exit and try again
    }
    delay(50);
  }
  for (int i = 0; i < 7; i++)
  {
    response[i] = portOne.read();
  }
}

unsigned long getValueA(byte packet[])
{
  int high = packet[3];                 // high byte for value is 4th byte in packet in the packet
  int low = packet[4];                  // low byte for value is 5th byte in the packet
  unsigned long val = high * 256 + low; // Combine high byte and low byte with this formula to get value
  return val;
}

//*************************************************************************
void sendRequestB(byte packet[])
{
  while (!portTwo.available()) // keep sending request until we start to get a response
  {
    portTwo.write(readCO2, 7);

    delay(50);
  }
  int timeout = 0;                // set a timeoute counter
  while (portTwo.available() < 7) // Wait to get a 7 byte response
  {
    timeout++;
    if (timeout > 10) // if it takes to long there was probably an error
    {
      while (portTwo.available()) // flush whatever we have
        portTwo.read();
      break; // exit and try again
    }

    delay(50);
  }
  for (int i = 0; i < 7; i++)
  {
    response[i] = portTwo.read();
  }
}

unsigned long getValueB(byte packet[])
{
  int high = packet[3];                 // high byte for value is 4th byte in packet in the packet
  int low = packet[4];                  // low byte for value is 5th byte in the packet
  unsigned long val = high * 256 + low; // Combine high byte and low byte with this formula to get value
  return val;
}
