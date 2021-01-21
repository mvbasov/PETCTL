#include "GyverStepper.h"
int stepDiv = 4; // DRV8825: 1/4 MS0=0, MS1=1, MS2=0
GStepper<STEPPER2WIRE> stepper(200 * stepDiv, 6, 5, 1);
// 6 - STEP
// 5 - DIR
// 1 - EN
#include "GyverTimers.h"
// Reductor constant ~ 4.69624E-6 (139 - gear ratio, 235 - bobin round length)
const float REDCONST = 232.5 /(360 * 139.0 * 1000);

#include "GyverOLED.h"
// попробуй с буфером и без
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
// можно передать адрес: GyverOLED oled(0x3C);

#define CLK 3
#define DT 2
#define SW 4
#include "GyverEncoder.h"
Encoder enc1(CLK, DT, SW);
int value = 0;

// Termistor definition
float prevTemp, curTemp = 0;
float targetTemp = 180;

// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 100000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 4388
// the value of the 'other' resistor
#define SERIESRESISTOR 4700    
int samples[NUMSAMPLES];

#include "GyverPID.h"
GyverPID regulator(14, 0.93, 59.87, 200);

boolean runMotor=false;
long Speed = 530; // degree/sec

#define heaterPin 9
boolean Heat = false;

#define CHANGE_NO 0
#define CHANGE_TEMPERATURE 1
#define CHANGE_SPEED 2
int whatToChange = CHANGE_NO;
unsigned long interactive = millis();

void setup() {
  // установка макс. скорости в градусах/сек
  stepper.setMaxSpeedDeg(3600);
  // установка ускорения в шагах/сек/сек
  stepper.setAcceleration(300);
  // настраиваем прерывания с периодом, при котором 
  // система сможет обеспечить максимальную скорость мотора.
  // Для большей плавности лучше лучше взять период чуть меньше, например в два раза
  Timer2.setPeriod(stepper.getMinPeriod() / 2);
  // взводим прерывание
  Timer2.enableISR();
  stepper.setRunMode(KEEP_SPEED);   // режим поддержания скорости
 
  oled.init();              // инициализация
  // ускорим вывод, ВЫЗЫВАТЬ ПОСЛЕ oled.init()!!!
  Wire.setClock(400000L);   // макс. 800'000
  oled.clear();

  enc1.setType(TYPE1);
  enc1.setPinMode(LOW_PULL);

  regulator.setpoint = targetTemp;
  printSpeed(Speed);
  printTargetTemp(targetTemp);
  printMilage(0.0);
}

// обработчик
ISR(TIMER2_A) {
  stepper.tick(); // тикаем тут
  enc1.tick();
}

void loop() {
    enc1.tick();
    //stepper.tick();

    long newTargetTemp = targetTemp;
    long newSpeed = Speed;

    if (enc1.isDouble()) {
      whatToChange = CHANGE_SPEED;
      interactiveSet();
      printTargetTemp(targetTemp); // to clear selection
      printSpeed(Speed);
    }
    if (enc1.isSingle()) {
      whatToChange = CHANGE_TEMPERATURE;
      interactiveSet();
      printSpeed(Speed); // to clear selection
      printTargetTemp(targetTemp);
    }
    if (!isInteractive()) {
      whatToChange = CHANGE_NO;
      printSpeed(Speed); // to clear selection
      printTargetTemp(targetTemp);
    }

    if( whatToChange == CHANGE_TEMPERATURE) {
      encRotationToValue(&newTargetTemp);
      if (enc1.isHolded()){
        Heat = ! Heat;
        oled.setCursorXY(0, 0);
        if(Heat) 
          oled.println("*");
        else
          oled.println(".");
      }

      if (newTargetTemp != targetTemp) {
        targetTemp = newTargetTemp;
        regulator.setpoint = newTargetTemp;
        printTargetTemp(newTargetTemp);
      }
    } else if (whatToChange == CHANGE_SPEED) {
      encRotationToValue(&newSpeed);
      if (enc1.isHolded()) {
        runMotor = ! runMotor;
        if (runMotor) {
          stepper.setSpeedDeg(newSpeed, SMOOTH);        // в градусах/сек
          oled.setCursorXY(0, 23);
          oled.println("*");
        } else {
          stepper.stop();      
          oled.setCursorXY(0, 23);
          oled.println(".");
        }
        interactiveSet();
      }
      if (newSpeed != Speed) {
        Speed = newSpeed;
        if (runMotor) stepper.setSpeedDeg(newSpeed, SMOOTH);        // в градусах/сек
        printSpeed(newSpeed);
      }
    }
    if (runMotor) {
      printMilage(stepper.getCurrentDeg() / 360);
    }

    curTemp = getTemp();
    regulator.input = curTemp;
    if (curTemp != prevTemp) {
      prevTemp = curTemp;
      printCurrentTemp(curTemp);
    }
         
    if (Heat) {
      analogWrite(heaterPin, regulator.getResultTimer());
    } else {
      analogWrite(heaterPin, 0);
    }
}

void encRotationToValue (long* value) {
      if (enc1.isRight()) { *value += 1; interactiveSet(); }     // если был поворот направо, увеличиваем на 1
      if (enc1.isFastR()) { *value += 10; interactiveSet(); }    // если был быстрый поворот направо, увеличиваем на 10
      if (enc1.isLeft())  { *value -= 1; interactiveSet(); }     // если был поворот налево, уменьшаем на 1
      if (enc1.isFastL()) { *value -= 10; interactiveSet(); }    // если был быстрый поворот налево, уменьшаем на на 10
}

void printTargetTemp(float t){
      oled.setScale(2);      
      //oled.home();
      if(whatToChange == CHANGE_TEMPERATURE)  oled.invertText(true);
      oled.setCursorXY(88, 0);
      oled.println((int)t);  
      oled.invertText(false);
}

void printCurrentTemp(float t) {
      oled.setScale(2);      
      oled.setCursorXY(12, 0);
      oled.println(t, 1);   
}

void printSpeed(long s){
      oled.setScale(2);      
      oled.setCursorXY(12, 23);
      if(whatToChange == CHANGE_SPEED)  oled.invertText(true);
      oled.println(s * REDCONST * 1000);  
      oled.invertText(false);
}

void printMilage(float m){
      oled.setScale(2);
      oled.setCursorXY(12, 47);
      oled.println(stepper.getCurrentDeg() * REDCONST);  
}

void interactiveSet() {
  interactive = millis() + 15000;
}

boolean isInteractive() {
  return millis() < interactive;
}

float getTemp() {
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

  return steinhart;
}
