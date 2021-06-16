#include "PETCTL_cfg.h"

//#include "PIDtuner2.h"
//PIDtuner2 tuner;
#include "PIDtuner.h"
PIDtuner tuner;

void setup() {
  Serial.begin(9600);
  long debug_time;
  while (getTemp() > 60.0) {
    if (debug_time < millis() ) {
      debug_time = millis() + 10000;
      Serial.println(getTemp());
    }
  }
  //tuner.setParameters(NORMAL, 150, 200, 1000, 1, 50);
  // направление, начальный сигнал, конечный, период плато, точность, время стабилизации, период итерации
  //tuner.setParameters(NORMAL, 10, 70, 4000, 0.5, 1000);
tuner.setParameters(NORMAL, 40, 15, 5000, 0.08, 15000, 500);
}

void loop() {
  
  tuner.setInput(getTemp());
  tuner.compute();
  analogWrite(CFG_HEATER_PIN, tuner.getOutput());

  // выводит в порт текстовые отладочные данные, включая коэффициенты
  tuner.debugText();

  // выводит в порт данные для построения графиков, без коэффициентов
  //tuner.debugPlot();

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
