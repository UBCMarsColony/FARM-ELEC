#include "kSeries.h" //include kSeries Library
kSeries K_30(12,13); //Initialize a kSeries Sensor with pin 12 as Rx and 13 as Tx
int pump = 1;


void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600); //start a serial port to communicate with the computer
   Serial.println("Serial Up!");

}

void loop() {
  // put your main code here, to run repeatedly:
   double co2 = K_30.getCO2('p'); //returns co2 value in ppm ('p') orpercent ('%')
   Serial.print("Co2 ppm = ");
   Serial.println(co2); //print value

   if(co2 < 1100){
    //pass voltage to thing
   
    digitalWrite(pump,1);
   } 


   delay(3000); //wait 3 seconds

}
