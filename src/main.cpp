#include <Arduino.h>
#include <timerAsserBas.h>
#include <espCan.h>

void CANloop();

void setup() {
  Serial.begin(921600);
  setupCAN(); //Attention ne marche qu'avec l'esp32-32E. Si tu utilises un autre esp32, supprime la bibli CAN de /lib et rajoute la bibli CAN de base qui est non modifié sur platformio lien : https://github.com/sandeepmistry/arduino-CAN.git
  Serial.printf("fin init setupCAN\n");
  init_Timer(); Serial.printf("fin init Timer\n");

  remplirStruct(txMsg, 0x01,0,0,0,0,0,0,0,0,0);
  writeStructInCAN(txMsg); Serial.printf("envoie CAN ALIVE\n");

  Serial.println("fin setup\n");
  mscount = 0;
}

void loop() {
  CANloop();

  TempsEchantionnage(TE_2500US);   
}

/***************************************************************************************
 NOM : CANloop                                                              
 ARGUMENT : aucun                                        
 RETOUR : rien                                                                        
 DESCRIPTIF :   Fonction principale appelé periodiquement, qui gère la communication avec le CAN et le Bluetooth
                Chaque id de message reçu, une tâche associé 
***************************************************************************************/
void CANloop(){
    //FIFO = Buffer. On traite chaque trames CAN une par une mais on continue d'en recevoir pendant le traitement. On les stocke dans un tableau
    //Donc FIFO_ecriture est la place dans ce tableau où on ecrit et FIFO_lecture est la place de la trame qu'on est actuellement entrain de lire
    static signed char FIFO_lecture=0,FIFO_occupation=0,FIFO_max_occupation=0; 

    FIFO_occupation=FIFO_ecriture-FIFO_lecture;
    if(FIFO_occupation<0){FIFO_occupation=FIFO_occupation+SIZE_FIFO;}
    if(FIFO_max_occupation<FIFO_occupation){FIFO_max_occupation=FIFO_occupation;}

/*   //if(canAvailable || BtAvailable){
    // if(canAvailable){canReadExtRtr();}//On le me ici pour ne pas surcharger l'interruption CAN.onRecveive
    // canAvailable = false; BtAvailable = false;
    */
    Serial.println("CAN received  :  ");
    printCANMsg(rxMsg[FIFO_lecture]);

    if(!FIFO_occupation){return;}
    switch (rxMsg[FIFO_lecture].ID)
    {

            case 0x02: //Si on reçoit l'ID 0x02 alors on fait ce qui suit
                //Serial.println("ESP32_RESTART");
                //esp_restart();
                
                break;

//---------------------------------------------
            
            default :
            break;
    }
    
    /*
    // Send message to master via bleutooth
    if (connected){
      prxRemoteCharacteristic->writeValue((uint8_t *)&rxMsg[FIFO_lecture], sizeof(rxMsg[FIFO_lecture]));
      //Serial.println("Sending via BT...");
    } else{//Serial.println("The device is not connected");}
//   }
  //if new CAN by BT are available, write it in CAN bus / CAN <-> Bt <-> CAN and use it to control the Robot
//   if (newCan){
//     newCan = false;
//     writeStructInCAN(rxMsg[FIFO_lecture]); 
//     ////Serial.println(rxMsg[FIFO_lecture].ID);
//     BtAvailable = true;
//   }*/
    FIFO_lecture=(FIFO_lecture+1)%SIZE_FIFO;
  
}
