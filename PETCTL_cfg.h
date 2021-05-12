/* enable/disable output temperature and PID output for debug */
//#define SERIAL_DEBUG
/*   Stepper driver microstep devision */
#define CFG_STEP_DIV 8
/* Which pin stepper driver STEP pin connected */
#define CFG_STEP_STEP_PIN 6
/* Which pin stepper driver DIR pin connected */
#define CFG_STEP_DIR_PIN 5
/* Which pin stepper driver EN pin connected */
#define CFG_STEP_EN_PIN 1
/* Which pin encoder CLK pin connected */
#define CFG_ENC_CLK 3
/* Which pin encoder DT pin connected */
#define CFG_ENC_DT 2
/* Which pin encoder SW pin connected */
#define CFG_ENC_SW 4
/* Initial target temperature [degree C]*/
#define CFG_TEMP_INIT 180
/* Maximum allowed temperature [degree C] */
#define CFG_TEMP_MAX 290
/* Maximum allowed temperature to set [degree C] */
#define CFG_TEMP_MIN 120
/* Which pin termistor connected to*/
#define CFG_TERM_PIN A0
/* Thermistor resistance at 25 degrees C [Om] */
#define CFG_TERM_VALUE 100000
/* Thermistor temperature for nominal resistance (almost always 25 C) [degree C] */
#define CFG_TERM_VALUE_TEMP 25
/* The beta coefficient of the thermistor (usually 3000-4000) */
#define CFG_TERM_B_COEFF 4388
/* the value of the 'other' resistor [Om] */
#define CFG_TERM_SERIAL_R 4700
/* Which pin endstop connected to */
#define CFG_ENDSTOP_PIN 8
/* Extra length to pull after end stop triggered [m] */
#define CFG_PULL_EXTRA_LENGTH 0.07
/* PID regulator coefficients */
#define CFG_PID_P 14
#define CFG_PID_I 0.93
#define CFG_PID_D 59.87
/* Which pin heater MOSFET connected to */
#define CFG_HEATER_PIN 9
/* Gear ratio for PETPull-2 Zneipas reductor variant */
/* 
  8 teeth gear on stepper shaft interact with
  34 teeth gear of 1-st gear.
  11 teeth of 1-st gear interact with 
  34teeth gear of 2-nd gear.
  11 teeth of 2-nd gear interact with 
  55 teeth of target bobin

  reduction ratio 65.68(18)
*/
#define CFG_RED_G1 34/8
#define CFG_RED_G2 34/11
#define CFG_RED_G3 55/11
/* Gear ratio for RobertSa reductor variant */
/* 
  8 teeth gear on stepper shaft interact with
  36 teeth gear of 1-st gear.
  8 teeth of 1-st gear interact with 
  36 teeth gear of 2-nd gear.
  8 teeth of 2-nd gear interact with 
  55 teeth of target bobin

  reduction ratio 139.21875
*/
//#define CFG_RED_G1 36/8
//#define CFG_RED_G2 36/8
//#define CFG_RED_G3 55/8
/* Gear ratio for PetPull Zneipas reductor variant */
/* 
  8 teeth gear on stepper shaft interact with
  36 teeth gear of 1-st gear.
  8 teeth of 1-st gear interact with 
  55 teeth of target bobin
  CFG_RED_G2 1 - to exclude) 2-nd gear

  reduction ratio 30.9375
*/
//#define CFG_RED_G1 36/8
//#define CFG_RED_G2 1
//#define CFG_RED_G3 55/8
/* Target filament bobin diameter [mm] */
#define CFG_BOBIN_DIAM 74
/* Initial pull speed [mm/s] */
#define CFG_SPEED_INIT 2.5
