#include <Arduino.h>

#define TE_2500US  25          //Temps d'echantionnage -> 25 x 100US = 2.5ms

extern volatile uint16_t mscount,      
                         mscount1,     
                         mscount2;  

//----------------------------------------------------------------------Timer
void init_Timer();
void onTime();//prototype de la fonction s'exécutent à chaque interruptions
void TempsEchantionnage(int TIME);