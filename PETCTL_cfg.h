//#define SERIAL_DEBUG
/*   Stepper driver microstep devision */
#define CFG_STEP_DIV 4
/* Which pin stepper driver STEP pin connected */
#define CFG_STEP_STEP_PIN 6
/* Which pin stepper driver DIR pin connected */
#define CFG_STEP_DIR_PIN 5
/* Which pin stepper driver EN pin connected */
#define CFG_STEP_EN_PIN 1
#define CFG_ENC_CLK 3
#define CFG_ENC_DT 2
#define CFG_ENC_SW 4
#define CFG_TEMP_INIT 180
#define CFG_TERM_PIN A0
#define CFG_TERM_VALUE 100000
#define CFG_TERM_VALUE_TEMP 25
#define CFG_TERM_B_COEFF 4388
#define CFG_TERM_SERIAL_R 4700
#define CFG_ENDSTOP_PIN 8
// Extra length to pull after end stop triggered (in m)
#define CFG_PULL_EXTRA_LENGTH 0.15
#define CFG_PID_P 14
#define CFG_PID_I 0.93
#define CFG_PID_D 59.87
#define CFG_HEATER_PIN 9
#define CFG_RED_G1 36/8
#define CFG_RED_G2 36/8
#define CFG_RED_G3 55/8
#define CFG_BOBIN_DIAM 74
#define CFG_SPEED_INIT 2.5
