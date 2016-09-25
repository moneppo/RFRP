#pragma once

#include "Arduino.h"
#define MAX_EVENTS 10
#define NAME_CMP(a, b) (a[0] == b[0] && a[1] == b[1])
#define ASCII_TABLE ("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ")
#define ASCII_TABLE_LEN 60
#define NUM_ID_CHARS 6
#define OPEN_ANALOG_PIN 0
#define MAX_PAYLOAD_SIZE 128

namespace LARF {

	enum PARSER_STATE {
		PARSER_STATE_NONE = 0,
		PARSER_STATE_ID,
		PARSER_STATE_MESSAGE,
		PARSER_STATE_PAYLOAD
	};

	typedef void (*Callback)(const char[] name, char[] from, char[] payload);

	class LARFManager {
	private:
		struct CallbackEntry {
			char id[2];
			Callback cb;
		};

	public:
		LARFManager(float frequency = 915.0f);

		bool on(const char[2] name, Callback callback);

		bool off(const char[2] name);

		void send(const char[2] name, char[] info = NULL);

		void update();

		unsigned char deviceId[NUM_ID_CHARS];

	private:
		CallbackEntry cbTable[MAX_EVENTS];
};