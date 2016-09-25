#include <SPI.h>  
#include <RH_RF95.h>
#include <LARF.h>

// We'll do the flipswitch since it's such simple logic.
LARF::Manager m("flip");

bool switchFlipped = false;
int SWITCH = 4;

void receiveMessage(int eventIndex, const char* from) {
  // Really, the only event a remote node listens for is reset, 
  // but we'll keep the switch statement here in case we ever want 
  // to add anything. 
  switch(eventIndex) {
    case 0: //reset
      switchFlipped = false;
      m.send("hello", "hub");
      break;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(SWITCH, INPUT);
  m.initialize("hello ready error", "reset");
  m.onMessage(receiveMessage);  
}

void loop() {
  if (digitalRead(SWITCH) && switchFlipped == false) {
    m.send("ready", "hub");
    switchFlipped = true;
  }
  m.update();
}


