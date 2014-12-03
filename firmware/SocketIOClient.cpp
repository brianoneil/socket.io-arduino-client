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

#include "SocketIOClient.h"

//Event handling attributes
HashType <char*, void(*)(EthernetClient client, char* data)> SocketIOClient::hashRawArray[HASH_SIZE];
HashMap  <char*, void(*)(EthernetClient client, char* data)> SocketIOClient::eventHandlers = HashMap<char*, void(*)(EthernetClient client, char* data)>(hashRawArray, HASH_SIZE);

//By default constructor, called when a client is instantiated
SocketIOClient::SocketIOClient() {
	//At the beginning, the number of handled event is null
	nbEvent = 0;	
}

/*
	Connection methods
*/

bool SocketIOClient::connect(char* theHostname, int thePort, char* theResource) {
	//Ethernet connection as a client, if it fails, socket connection cannot be done
	if(!client.connect(theHostname, thePort)) return false;
	hostname = theHostname;
	port = thePort;
	resource = theResource;
	//Send handshake to start the socket connection
	sendHandshake();
	//Read the handshake's response to know if connection succeed
	return readHandshake();
}

void SocketIOClient::sendHandshake() {
	//Construction of the HTTP request
	client.print(F("GET "));
	client.print(resource);
	client.println(F("1/ HTTP/1.1"));
	client.print(F("Host: "));
	client.print(hostname);
	client.print(F(":"));
	client.println(port);
	client.println(F("Origin: Arduino\r\n"));
}

bool SocketIOClient::readHandshake(){
	//Check for the server's response
	if(!waitForInput()) return false;

	//Check for the "HTTP/1.1 200 OK" response
	readLine();
	if(atoi(&databuffer[9]) != 200) {
		while(client.available()) readLine();
		client.stop();
		return false;
	}

	//Go to the line [sid:transport:timeout:available_transports]
	//corresponding to the last line of the response
	eatHeader(); //Consume the rest of the header
	readLine(); //Read the next line and save it
	char databuffer_save[DATA_BUFFER_LEN];
	memcpy(databuffer_save, (const char*) databuffer, DATA_BUFFER_LEN);
	readLine(); //Read again
	//Loop until the line isn't empty
	while(*databuffer != '\0') {
		memcpy(databuffer_save, (const char*) databuffer, DATA_BUFFER_LEN);
		readLine();
	}
	memcpy(databuffer, (const char*) databuffer_save, DATA_BUFFER_LEN);

	//Get the SID in the response
	char* iptr = databuffer;
	char* optr = sid;
	while(*iptr && (*iptr != ':') && (optr < &sid[SID_LEN - 2])) *optr++ = *iptr++;
	*optr = 0;

	Serial.print(F("Connected. SID = "));
	Serial.println(sid);

	//Consume the rest of the response
	while(client.available()) readLine();

	//Stop the connection
	client.stop();
	delay(1000);

	//Reconnect on WebSocket connection
	Serial.print(F("WebSocket Connect... "));
	if(!client.connect(hostname, port)) {
		Serial.print(F("Reconnect failed."));
		return false;
	}
	Serial.println(F("Reconnected."));

	//Construction of the protocol switching request
	client.print(F("GET "));
	client.print(resource);
	client.print(F("1/websocket/"));
	client.print(sid);
	client.println(F(" HTTP/1.1"));
	client.print(F("Host: "));
	client.print(hostname);
	client.print(F(":"));
	client.println(port);
	client.println(F("Origin: ArduinoSocketIOClient"));
	client.println(F("Upgrade: WebSocket"));
	client.println(F("Connection: Upgrade\r\n"));

	//Check for the server's response
	if(!waitForInput()) return false;

	//Check for the "HTTP/1.1 101 Switching Protocols" response
	readLine();
	if(atoi(&databuffer[9]) != 101) {
		while (client.available()) readLine();
		client.stop();
		return false;
	}

	eatHeader(); //Consume the rest of the header
	monitor(); //Treat the response as input

	return true;
}

