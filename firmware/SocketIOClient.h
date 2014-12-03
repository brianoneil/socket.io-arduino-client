/*
	socket.io-arduino-client: a Socket.IO client for the Arduino

	Based on Bill Roy's Arduino Socket.IO Client (which is based on
	Kevin Rohling's Arduino WebSocket Client) with event handling by
	@dantaex

	Copyright 2014 Quentin Pigné

	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef SocketIOClient_H
#define SocketIOClient_H



#include "HashMap.h"

//Length of static data buffers
#define DATA_BUFFER_LEN 120
#define SID_LEN 24

//Max size of the HashTable
#define HASH_SIZE 20

class SocketIOClient {
	public:
		SocketIOClient();
		//Public connection methods
		bool connect(char* hostname, int port = 80, char* resource = "/socket.io/");
		//Public event handling methods
		void setEventHandler(char* eventName, void (*handler)(EthernetClient client, char *data));
		//Public data emitting methods
		void emit(char* event, char* data);
		//Monitoring for incoming data
		void monitor();

	private:
		//Connection attributes
		EthernetClient client;
		char* hostname;
		int port;
		char* resource;
		char sid[SID_LEN];
		//Incoming data reading attributes
		char databuffer[DATA_BUFFER_LEN];
		char* dataptr;
		//Event handling attributes
		int nbEvent;
		static HashType<char*, void(*)(EthernetClient client, char* data)> hashRawArray[HASH_SIZE];
		static HashMap<char*, void(*)(EthernetClient client, char* data)> eventHandlers;

		//Private connection methods
		void sendHandshake();
		bool readHandshake();
		bool waitForInput();
		//Private incoming data reading methods
		void readLine();
		void eatHeader();
		void findColon(char which);
		//Private event handling methods
		char* getName(char* databuffer);
};

#endif
