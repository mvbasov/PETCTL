#include <Wire.h>
#import "ASOLED.h"
#include <Encoder.h>
#include <AccelStepper.h>

Encoder myEnc(2, 3); 
// Define stepper motor connections and motor interface type. Motor interface type must be set to 1 when using a driver:
#define dirPin 5
#define stepPin 6
#define heaterPin 11
#define motorInterfaceType 1

// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 4700    
 
int samples[NUMSAMPLES];     

// Create a new instance of the AccelStepper class:
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);
long oldPosition  = -999;

void setup() {
  LD.init();  //initialze OLED display
  LD.clearDisplay();
  LD.SetNormalOrientation(); // pins on top
  //LD.SetTurnedOrientation(); // pins on bottom
  LD.setBrightness(128);
  LD.setNormalDisplay();
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);
  // Set the maximum speed in steps per second:
  stepper.setMaxSpeed(1000);
  stepper.setCurrentPosition(0);
  //analogReference(EXTERNAL);
  digitalWrite(heaterPin,LOW);
}


void loop() {
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    LD.clearDisplay();
    //LD.printString_12x16("Pos.: ", 0, 0);
    LD.printNumber(newPosition, 3, 5, 6);
  }
  // Set the current position to 0:

  // Run the motor forward at 200 steps/second until the motor reaches 400 steps (2 revolutions):
  //while(stepper.currentPosition() != newPosition * 100) {
  //  stepper.setSpeed(200);
  //  stepper.runSpeed();
  //}
  //float reading;
 
  //reading = analogRead(THERMISTORPIN);
  //reading = 1000;
  // convert the value to resistance
  //reading = (1023 / reading)  - 1;     // (1023/ADC - 1) 
  //reading = SERIESRESISTOR / reading;  // 10K / (1023/ADC - 1)

  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
  
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  //Serial.print("Average analog reading "); 
  //Serial.println(average);
  
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);
  
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to C
  
  //Serial.print("Temperature "); 
  //Serial.print(steinhart);
  //Serial.println(" *C");
  LD.printString_12x16("T:      ", 0, 0);
  LD.printNumber(steinhart, 1, 24, 0);
  
  delay(1000);
}
