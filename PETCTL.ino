#include "GyverStepper.h"
GStepper<STEPPER2WIRE> stepper(200, 6, 5, 1);
// 6 - STEP
// 5 - DIR
// 1 - EN
#include "GyverTimers.h"

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
float prevTemp = 0;
float targetTemp = 125;

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
#define BCOEFFICIENT 4388
// the value of the 'other' resistor
#define SERIESRESISTOR 4700    
int samples[NUMSAMPLES];

boolean runMotor=false;
long Speed = 530; // degree/sec

boolean Heat = false;

#define CHANGE_NO 0
#define CHANGE_TEMPERATURE 1
#define CHANGE_SPEED 2
int whatToChange = CHANGE_NO;

void setup() {
  // установка макс. скорости в шагах/сек
  stepper.setMaxSpeedDeg(3600);
  // установка ускорения в шагах/сек/сек
  stepper.setAcceleration(500);
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
}

// обработчик
ISR(TIMER2_A) {
  stepper.tick(); // тикаем тут
}

void loop() {
    enc1.tick();
    //stepper.tick();

    long newTargetTemp = targetTemp;
    float curTemp = getTemp();
    long newSpeed = Speed;

    if (curTemp != prevTemp) {
      prevTemp = curTemp;
      oled.setScale(2);      
      oled.setCursorXY(0, 0);
      oled.println(curTemp);   
    }
     

    if (enc1.isDouble()) whatToChange = CHANGE_SPEED;
    if (enc1.isSingle()) whatToChange = CHANGE_TEMPERATURE;

    if( whatToChange == CHANGE_TEMPERATURE) {
      if (enc1.isRight()) newTargetTemp += 1;     // если был поворот направо, увеличиваем на 1
      //if (enc1.isFastR()) newTargetTemp += 10;    // если был быстрый поворот направо, увеличиваем на 10
      if (enc1.isLeft())  newTargetTemp -= 1;     // если был поворот налево, уменьшаем на 1
      //if (enc1.isFastL()) newTargetTemp -= 10;    // если был быстрый поворот налево, уменьшаем на на 10
      if (enc1.isHolded()) Heat = ! Heat;
      if (newTargetTemp != targetTemp) {
        targetTemp = newTargetTemp;
        oled.setScale(2);      
        //oled.home();
        oled.setCursorXY(90, 0);
        oled.println(newTargetTemp);
      }
    } else if (whatToChange == CHANGE_SPEED) {
      if (enc1.isRight()) newSpeed += 1;     // если был поворот направо, увеличиваем на 1
      if (enc1.isLeft())  newSpeed -= 1;     // если был поворот налево, уменьшаем на 1
      if (enc1.isHolded()) {
        runMotor = ! runMotor;
        if (runMotor) {
          stepper.setSpeedDeg(Speed, SMOOTH);        // в градусах/сек
        } else {
          stepper.stop();      
        }
      }
      if (newSpeed != Speed) {
        Speed = newSpeed;
        oled.setScale(2);      
        oled.setCursorXY(0, 23);
        oled.println(newSpeed);
      }
    }
    if (runMotor) {
      oled.setScale(2);
      oled.setCursorXY(0, 47);
      oled.println(stepper.getCurrentDeg() / 360);
    }
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
