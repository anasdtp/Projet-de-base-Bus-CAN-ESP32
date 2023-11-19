#include "espCan.h"



CANMessage rxMsg[SIZE_FIFO];//data received by CAN to control the robot //ce qu'on reçoit sera ecris ici -> Buffer
CANMessage txMsg;//DATA that the robot will write on CAN -> Variable globale utilisé pour ecrire. Pourquoi je l'ai mis en globale? Aucune idée mais flemme de changer les fonctions 

//Pour le Bt :
CANMessage myData;//data received by BT to write on CAN
CANMessage DATAtoSend;//data received by CAN to send on BT

unsigned char FIFO_ecriture = 0;

char vitesse_danger = 0, Stop_Danger = 0, asser_actif = 1, attente = 0, mode_xyt = 0,
                finMvtElem = 0, finRecalage = 0, finRayonCourbure = 0,finRayonCourbureClo = 0, finXYT = 0,  Fin_Match = 0, Message_Fin_Mouvement = 0, explosionErreur = 0; 
int nb_ordres = 0;
// Tout plein de flags
short           etat_automate = 0, etat_automate_depl = 0, new_message = 0,
                xytheta_sens, next_move_xyt = 0, next_move = 0, i, stop = 0, stop_receive = 0, param_xytheta[3],
                etat_automate_xytheta = 0, ralentare = 0;

bool onPrendsEnCompte(uint16_t ID);




void setupCAN(){
  while (!Serial);
  //Serial.println("Base Roulante de Anas Le bg/dg");
  // start the CAN bus at 1000 kbps
  if (!CAN.begin(1000E3)) { //ici, nous avons modifié la bibliotheque CAN pour qu'elle soit compatible avec l'ESP32-E
    //Serial.println("Starting CAN failed!");
    while (1);
  }
  CAN.onReceive(canReadData); //init CAN callback function
}


void canReadData(int packetSize){//Fonction d'interruption se declenchant quand on reçoit une trame CAN
  remplirStruct(rxMsg[FIFO_ecriture], 0,0,0,0,0,0,0,0,0,0); //A tester si ça ne surcharge pas
  rxMsg[FIFO_ecriture].ID = CAN.packetId();
//   if(!onPrendsEnCompte(rxMsg[FIFO_ecriture].ID)){return;} //A decommenter si on veut ignorer les trames qui nous concerne pas
  rxMsg[FIFO_ecriture].ln = CAN.packetDlc();
  //Serial.printf("Received CAN, ID : 0x%.3X ; len : %d\n", rxMsg[FIFO_ecriture].ID, rxMsg[FIFO_ecriture].ln);
  // only print packet rxMsg[FIFO_ecriture].dt for non-RTR packets
  int i = 0;
  while (CAN.available())
  {
    rxMsg[FIFO_ecriture].dt[i]=CAN.read();
    i++;
  }
//   canAvailable = true;
    FIFO_ecriture=(FIFO_ecriture+1)%SIZE_FIFO;//On passe à la prochaine
}

void canReadExtRtr(){//On n'execute plus cette fonction pour compléter la msg CAN car on part du principe qu'on n'utilise pas de ID extented et de msg rtr
  if (CAN.packetExtended()){
      //Serial.print("extended ");
      rxMsg[FIFO_ecriture].extented = true;
  }else{rxMsg[FIFO_ecriture].extented = false;}
  if (CAN.packetRtr()){
      // Remote transmission request, packet contains no rxMsg[FIFO_ecriture].dt
      //Serial.print("RTR ");
      rxMsg[FIFO_ecriture].RTR = true;
  }
  else{rxMsg[FIFO_ecriture].RTR = false;}
  
  //printCANMsg(rxMsg[FIFO_ecriture]);
//   FIFO_ecriture=(FIFO_ecriture+1)%SIZE_FIFO;
}