//Server unreachable if it takes more than 30sec to answer
bool SocketIOClient::waitForInput() {
	unsigned long now = millis();
	while(!client.available() && ((millis() - now) < 30000UL)) {}
	return client.available();
}

//Consume the header
void SocketIOClient::eatHeader() {
	//Consume lines to the empty line between the end of the header and the beginning of the response body
	while(client.available()) {
		readLine();
		if(strlen(databuffer) == 0) break;
	}
}

/*
	Event handling methods
	Event data format = 5:::{"name":"event_name","args":[]}
*/

//Map an event name and its handler function
void SocketIOClient::setEventHandler(char* eventName,  void (*handler)(EthernetClient client, char* data)) {
	if(nbEvent < HASH_SIZE) eventHandlers[nbEvent++](eventName, handler);
	else Serial.println('Max number of events reached');
}

//Event data emitting method
void SocketIOClient::emit(char* event, char* data) {
	client.print((char)0);
	client.print("5:::{\"name\":\"");
	client.print(event);
	client.print("\",\"args\":[\"");
	client.print(data);
	client.print("\"]}");
	client.print((char)255);
}

/*
	Monitoring for incoming data
	https://github.com/automattic/socket.io-protocol
*/

void SocketIOClient::monitor(){
	*databuffer = 0;

	//If Ethernet client is disconnected, try to reconnect
	if(!client.connected()) {
		if(!client.connect(hostname, port)) return;
	}

	//Stop the method if no data
	if(!client.available()) return;

	char which;

	while(client.available()) {
		readLine();
		dataptr = databuffer;
		switch(databuffer[0]) {
			case '1': //connect: [1::]
				which = 6;
				break;

			case '2': //heartbeat: [2::]
				client.print((char)0);
				client.print("2::");
				client.print((char)255);
				continue;

			case '5': //event: [5:::{"name":"event_name","args":[]}]
				//Get the event name
				char* evtnm;
				evtnm = getName(databuffer);
				//Get the event handler function and call it
				void (*evhand)(EthernetClient client, char *data );
				if(eventHandlers.getFunction(evtnm , &evhand)) {
					evhand(client, databuffer);
				}
				//uhm...
				which = 4;
				break;

			default:
				Serial.print("Drop ");
				Serial.println(dataptr);
				continue;
		}

		findColon(which);
		dataptr += 2;

		//Handle backslash-delimited escapes
		char *optr = databuffer;
		while(*dataptr && (*dataptr != '"')) {
			if(*dataptr == '\\') {
				++dataptr; //TODO: this just handles "; handle \r, \n, \t, \xdd
			}
			*optr++ = *dataptr++;
		}
		*optr = 0;
	}
}

//Put incoming data's first line into the data buffer
void SocketIOClient::readLine() {
	//dataptr pointer points to the beginning of the buffer
	dataptr = databuffer;
	//Stop if there is no more data or if reading reaches buffer's max size
	while(client.available() && (dataptr < &databuffer[DATA_BUFFER_LEN - 2])) {
		char c = client.read();
		if(c == 0) {} //Serial.println(F("NULL"));
		else if(c == 255) {} //Serial.println(F("0x255"));
		else if(c == '\r') {}
		else if(c == '\n') break;
		else *dataptr++ = c;
	}
	*dataptr = 0;
}

//Find the nth colon starting from dataptr
void SocketIOClient::findColon(char which) {
	while(*dataptr) {
		if(*dataptr == ':') {
			if(--which <= 0) return;
		}
		++dataptr;
	}
}

//Get the name of the incoming data's event
char* SocketIOClient::getName(char* databuffer){
	char* nm = databuffer;
	char* longname;
	int quotes = 0;
	int beg = 0;
	int end = 0;

	while(*nm && quotes < 3) { 
		if(*nm  == '"') quotes++;
		beg++; nm++; 
	}	
	longname = nm;
	end = beg;
	while(*nm && (*nm != '"')) { end++; nm++; }

	char* name = new char[end - beg + 1];
	strncpy(name, longname, end - beg);
	name[end - beg] = '\0';

	return name;
}