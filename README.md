# SocketIO Arduino Client, an Arduino client for connecting and messaging with Socket.io servers

Based on Kevin Rohling's arduino websocket client modified to work with socket.io servers.  Along the way, all uses of the String class were replaced with fixed char buffers so as to use less memory.

The bitlashsocketio.ino example provides an integration with Bitlash on the Arduino and a node.js example server that can be used to send Bitlash commands over the Websocket fabric to the Arduino, and see its output in reply.

Kevin's documentation is reproduced hereinafter, with changes as needed.


## Caveats

This library doesn't support every inch of the Websocket spec, most notably the use of a Sec-Websocket-Key.  Also, because Arduino doesn't support SSL, this library also doesn't support the use of Websockets over https.  If you're interested in learning more about the Websocket spec I recommend checking out the [Wikipedia Page](http://en.wikipedia.org/wiki/WebSocket).  Now that I've got that out of the way, I've been able to successfully use this to connect to several hosted Websocket services, including: [echo.websocket.org](http://websocket.org/echo.html) and [pusherapp.com](http://pusherapp.com).

## Installation instructions

Clone this repo into your Arduino Sketchbook directory under libraries, then restart the Arduino IDE so that it notices the new library.  Now, under File\Examples you should see SocketIOClient.  

## How To Use This Library

```
SocketIOClient client;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char hostname[] = "148.XXX.XX.XX";
int port = 3000;

// Socket.io "hello" EVENT handler
void hello(EthernetClient ethclient, char *data ){
    Serial.print("[hello] event hapenning:\t");
    Serial.println(data);
    client.sendEvent( "goodbye", "Client here, goodbye!" );
}

// Socket.io "goodbye" EVENT handler
void goodbye(EthernetClient ethclient, char *data ){
    Serial.print("[goodbye] event hapenning:\t");
    Serial.println(data);
    Serial.println("That is all.");
}

void setup() {
	Serial.begin(9600);
	Ethernet.begin(mac);
    client.init(5);
       
    if (!client.connect(hostname, port)) 
        Serial.println(F("Not connected."));

    //Event hanlders
    client.setEventHandler("hello",   hello);
    client.setEventHandler("goodbye", goodbye);

    //say hello to server
	if (client.connected())
        client.sendEvent("hello","Client here, hello!");
}

void loop() {
  client.monitor();
}

```

## Examples

There are some examples: EventsExample will show you how to use events, and EchoExample, will connect to echo.websocket.org, which hosts a service that simply echos any messages that you send it via Websocket.  This example sends the message "Hello World!".  If the example runs correctly, the Arduino will receive this same message back over the Websocket and print it via Serial.println.