void remplirStruct(CANMessage &theDATA, int idf, char lenf, char dt0f, char dt1f, char dt2f, char dt3f, char dt4f, char dt5f, char dt6f, char dt7f){
  theDATA.RTR = false;
  if(idf>0x7FF){theDATA.extented = true;}
  else{theDATA.extented = false;}
  theDATA.ID = idf;
  theDATA.ln = lenf;
  theDATA.dt[0] = dt0f;
  theDATA.dt[1] = dt1f;
  theDATA.dt[2] = dt2f;
  theDATA.dt[3] = dt3f;
  theDATA.dt[4] = dt4f;
  theDATA.dt[5] = dt5f;
  theDATA.dt[6] = dt6f;
  theDATA.dt[7] = dt7f;
}
void remplirStruct2x4Bytes(uint32_t id, void *pdata1, void *pdata2){
    txMsg.RTR = false;
    if(id>0x7FF){txMsg.extented = true;}
    else{txMsg.extented = false;}
    txMsg.ID = id;
    txMsg.ln = 8;
    memcpy(&txMsg.dt, pdata1, 4);
    memcpy(&(txMsg.dt[4]), pdata2, 4);
    
}

void writeStructInCAN(const CANMessage &theDATA){
  //Serial.print("Sending ");
  if(theDATA.extented){
    CAN.beginExtendedPacket(theDATA.ID, theDATA.ln, theDATA.RTR);
    Serial.print("extended ");
  }
  else{CAN.beginPacket(theDATA.ID, theDATA.ln, theDATA.RTR);}
  //Serial.print("packet on CAN...");
  if(!theDATA.RTR){CAN.write(theDATA.dt, theDATA.ln);}
  
  CAN.endPacket();
  /*Serial.print(" ID : 0x");
  Serial.print(theDATA.ID, HEX);
  //Serial.println(" done");
  //Serial.println();*/
}

void CANenvoiMsg1x8Bytes(uint32_t id, void *pdata)
{
    txMsg.RTR = false;
    if(id>0x7FF){txMsg.extented = true;}
    else{txMsg.extented = false;}
    txMsg.ID = id;
    txMsg.ln = 8;
    memcpy(&txMsg.dt, pdata, 8);
    writeStructInCAN(txMsg);          
}
void CANenvoiMsg2x4Bytes(uint32_t id, void *pdata1, void *pdata2)
{
    txMsg.RTR = false;
    if(id>0x7FF){txMsg.extented = true;}
    else{txMsg.extented = false;}
    txMsg.ID = id;
    txMsg.ln = 8;
    memcpy(&txMsg.dt, pdata1, 4);
    memcpy(&(txMsg.dt[4]), pdata2, 4);
    writeStructInCAN(txMsg);     
}

void CANenvoiMsg3x2Bytes(uint32_t id, int16_t data1, int16_t data2, int16_t data3){
    txMsg.RTR = false;
    if(id>0x7FF){txMsg.extented = true;}
    else{txMsg.extented = false;}
    txMsg.ID = id;
    txMsg.ln = 6;
    txMsg.dt[0] = data1 & 0xFF;
    txMsg.dt[1] = (data1 >> 8) & 0xFF;
    txMsg.dt[2] = data2 & 0xFF;
    txMsg.dt[3] = (data2 >> 8) & 0xFF;
    txMsg.dt[4] = data3 & 0xFF;
    txMsg.dt[5] = (data3 >> 8) & 0xFF;
    writeStructInCAN(txMsg);
}

void printCANMsg(CANMessage& msg) {
    printf("  ID      = 0x%.3x\n", msg.ID);
    printf("  extented    = %d\n", msg.extented);
    printf("  format rtr = %d\n", msg.RTR);
    printf("  Length  = %d\n", msg.ln);
    printf("  Data    = 0x");            
    for(int i = 0 ; i < msg.ln ; i++){printf(" %.2X", msg.dt[i]);}
    printf("\n");
 }




bool onPrendsEnCompte(uint16_t ID){//Utilser pour ignorer toutes les IDs qui nous concerne pas, pour pas remplir le buffer unitilement et ignorer les trames qui reviennent tous le temps mais nous concerne pas
    switch (ID)
    {
             case 0x01:
                return true;
                break;
            
            default :
            return false;
            break;
    }

    return false;
  
}

