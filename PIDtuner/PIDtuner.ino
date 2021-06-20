#include "PETCTL_cfg.h"

#include "PIDtuner.h"
PIDtuner tuner;

#include "GyverOLED.h"
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;

#define RUNTIME uint32_t(1200000) 
uint32_t startTime; 

void setup() {
  pinMode(CFG_SOUND_PIN, OUTPUT);
  
  oled.init();              // инициализация
  // ускорим вывод, ВЫЗЫВАТЬ ПОСЛЕ oled.init()!!!
  Wire.setClock(400000L);   // макс. 800'000

#if defined(CFG_SOUND_START)
  beepE();
#endif
  oled.clear();
  oled.setScale(3);
  oled.setCursor(10, 1);
  oled.println("PETCTL");
  oled.setCursor(30, 4);
  oled.println("PID");
  oled.setScale(1);
  oled.setCursor(20, 7);
  oled.print("mvb    V 0.1");
  delay(3000);
  oled.clear();
  
  static long debug_time = 0;
  Serial.begin(9600);
  Serial.print("Cool down to ");
  Serial.print(CFG_TEMP_INIT - 10);
  Serial.println("C");
  analogWrite(CFG_HEATER_PIN, 0);
  getTemp();
  getTemp();
  //printPID(1000, 1111.11, 2222.22, 3333.33); //for test screen garbage cleaning
  while (getTemp() > CFG_TEMP_INIT - 10.0) {
    if (debug_time < millis() ) {
      debug_time = millis() + 2000;
      Serial.println(getTemp());
      printTemp(getTemp());
      printStatus("Cool Down");
    }
  }
  Serial.print("Heat up to "); 
  Serial.print(CFG_TEMP_INIT);
  Serial.println("C");
  analogWrite(CFG_HEATER_PIN, 128);
  while (getTemp() < CFG_TEMP_INIT + 0.0) {
    if (debug_time < millis() ) {
      debug_time = millis() + 2000;
      Serial.println(getTemp());
      printTemp(getTemp());
      printStatus("Heat Up  ");
    }
  }
  analogWrite(CFG_HEATER_PIN, 0);
  // направление, сигнал, ступенька, период стабилизации, точность стабилизации, продолж. импульса, период итерации  
  tuner.setParameters(NORMAL, 80, 15, 5000, 0.08, 10000, 200);
  startTime = millis();
  Serial.println(startTime);
  beepE();
}
void loop() {
  static float pidP;
  static float pidI;
  static float pidD;
  static int accuracy=0;
  static unsigned long when=millis(); 
  static float curTemp; 

  curTemp = getTemp();
  if (curTemp > 230) {
    analogWrite(CFG_HEATER_PIN, 0);
    Serial.print("*** OVERHEAT 230C ***: ");
    Serial.println(curTemp);
    printTemp(curTemp);
    printStatus("OverHeat!");
    while(1){
        printTemp(getTemp());
    }
  }
  
  tuner.setInput(curTemp);
  tuner.compute();
  analogWrite(CFG_HEATER_PIN, tuner.getOutput());

  if (when < millis()) {
    when = millis() + 10000;
    printTemp(curTemp);
    if(accuracy <= tuner.getAccuracy()){
      accuracy = tuner.getAccuracy();
      pidP=tuner.getPID_p();
      pidI=tuner.getPID_i();
      pidD=tuner.getPID_d();
      printPID(accuracy, pidP, pidI, pidD);
      printStatus("Calculate");
    }
  }

  // выводит в порт текстовые отладочные данные, включая коэффициенты
  tuner.debugText();

  // выводит в порт данные для построения графиков, без коэффициентов
  //tuner.debugPlot();
  long rTime = millis() - startTime;
  printTimer(RUNTIME - rTime);
  if(rTime >= RUNTIME){
    analogWrite(CFG_HEATER_PIN, 0);
    Serial.println(millis()- startTime );
    Serial.println(RUNTIME );
    Serial.println("*** Final result: ***");
    Serial.print("A:");
    Serial.print(accuracy); 
    Serial.print(", P:");
    Serial.print(pidP); 
    Serial.print(", I:");
    Serial.print(pidI); 
    Serial.print(", D:");
    Serial.print(pidD);
    Serial.println(" ");       
    printPID(accuracy, pidP, pidI, pidD);
    printStatus("Done!    ");
    beepI();

    while(1){
      printTemp(getTemp());
    }

  }

}

void beepE() {
  digitalWrite(CFG_SOUND_PIN, 1);
  delay(50);
  digitalWrite(CFG_SOUND_PIN, 0);
  delay(50);
}

void beepI() {
  beepE();
  beepE();
}

void printTemp(float temp){
      oled.setScale(1);      
      oled.setCursor(85, 1);
      oled.print("       ");
      oled.setCursor(90, 1);
      oled.print("T: ");
      oled.println((int)temp, 1);  
}

void printPID(int A, float P, float I, float D){
      static char tbuf[8];
      oled.setScale(1);      
      oled.setCursor(1, 1);
      oled.print("          ");
      oled.setCursor(1, 1);
      oled.print("A: ");
      oled.println(A);
      oled.setCursor(1, 3);
      oled.print("          ");
      oled.setCursor(1, 3);
      oled.print("P: ");
      oled.print(P,2);  
      oled.setCursor(1, 5);
      oled.print("          ");
      oled.setCursor(1, 5);
      oled.print("I: ");
      oled.print(I,2);
      oled.setCursor(1, 7);  
      oled.print("          ");
      oled.setCursor(1, 7);  
      oled.print("D: ");
      oled.print(D,2);  
}

void printStatus(String s){
      oled.setScale(1);      
      oled.setCursor(65, 7);
      oled.println(s);  
  
}

void printTimer(long msec){
      static char tbuf[6];
      int sec = msec / 1000;
      sprintf_P(tbuf,PSTR("%02d:%02d"), sec / 60, sec % 60);
      oled.setScale(1);      
      oled.setCursor(85, 5);
      oled.print(tbuf);  
}

float getTemp() {
  uint8_t i;
  float average;

  average = analogRead(CFG_TERM_PIN);
  // convert the value to resistance
  average = 1023 / average - 1;
  average = CFG_TERM_SERIAL_R / average;
  //Serial.print("Thermistor resistance ");
  //Serial.println(average);

  float steinhart;
  steinhart = average / CFG_TERM_VALUE;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= CFG_TERM_B_COEFF;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (CFG_TERM_VALUE_TEMP + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to C

  return simpleKalman(steinhart);
}

// Простой “Калман”
// https://alexgyver.ru/lessons/filters/
float _err_measure = 0.8;  // примерный шум измерений
float _q = 0.02;   // скорость изменения значений 0.001-1, варьировать самому

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
