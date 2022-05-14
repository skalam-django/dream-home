#include <DreamHome-Appliance.h>
#include <Ticker.h>
WebSocketsClient webSocket;

DreamHomeAppliance* dreamhome_appliance;

Ticker ticker1;

bool apHandshake = false;

void setApplianceDatabase(String response){
    Serial.println();
    Serial.print("setApplianceDatabase response: ");
    Serial.println(response);
    if(response.length()<=0){
      return;
    }
    JSONVar docMaster = JSON.parse(response);
    if (JSON.typeof(docMaster) == "undefined") {
      Serial.println("Parsing input failed! key : <Whole>");
      return;
    }
    
    JSONVar apData = docMaster["ap_data"];
    if (JSON.typeof(apData) == "undefined") {
      Serial.println("Parsing input failed! key : ap_data.");
      return;
    }
    
    JSONVar ap_keys = apData.keys();
    bool done;
    for (int i = 0; i < ap_keys.length(); i++) {
      String apMac = JSON.stringify(ap_keys[i]);
      apMac.replace("\"", "");      
      Serial.print("myMAC --> ");
      Serial.println(dreamhome_appliance->myMAC);
      if(dreamhome_appliance->myMAC==apMac){
        JSONVar apMacValue = apData[ap_keys[i]];
        JSONVar ap_mac_keys = apMacValue.keys();
        for (int j = 0; j < ap_mac_keys.length(); j++) {
          String apPort = JSON.stringify(ap_mac_keys[j]);
          apPort.replace("\"", "");         
          String apValue = JSON.stringify(apMacValue[ap_mac_keys[j]]);
          //apValue.replace("\"", "");
          Serial.print("apValue:   ");
          Serial.println(apValue);
          done = dreamhome_appliance->database->putValue(AP, "", apPort, apValue);
          Serial.print("done: ");
          Serial.println(done);
          Serial.print("AP read_data: ");
          Serial.println(dreamhome_appliance->database->getJSON(AP, "", apPort));
      
        }
        
      }          
    }
    updatePorts();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED: {
			Serial.printf("[WSc] Connected to url: %s\n", payload);
     apHandshake = false;
		}
			break;
		case WStype_TEXT:{
        Serial.printf("[WSc] get text: %s\n", payload);
        String response = (char*)payload;
        if(response.indexOf("OK", 0)==0){
          apHandshake = true;
          return;
        }  
        setApplianceDatabase(response);
        webSocket.sendTXT("OK");    
		  }
		  break;
		case WStype_BIN:
			Serial.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
      case WStype_PING:
          // pong will be send automatically
          Serial.printf("[WSc] get ping\n");
          break;
      case WStype_PONG:
          // answer to a ping we send
          Serial.printf("[WSc] get pong\n");
          break;
    }

}

void updatePorts(){
  Serial.println("Updating the ports");
  dreamhome_appliance->setPorts();
}


void setup() {
	Serial.begin(115200);
	Serial.setDebugOutput(true);

	Serial.println();
	Serial.println();
	Serial.println();

	for(uint8_t t = 4; t > 0; t--) {
		Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
		Serial.flush();
		delay(1);
	}
  delay(3);
  dreamhome_appliance = new DreamHomeAppliance((Stream *)&Serial); //, (Stream *)&Serial);
  
	webSocket.begin("192.168.4.1", 81, "/");
	webSocket.onEvent(webSocketEvent);
	webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
}

void loop() {
	webSocket.loop();
}
