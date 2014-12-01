/**

	This file mixes up SocketIOClient by Bill Roy and 
	part of HashMap by Alexander Brevig.

*/

#ifndef SocketIOClient_included
#define SocketIOClient_included

#include <Arduino.h>
#include <Ethernet.h>
#include <SPI.h>
#include <WProgram.h>

/*

	(Part of) HASHMAP Library by Alexander Brevig
	|| @file HashMap.h
	|| @version 1.0 Beta
	|| @author Alexander Brevig
	|| @contact alexanderbrevig@gmail.com

	@license
	|| | This library is free software; you can redistribute it and/or
	|| | modify it under the terms of the GNU Lesser General Public
	|| | License as published by the Free Software Foundation; version
	|| | 2.1 of the License.
	|| |
	|| | This library is distributed in the hope that it will be useful,
	|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
	|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	|| | Lesser General Public License for more details.
	|| |
	|| | You should have received a copy of the GNU Lesser General Public
	|| | License along with this library; if not, write to the Free Software
	|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
	|| #	
*/


/* Handle association */
template<typename hash,typename map>
class HashType {
	public:
		HashType(){ reset(); }
		HashType(hash code,map value):hashCode(code),mappedValue(value){}
		void reset(){ hashCode = 0; mappedValue = 0; }
		hash getHash(){ return hashCode; }
		void setHash(hash code){ hashCode = code; }
		map getValue(){ return mappedValue; }
		void setValue(map value){ mappedValue = value; }		
		HashType& operator()(hash code, map value){
			setHash( code );
			setValue( value );
		}
	private:
		hash hashCode;
		map mappedValue;
};

template<typename hash,typename map>
class HashMap {
	public:
		HashMap(HashType<hash,map>* newMap,int newSize){
			hashMap = newMap;
			size = newSize;
			for (int i=0; i<size; i++){
				hashMap[i].reset();
			}
		}
		
		HashType<hash,map>& operator[](int x){
			//TODO - bounds
			return hashMap[x];
		}

		/*
			Added by @dantaex
			Specially for SocketIOClient
			Returns true if specified key exists, and
			sends back the stored function pointer.
		*/
		bool getFunction(hash key, void (**handlerp)( EthernetClient client, char *data  ) ){
			for (int i=0; i<size; i++){
				if (	strcmp( (char*)hashMap[i].getHash(), (char*)key )	== 0	){
					*handlerp = hashMap[i].getValue();
					return true;
				}
			}
			return false;
		}

	private:
		HashType<hash,map>* hashMap;
		int size;
};



/*
	socket.io-arduino-client: a Socket.IO client for the Arduino

	Based on the Kevin Rohling WebSocketClient

	Copyright 2013 Bill Roy

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


// Length of static data buffers
#define DATA_BUFFER_LEN 120
#define SID_LEN 24

//define the max size of the hashtable
const byte HASH_SIZE = 20;

class SocketIOClient {
	public:
		typedef void (*DataArrivedDelegate)(SocketIOClient client, char *data);
		bool connect(char hostname[], int port = 80);
        bool connected();
        void disconnect();
		void monitor();
		void setDataArrivedDelegate(DataArrivedDelegate dataArrivedDelegate);
		void send(char *data);
		void sendEvent(char *event, char *data); //added
		void setEventHandler(char* eventName,  void (*handler)( EthernetClient client, char *data )); //added
		void init(int max_events);
	private:
        void sendHandshake(char hostname[], int port);
        EthernetClient client;
        DataArrivedDelegate dataArrivedDelegate;
        bool readHandshake();
		void readLine();
		char *dataptr;
		char databuffer[DATA_BUFFER_LEN];
		char sid[SID_LEN];
		char *hostname;
		int port;
		void findColon(char which);
		void terminateCommand(void);
		bool waitForInput(void);
		void eatHeader(void);
		char* getName(char* databuffer);
		static HashType<char*, void(*)( EthernetClient client, char *data )> hashRawArray[HASH_SIZE]; //added
		static HashMap< char*, void(*)( EthernetClient client, char *data )> eventHandlers; //added
		int eventc;
};

#endif