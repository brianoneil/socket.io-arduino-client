#include "Ethernet.h"
#include "SPI.h"
#include "SocketIOClient.h"

SocketIOClient client;


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "148.228.24.25";
int port = 3000;

// websocket message handler: do something with MESSAGE from server
void ondata(SocketIOClient client, char *data) {
    Serial.print("Message arrived:\t");
    Serial.println(data);
}

// websocket "hello" EVENT handler
void hello(EthernetClient ethclient, char *data ){
    Serial.print("[hello] event hapenning:\t");
    Serial.println(data);
    client.sendEvent( "goodbye", "Client here, goodbye!" );
}

// websocket "goodbye" EVENT handler
void goodbye(EthernetClient ethclient, char *data ){
    Serial.print("[goodbye] event hapenning:\t");
    Serial.println(data);
    Serial.println("That is all.");
}


void setup() {
	Serial.begin(9600);
	Ethernet.begin(mac);
    client.init(5);
        
        
        //Conectar con servidor        
	if (!client.connect(hostname, port)) 
        Serial.println(F("Not connected."));

    //MESSAGE hanlder
    client.setDataArrivedDelegate(ondata);

    //Event hanlders
    client.setEventHandler("hello",   hello);
    client.setEventHandler("goodbye", goodbye);

    //say hello to server
	if (client.connected())
        client.sendEvent("hello","Client here, hello!");

}

#define HELLO_INTERVAL 9000UL
unsigned long lasthello;

void loop() {
	client.monitor();
}