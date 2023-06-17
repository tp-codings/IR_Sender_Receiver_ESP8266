#include <Arduino.h>
#include <IRremote.hpp>

//Pinbelegung
const int RECV_PIN = 5;

void setup(){
  Serial.begin(115200);

  //Initialisierung des IR-Empfängers
  IrReceiver.begin(RECV_PIN, ENABLE_LED_FEEDBACK);

  //Ausgabe der aktiven Protokolle, die der Empfänger empfangen kann
  printActiveIRProtocols(&Serial);
}

void loop(){

  //Dekodierung der empfangenen Signale
  if(IrReceiver.decode()){

    //Ausgabe der empfangenen Protokolle
    IrReceiver.printIRResultShort(&Serial);
    Serial.println();
    
    //Fortsetzen des Empfangs von IR-Signalen
    IrReceiver.resume();
  }
}


