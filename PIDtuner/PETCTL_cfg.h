/*   Stepper driver microstep devision */
#define CFG_STEP_DIV 8
/* Which pin stepper driver STEP pin connected */
#define CFG_STEP_STEP_PIN 6
/* Which pin stepper driver DIR pin connected */
#define CFG_STEP_DIR_PIN 5
/* Which pin stepper driver EN pin connected */
#define CFG_STEP_EN_PIN 7
/* Which pin encoder CLK pin connected */
#define CFG_ENC_CLK 3
/* Which pin encoder DT pin connected */
#define CFG_ENC_DT 2
/* Which pin encoder SW pin connected */
#define CFG_ENC_SW 4
/* Type of encoder: TYPE1 or TYPE2 */
#define CFG_ENC_TYPE TYPE1
/* Initial target temperature [degree C]*/
#define CFG_TEMP_INIT 180
/* Maximum allowed temperature [degree C], allowed to set to 10 degree less */
#define CFG_TEMP_MAX 290
/* Minimum allowed temperature to set [degree C] */
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
//PID p: 12.69  PID i: 0.71 PID d: 57.11
#define CFG_PID_P 12.69
#define CFG_PID_I 0.71
#define CFG_PID_D 57.11
/* Which pin heater MOSFET connected to */
#define CFG_HEATER_PIN 9
/* Target filament bobin diameter [mm] */
#define CFG_BOBIN_DIAM 74
/* Initial pull speed [mm/s] */
#define CFG_SPEED_INIT 2.5
/* Buzzer pin connection */
#define CFG_SOUND_PIN 13
/* Enable startup sound (comment to disable).
   Special for GEORGIY (@nehilo011) :) */
#define CFG_SOUND_START
/* 
 *  Chouse reductor type. 
 * Only one CFG_RED_RA, CFG_RED_PP1 or CFG_RED_PP2 can be uncomment
 */
/* RobertSa/Anatoly reductor variant (1:139.21875 ratio)*/
//#define CFG_RED_RA
/* PETPull Zneipas classic old reductor variant (1:30.9375 ratio)*/
//#define CFG_RED_PP1
/* PETPull-2 Zneipas reductor variant (1:65.68(18) ratio)*/
#define CFG_RED_PP2

/* DON'T CHANGE ANYTHING AFTER THIS LINE IF YOU NOT SHURE TO 146% */

/* 
  enable/disable serial debug output
*/
//#define SERIAL_DEBUG_TEMP 
//#define SERIAL_DEBUG_TEMP_PID 
//#define SERIAL_DEBUG_STEPPER

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
#if defined(CFG_RED_PP2)
#define CFG_RED_G1 34/8
#define CFG_RED_G2 34/11
#define CFG_RED_G3 55/11
#endif //CFG_RED_PP2

/* Gear ratio for RobertSa/Anatoly reductor variant */
/* 
  8 teeth gear on stepper shaft interact with
  36 teeth gear of 1-st gear.
  8 teeth of 1-st gear interact with 
  36 teeth gear of 2-nd gear.
  8 teeth of 2-nd gear interact with 
  55 teeth of target bobin

  reduction ratio 139.21875
*/
#if defined(CFG_RED_RA)
#define CFG_RED_G1 36/8
#define CFG_RED_G2 36/8
#define CFG_RED_G3 55/8
#endif //CFG_RED_RA

/* Gear ratio for PetPull Zneipas reductor variant */
/* 
  8 teeth gear on stepper shaft interact with
  36 teeth gear of 1-st gear.
  8 teeth of 1-st gear interact with 
  55 teeth of target bobin
  CFG_RED_G2 1 - to exclude) 2-nd gear

  reduction ratio 30.9375
*/
#if defined(CFG_RED_PP1)
#define CFG_RED_G1 36/8
#define CFG_RED_G2 1
#define CFG_RED_G3 55/8
#endif //CFG_RED_PP1
