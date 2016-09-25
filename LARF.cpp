#include "LARF.H"
#include "Arduino.h"

#include <SPI.h>
#include <RH_RF95.h>

using namespace LARF;

// TODO: optimize listener list to canReceive list
LARF::LARF(float frequency, char canSend[], char canReceive[], char type[]) {
	len = sizeof(buf);

	for (int i = 0; i < MAX_EVENTS; i++) {
		cbTable[i].id[0] = 0;
		cbTable[i].id[1] = 0;
	}

	initiDeviceID();

	pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  initDeviceID_SAMD21();
  Serial.print("ID: "); Serial.println(deviceId);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  String payload = String("cs:\"") + canSend + "\",cr:\"" + canReceive + "\",ty:\"" + type + "\"";
  send("hi", payload.c_str());
}

bool LARF::on(const char[2] name, Callback callback) {
	// You can't have the first character be NULL as it isn't a valid identifier 
	// and it will get overwritten.
	if (name[0] == 0) return false;

	for (int i = 0; i < MAX_EVENTS; i++) {
		if (NAME_CMP(cbTable[i].id, name)) {
			cbTable[i].cb = callback;
			return true;
		} else if (cbTable[i].id[0] == 0) {
			cbTable[i].name[0] = name[0];
			cbTable[i].name[1] = name[1];
			cbTable[i].cb = callback;
			return true;
		}
	}

	// You're out of slots.
	return false;
}

bool LARF::off(const char[2] name) {
	if (name[0] == 0) return false;

	for (int i = 0; i < MAX_EVENTS; i++) {
		if (NAME_CMP(cbTable[i].id, name)) {
			cbTable[i].id[0] = 0;
			cbTable[i].id[1] = 0;
			return true;
		}
	}

	return false;
}

void LARF::send(const char[2] name, char[] payload = NULL) {
	String messageBody = String("i:") + deviceId + ", m:" + name + ", p:" + payload;
	rf95.send((uint8_t *)messageBody.c_str(), messageBody.length());
}

void LARF::update() {
	uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
	uint8_t len;

	if (rf95.recv(buf, &len)) {
		char name[2] = {0,0};
		char from[NUM_ID_CHARS] = {0};
		char payload[MAX_PAYLOAD_SIZE] = {0};
		
		uint_8 state = PARSER_STATE_NONE;
		uint_32 index = 0;
		uint_8 expecting = 0;
		for (int i = 0; i < len; i++) {
			uint8_t c = buf[i];

			if (expecting) {
				if (c == expecting) {
					expecting = 0;
					continue;
				} else {
					Serial.print("D Expected a colon from message, got: ");
					Serial.println(c);
					return;
				}
			}

			switch (state) {
				case PARSER_STATE_PAYLOAD:
					if (c == ',') {
						state = PARSER_STATE_NONE;
						index = 0;
					} else if (index < MAX_PAYLOAD_SIZE) {
						payload[index] = c;
						index++;
					} else {
						state = PARSER_STATE_NONE;
						index = 0;
					}
					break;
				case PARSER_STATE_ID:
					if (index == NUM_ID_CHARS) {
						state = PARSER_STATE_NONE;
						index = 0;
					} else {
						from[index] = c;
						index++;
					}
					break;
				case PARSER_STATE_MESSAGE:
					if (index == 2) {
						state = PARSER_STATE_NONE;
						index = 0;
					} else {
						name[index] = c;
						index++;
					}
					break;
				case PARSER_STATE_NONE:
					switch(c) {
						case ' ':
						case ',':
							break;
						case 'i':
							state = PARSER_STATE_ID;
							expecting = ':';
							break;
						case 'm':
							state = PARSER_STATE_MESSAGE;
							expecting = ':';
							break;
						case 'p':
							state = PARSER_STATE_PAYLOAD;
							expecting = ':';
							break;
						default:
							Serial.print("D Unknown character waiting for key: ");
							Serial.println(c);
							return;
					}
			}
		}

		for (int i = 0; i < MAX_EVENTS; i++) {
			if (NAME_CMP(cbTable[i].id, name)) {
				cbTable[i].cb(name, from, payload);
			}
		}
	}
}


void LARF::initDeviceID() {
	randomSeed(analogRead(OPEN_ANALOG_PIN));
  for (int i = 0; i < NUM_ID_CHARS; i++) {
  	deviceId[i] = ASCII_TABLE[ random(ASCII_TABLE_LEN) ];
  }
}