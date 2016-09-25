#include <SPI.h>  
#include <RH_RF95.h>
#include <LARF.h>

LARF::Manager m("hub");

enum State {
  OFFLINE,
  ONLINE,
  READY
};

State dial = OFFLINE;
State buttons = OFFLINE;
State flip = OFFLINE;

void receiveMessage(int eventIndex, const char* from) { 
  switch(eventIndex) {
    case 0: //hello
      if (strcmp(from, "dial")) {
        dial = ONLINE;
      } else if (strcmp(from, "buttons")) {
        buttons = ONLINE;
      } else if (strcmp(from, "flip")) {
        flip = ONLINE;
      }
      if (dial == ONLINE && buttons == ONLINE && flip == ONLINE) {
        // All systems go - show something to that effect on the light board.
      }
      break;
    case 1: //ready
      if (strcmp(from, "dial")) {
        dial = READY;
      } else if (strcmp(from, "buttons")) {
        buttons = READY;
      } else if (strcmp(from, "flip")) {
        flip = READY;
      }
      if (dial == READY && buttons == READY && flip == READY) {
        // Here's where you show something cool. Afterwards, send a reset command.
      }
      break;
    case 2: //error
      // Tell the remote device, whatever it is, to reset itself.
      m.send("reset", from);
  }
}

void setup() {
  Serial.begin(9600 );
  m.initialize("reset", "hello ready error");
  m.onMessage(receiveMessage);  
}

void loop() { 
  m.update();
}


