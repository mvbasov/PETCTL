# PID tuner

<img style="max-width:75%;height:auto" src="PID_screen.jpg" alt="Screen photo" />

Подобрать коэффициенты PID регулятора нагревателя можно с помощью этого скетча.
Скетч загружается **вместо** основной прошивки. Время работы скетча не менне 20 минут. Результат работы отображается на экране и в serial console. Коеффициенты P, I и D нужно руками перенести в файл PETCTL_cfg.h

Example:
```
#define CFG_PID_P 12.69
#define CFG_PID_I 0.71
#define CFG_PID_D 57.11
```
