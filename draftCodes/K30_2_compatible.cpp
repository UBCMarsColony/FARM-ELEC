/*
  Basic Arduino example for K-Series sensor
  Created by Jason Berger
  
  *edited to use 2 sensors on the same line*
  Co2meter.com  
*/


#include "SoftwareSerial.h"


byte addressA = 0x68;
byte addressB = 0x34;	//change to address


SoftwareSerial K_30_Serial(12,13);  //Sets up a virtual serial port
                                    //Using pin 12 for Rx and pin 13 for Tx


byte readCO2_a[] = {addressA, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};  //Command packet to read Co2 (see app note)
byte readCO2_b[] = {addressB, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25};
byte response[] = {0,0,0,0,0,0,0};  //create an array to store the response

unsigned long valCO2_A;
unsigned long valCO2_B;

//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);         //Opens the main serial port to communicate with the computer
  K_30_Serial.begin(9600);    //Opens the virtual serial port with a baud of 9600
  
  
  
  //Uncomment the following line to and run to change address of connected sensor to 0x34;
  //This should only be done with one sensor attached
  //changeAddress();
}

void loop() 
{
  sendRequest(readCO2_a);			//send request to A and store response
  valCO2_A = getValue(response);	//parse response and store as valCO2_A
  
  sendRequest(readCO2_b);			//send request to B and store response
  valCO2_B = getValue(response);	//parse response and store as valCO2_A
  
  Serial.print("Co2[A] ppm = ");
  Serial.println(valCO2_A);
   Serial.print("Co2[B] ppm = ");
  Serial.println(valCO2_B);
  delay(2000);
  
}

void sendRequest(byte packet[])
{
  while(!K_30_Serial.available())  //keep sending request until we start to get a response
  {
    K_30_Serial.write(readCO2,7);
    delay(50);
  }
  
  int timeout=0;  //set a timeoute counter
  while(K_30_Serial.available() < 7 ) //Wait to get a 7 byte response
  {
    timeout++;  
    if(timeout > 10)    //if it takes to long there was probably an error
      {
        while(K_30_Serial.available())  //flush whatever we have
          K_30_Serial.read();
          
          break;                        //exit and try again
      }
      delay(50);
  }
  
  for (int i=0; i < 7; i++)
  {
    response[i] = K_30_Serial.read();
  }  
}

unsigned long getValue(byte packet[])
{
    int high = packet[3];                        //high byte for value is 4th byte in packet in the packet
    int low = packet[4];                         //low byte for value is 5th byte in the packet

  
    unsigned long val = high*256 + low;                //Combine high byte and low byte with this formula to get value
    return val* valMultiplier;
}

void changeAddress()
{

	//The adress is stored in the eeprom at address 0x00 and RAM address 0x20
	//if it is changed in eeprom it will automatically be loaded into ram after a power cycle
	//but we will write it manually so we dont have to powercycle
	//
	//if desired adress is changed then the CRC (last 2 bytes) must be recalculated also
	//
	byte changeEEPROM[] = {0xFE,0x43,0x00,0x00,0x1,0x34,0x50,0x4d};
	byte changeRAM[] =  {0xFE,0x41,0x00,0x20,0x1,0x34,0x28,0x47};
	
	K_30_Serial.write(changeEEPROM,8);	//Send each one 3 times just to ensure it goes through
	delay(100);
	K_30_Serial.write(changeEEPROM,8);
	delay(100);
	K_30_Serial.write(changeEEPROM,8);
	delay(100);
	K_30_Serial.write(changeRAM,8);
	delay(100);
	K_30_Serial.write(changeRAM,8);
	delay(100);
	K_30_Serial.write(changeRAM,8);
	delay(100)
}