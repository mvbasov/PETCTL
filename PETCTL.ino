//#define SERIAL_DEBUG
#include "GyverStepper.h"
int stepDiv = 8; // DRV8825: 1/4 MS0=0, MS1=1, MS2=0  TMC2225- 8 MS1+
GStepper<STEPPER2WIRE> stepper(200 * stepDiv, 6, 5, 7);
// 6 - STEP
// 5 - DIR
// 7 - EN
#include "GyverTimers.h"
// Reductor constant ~ 4.69624E-6 m/deg
// 139.21875 - gear ratio
// ((36/8) * (36/8) * (55/8)) = 139.21875
// 232.478 - bobin round length
// 74 * Pi = 232.478
const float REDCONST = 232.478 /(360 * 30.9375 * 1000);

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
GyverPID regulator(38.62, 1.89, 197.71, 300);

boolean runMotor=false;
long Speed = 316; // degree/sec

#define heaterPin 9
boolean Heat = false;

#define CHANGE_NO 0
#define CHANGE_TEMPERATURE 1
#define CHANGE_SPEED 2
int whatToChange = CHANGE_NO;
unsigned long interactive = millis();

void encRotationToValue (long* value, int inc = 1);

void setup() {
#if defined(SERIAL_DEBUG)
  Serial.begin(9600);
#endif //SERIAL_DEBUG
  // установка макс. скорости в градусах/сек
  stepper.setMaxSpeedDeg(3600);
  // установка ускорения в шагах/сек/сек
  stepper.setAcceleration(200);
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
  oled.setScale(1);
  oled.setCursorXY(73,4); //65
  oled.println(">");
  oled.setCursorXY(78,5+5+5+5+11);
  oled.println("ММ/С");
  oled.setCursorXY(78,5+5+5+5+34);
  oled.println("МЕТРОВ");

  oled.setCursorXY(0,16);
  oled.println("---------------------");

  oled.setCursorXY(115,0);
  oled.println("o");
  
  oled.setCursorXY(120,7);
  oled.println("C");

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
      encRotationToValue(&newSpeed, 2);
      if (enc1.isHolded()) {
        runMotor = ! runMotor;
        if (runMotor) {
          stepper.setSpeedDeg(newSpeed, SMOOTH);        // в градусах/сек
          oled.setCursorXY(0, 24);
          oled.println("*");
        } else {
          stepper.stop();      
          oled.setCursorXY(0, 24);
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
      printMilage(stepper.getCurrentDeg());
    }

    curTemp = getTemp();
    regulator.input = curTemp;
    if (curTemp != prevTemp) {
      prevTemp = curTemp;
      printCurrentTemp(curTemp);
    }
#if defined(SERIAL_DEBUG)
    Serial.print(curTemp);
#endif //end SERIAL_DEBUG
    if (Heat) {
      int pidOut = (int) constrain(regulator.getResultTimer(), 0, 255);
      analogWrite(heaterPin, pidOut);
#if defined(SERIAL_DEBUG)
      Serial.print(' ');
      Serial.print(pidOut);
#endif //end SERIAL_DEBUG
    } else {
      analogWrite(heaterPin, 0);
#if defined(SERIAL_DEBUG)
      Serial.print(' ');
      Serial.print(0);
#endif //end SERIAL_DEBUG
    }
#ifdef SERIAL_DEBUG
    Serial.println(' ');
#endif //end SERIAL_DEBUG
    
}

void encRotationToValue (long* value, int inc = 1) {
      if (enc1.isRight()) { *value += inc; interactiveSet(); }     // если был поворот направо, увеличиваем на 1
      if (enc1.isFastR()) { *value += inc * 5; interactiveSet(); }    // если был быстрый поворот направо, увеличиваем на 10
      if (enc1.isLeft())  { *value -= inc; interactiveSet(); }     // если был поворот налево, уменьшаем на 1
      if (enc1.isFastL()) { *value -= inc * 5; interactiveSet(); }    // если был быстрый поворот налево, уменьшаем на на 10
}

void printTargetTemp(float t){
      oled.setScale(2);      
      if(whatToChange == CHANGE_TEMPERATURE)  oled.invertText(true);
      oled.setCursorXY(79, 0);
      oled.println((int)t);  
      oled.invertText(false);
}

void printCurrentTemp(float t) {
      oled.setScale(2);    
      oled.setCursorXY(12, 0);
      oled.println(t, 1);
}

void printSpeed(long s){
      // s - speed in degree per second
      // pint in mm/s
      oled.setScale(2);      
      oled.setCursorXY(12, 24);
      if(whatToChange == CHANGE_SPEED)  oled.invertText(true);
      oled.println(s * REDCONST * 1000,2);  
      oled.invertText(false);
}

void printMilage(float m){
      // m - current stepper position in degree
      // output to display in meters
      oled.setScale(2);
      oled.setCursorXY(12, 47);
      oled.println(m * REDCONST);  
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

  return simpleKalman(steinhart);
}

// Простой “Калман” 
// https://alexgyver.ru/lessons/filters/
float _err_measure = 0.8;  // примерный шум измерений
float _q = 0.1;   // скорость изменения значений 0.001-1, варьировать самому
float simpleKalman(float newVal) {
  float _kalman_gain, _current_estimate;
  static float _err_estimate = _err_measure;
  static float _last_estimate;
  _kalman_gain = (float)_err_estimate / (_err_estimate + _err_measure);
  _current_estimate = _last_estimate + (float)_kalman_gain * (newVal - _last_estimate);
  _err_estimate =  (1.0 - _kalman_gain) * _err_estimate + fabs(_last_estimate - _current_estimate) * _q;
  _last_estimate = _current_estimate;
  return _current_estimate;
}
