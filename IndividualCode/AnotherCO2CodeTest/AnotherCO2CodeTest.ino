/*
  Software serial multple serial test
 
 Receives from the two software serial ports, 
 sends to the hardware serial port. 
 
 */

#include <SoftwareSerial.h>
// software serial #1: TX = digital pin 10, RX = digital pin 11
SoftwareSerial portOne(12,13);
#include "Wire.h"

// software serial #2: TX = digital pin 8, RX = digital pin 9
// on the Mega, use other pins instead, since 8 and 9 don't work on the Mega
SoftwareSerial portTwo(8,9);

byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25}; //Command packet to read Co2 (see app note) 

byte response[] = {0,0,0,0,0,0,0}; //create an array to store the response 

//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB 

int valMultiplier = 1; 

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);


  // Start each software serial port
  portOne.begin(9600);
  portTwo.begin(9600);
}

void loop()
{
  // By default, the last intialized port is listening.
  // when you want to listen on a port, explicitly select it:
  portOne.listen();
  sendRequestA(readCO2); 
  unsigned long valCO2A = getValueA(response); 
  Serial.print("Co2 A = "); 
  Serial.println(valCO2A); 
  delay(5000);

 

  // blank line to separate data from the two ports:
  Serial.println();

  // Now listen on the second port
  portTwo.listen();
  sendRequestB(readCO2); 
  unsigned long valCO2B = getValueB(response); 
  Serial.print("Co2 B = "); 
  Serial.println(valCO2B);
  delay(5000);


  // blank line to separate data from the two ports:
  Serial.println();
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
 return val* valMultiplier; 

}



//*************************************************************************
void sendRequestB(byte packet[]) 
{ 
 while(!portTwo.available()) //keep sending request until we start to get a response 
 { 
 portTwo.write(readCO2,7); 
  Serial.println("ded");

 delay(50); 

 } 
 int timeout=500; //set a timeoute counter 
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
 return val* valMultiplier; 

}
