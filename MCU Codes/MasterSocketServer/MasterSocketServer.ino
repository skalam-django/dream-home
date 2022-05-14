#define DEBUG
#define DISABLE_TLS
#define ASYNC_HTTP_DEBUG_PORT           Serial
// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _ASYNC_HTTP_LOGLEVEL_           3
#define CONFIG_ASYNC_TCP_USE_WDT 0 
#include <DreamHome-Master.h>
#include <Ticker.h>

WebSocketsServer wsOP = WebSocketsServer(80);
WebSocketsServer wsAP = WebSocketsServer(81);

DreamHomeMaster* dreamhome_master;
Ticker ticker1;
Ticker ticker2;
Ticker ticker3;


#define DEBUG false
#define ASYNC_ALIVE_TIMEOUT 10000
static uint32_t start_time;

const String baseUrl = "http://f10843f1c577.ngrok.io";

JSONVar macNum; 
JSONVar opHandshake;
JSONVar apHandshake;
String serverResponse;


void wsOPEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    IPAddress ip = wsOP.remoteIP(num);
    String opMac = dreamhome_master->getMACfromIP(ip);
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println(num);
            dreamhome_master->logger(FPSTR(__func__), "Start\nDisconnected with ip: " + ip.toString() +" opMac:"+ opMac);
            JSON.deleteKey(macNum, opMac.c_str());
            JSON.deleteKey(opHandshake, String((int)num).c_str());
            break;
        case WStype_CONNECTED:
            {
                dreamhome_master->logger(FPSTR(__func__), "Start\nConnected from ip:" + ip.toString() +" opMac:"+ opMac);
                opHandshake[String((int)num).c_str()] = false;
                macNum[opMac.c_str()] = String((int)num).c_str();
                broadcastToOperator(opMac, true);
                dreamhome_master->logger(FPSTR(__func__), "End");
            }
            break;
        case WStype_TEXT:{
            dreamhome_master->logger(FPSTR(__func__), "Start\nRecieved Text: "+String((char*)payload)+" from "+opMac);
            String response = (char*)payload;
            if(response.indexOf("OK", 0)==0){
              opHandshake[String((int)num).c_str()] = true;
              return;
            }
            
            JSONVar docSlave = JSON.parse(response);
            dreamhome_master->logger(FPSTR(__func__), "docSlave : " + String((const char*)docSlave));
            if (JSON.typeof(docSlave) == "undefined") {
              dreamhome_master->logger(FPSTR(__func__), F("Parsing input failed! key: <Whole>\nEnd"));
              return;
            }
            
            JSONVar opData = docSlave["op_data"];
            
            if (JSON.typeof(opData) == "undefined") {
              dreamhome_master->logger(FPSTR(__func__), F("Parsing input failed! key : op_data"));
            }
            
            JSONVar op_keys = opData.keys();
            bool done;     
            for (int i = 0; i < op_keys.length(); i++) {
              String opPort = JSON.stringify(op_keys[i]);
              opPort.replace("\"", "");
              String opPortValue = JSON.stringify(opData[op_keys[i]]);
              dreamhome_master->logger(FPSTR(__func__), "opPortValue: " + opPortValue);
              done = dreamhome_master->database->putValue(OP, opMac, opPort, opPortValue);
              if(done){
                JSONVar serverJsonVar = dreamhome_master->database->getJSON(SERVER_OP, opMac, opPort);
                if (JSON.typeof(serverJsonVar) != "undefined") {
                  String apMac = (const char*)serverJsonVar[6];
                  broadcastToAppliance(apMac, false, true);                 //it should be outside of port loop <-------------
                }
              }
            }
            
//            wsOP.sendTXT("OK");
            dreamhome_master->logger(FPSTR(__func__), "End");
          }
          break;
        case WStype_BIN:
            break;
    }

}


void wsAPEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    IPAddress ip = wsAP.remoteIP(num);
    String apMac = dreamhome_master->getMACfromIP(ip); 
    switch(type) {
        case WStype_DISCONNECTED:
            dreamhome_master->logger(FPSTR(__func__), "Start\nDisconnected with ip: " + ip.toString() +" apMac:"+ apMac);
            JSON.deleteKey(macNum, apMac.c_str());
            JSON.deleteKey(apHandshake, String((int)num).c_str());
            dreamhome_master->logger(FPSTR(__func__),"End");
            break;
        case WStype_CONNECTED:
            {
                dreamhome_master->logger(FPSTR(__func__), "Start\nConnected from ip: " + ip.toString() +" apMac:"+ apMac);
                apHandshake[String((int)num).c_str()] = false;
                macNum[apMac.c_str()] = String((int)num).c_str();
                broadcastToAppliance(apMac, true, true);
                dreamhome_master->logger(FPSTR(__func__), "End");
            }
            break;
        case WStype_TEXT:{
            dreamhome_master->logger(FPSTR(__func__), "Start\nRecieved Text: "+String((char*)payload)+" from "+apMac);
            String response = (char*)payload;
            if(response.indexOf("OK", 0)==0){
              apHandshake[String((int)num).c_str()] = true;
              return;
            }
//             wsAP.sendTXT("OK");
            dreamhome_master->logger(FPSTR(__func__), "End");
          }
            break;
        case WStype_BIN:
            break;
    }

}



void broadcastToOperator(String opMac, bool forced){
  dreamhome_master->logger(FPSTR(__func__), "Start");
  if(!macNum.hasOwnProperty(opMac.c_str())){
    dreamhome_master->logger(FPSTR(__func__), opMac + " not connected yet.\nEnd");
    return;
  }
  uint8_t opNum = (int)macNum[opMac.c_str()];
  bool handshake = (bool)opHandshake[String((int)opNum).c_str()];
  forced = forced || !handshake;
  dreamhome_master->logger(FPSTR(__func__), String(forced));  
  if(forced){
    String op_data = dreamhome_master->operatorDatabaseByMAC(opMac);
    dreamhome_master->logger(FPSTR(__func__), String(op_data.length()));  
    if(op_data.length()>0){
      op_data = "{\"op_data\":"+op_data+"}";
      dreamhome_master->logger(FPSTR(__func__),"Sending " + op_data + " to " +opMac);
      opHandshake[String((int)opNum).c_str()] = false;
      dreamhome_master->logger(FPSTR(__func__), "Broadcasting to : "+String((int)opNum));
      wsOP.sendTXT(opNum, op_data.c_str());
    }       
  }  
  dreamhome_master->logger(FPSTR(__func__), "End");
}



void broadcastToAppliance(String apMac, bool forced, bool retry){
  dreamhome_master->logger(FPSTR(__func__), "Start");
  if(!macNum.hasOwnProperty(apMac)){
    dreamhome_master->logger(FPSTR(__func__), apMac + " not connected yet.\nEnd");
    return;
  }
  uint8_t apNum = (int)macNum[apMac.c_str()];
  bool handshake = (bool)apHandshake[String((int)apNum).c_str()];
  bool forced1 = forced || !handshake;
  if(forced1){
    String ap_data = dreamhome_master->applianceDatabaseByMAC(apMac);
    if(ap_data.length()>0){
      ap_data = "{\"ap_data\":"+ap_data+"}";
      dreamhome_master->logger(FPSTR(__func__),"Sending " + ap_data + " to " +apMac);
      dreamhome_master->logger(FPSTR(__func__), "Broadcasting to : "+String((int)apNum));
      wsAP.sendTXT(apNum, ap_data.c_str());
      apHandshake[String((int)apNum).c_str()] = false;
    }else{
        broadcastToAppliance(apMac, forced, false);
    }    
  }  
  dreamhome_master->logger(FPSTR(__func__), "End");
}



//void broadcastToAppliances(){
//  dreamhome_master->logger(FPSTR(__func__), "Start");
//  String payload = dreamhome_master->applianceDatabase();
//  if(payload.length()>0){
//    payload = "{\"ap_data\":"+payload+"}"; 
//    dreamhome_master->logger(FPSTR(__func__), "Payload : "+payload);
//    wsAP.broadcastTXT(payload);
//  }
//  dreamhome_master->logger(FPSTR(__func__), "End");
//}




void isAsyncAlive(){
  if((millis()-start_time)>ASYNC_ALIVE_TIMEOUT){
    ayncHttprequest();
  }
}

void AyncResponseCallback(){
  dreamhome_master->logger(FPSTR(__func__), "Start");
  JSONVar docServer = JSON.parse(serverResponse);
  if (JSON.typeof(docServer) == "undefined") {
    dreamhome_master->logger(FPSTR(__func__), "Parsing input failed! key : <Whole>\nEnd");
  } 

  bool done;
    
  JSONVar apData = docServer["ap_data"];
  JSONVar opData = docServer["op_data"];
  JSONVar ap_keys = apData.keys();
  JSONVar op_keys = opData.keys();  
//  dreamhome_master->logger(FPSTR(__func__),"Opening SERVER_OP: ");
//  JSONVar prev_opValue = dreamhome_master->database->getJSON(SERVER_OP, "84:0D:8E:82:B6:94", "A0");
//  dreamhome_master->logger(FPSTR(__func__),"prev_opValue: ");
//  prev_opValue.printTo(Serial);
//  Serial.println();
  if (JSON.typeof(apData) == "undefined") {
    dreamhome_master->logger(FPSTR(__func__), "Parsing input failed! key : ap_data");
  }else{
    for (int i = 0; i < ap_keys.length(); i++) {
      String apMac = JSON.stringify(ap_keys[i]);
      apMac.replace("\"", "");
      JSONVar apMacValue = apData[ap_keys[i]];
      JSONVar ap_mac_keys = apMacValue.keys();
      bool force= false;
      for (int j = 0; j < ap_mac_keys.length(); j++) {
          String apPort = JSON.stringify(ap_mac_keys[j]);
          apPort.replace("\"", "");
          String apValue = JSON.stringify(apMacValue[ap_mac_keys[j]]);
          JSONVar prev_apValue = dreamhome_master->database->getJSON(AP, apMac, apPort);
          dreamhome_master->logger(FPSTR(__func__), apValue);
          done = dreamhome_master->database->putValue(AP, apMac, apPort, apValue);
          if(done){
            if(apValue!=prev_apValue)force = true;  
          }
          dreamhome_master->logger(FPSTR(__func__), done?"AP Database: Put Success":"AP Database: Put Failed");        
      }
      broadcastToAppliance(apMac,force, true);
    }
  }   
  if (JSON.typeof(opData) == "undefined") {
    dreamhome_master->logger(FPSTR(__func__),"Parsing input failed! key : op_data");
  }else{
    for (int i = 0; i < op_keys.length(); i++) {
      String opMac = JSON.stringify(op_keys[i]);
      opMac.replace("\"", "");
      JSONVar opMacValue = opData[op_keys[i]];
      JSONVar op_mac_keys = opMacValue.keys();
      bool force = false;
      for (int j = 0; j < op_mac_keys.length(); j++) {
          String opPort = JSON.stringify(op_mac_keys[j]);
          opPort.replace("\"", "");
          String opValue = JSON.stringify(opMacValue[op_mac_keys[j]]);
          JSONVar prev_opValue = dreamhome_master->database->getJSON(SERVER_OP, opMac, opPort);
          done = dreamhome_master->database->putValue(SERVER_OP, opMac, opPort, opValue);
          if(done){
            if(opValue!=prev_opValue)force = true;  
          }
          dreamhome_master->logger(FPSTR(__func__), done?"OP Database: Put Success":"OP Database: Put Failed"); 
      }
      broadcastToOperator(opMac, force);
    }
    
  }
    
}

void ayncResponse(void* optParm, asyncHTTPrequest* request, int readyState){
  dreamhome_master->logger(FPSTR(__func__), "Start");
  if(readyState == 4){
    serverResponse = request->responseText();
    if(serverResponse.length()<=0){
      dreamhome_master->logger(FPSTR(__func__), "No data\nEnd");
      return;
    }
    ticker3.once(1, AyncResponseCallback);
    request->setDebug(false);
  }
  ticker1.once(2, ayncHttprequest);
  dreamhome_master->logger(FPSTR(__func__), "End");
}


void ayncHttprequest(){
  dreamhome_master->logger(FPSTR(__func__), "Start");
  start_time = millis();
  String url = baseUrl + "/api/v1/device/";
  String payload = dreamhome_master->operatorDatabase();
  if(payload.length()<=0){
    payload = "{\"op_data\": null}";
  }else{
    payload = "{\"op_data\":" +payload+"}";
  }
  dreamhome_master->logger(FPSTR(__func__), "payload : "+ payload);
  dreamhome_master->ayncHttpPost(url.c_str(), payload.c_str());
  dreamhome_master->logger(FPSTR(__func__), "End");
}


//void simHttpPost(){
//  Serial.println(F("Start HTTP POST..."));
//  const char* recived;
//  String op_data = "";
//  serializeJson(dreamhome_master->operatorDatabase, op_data);
//  Serial.print(op_data);
////  sim800l->checkConnection()
//  if(true){    
//  char a_header[] = "Authorization:dh ";
//  char* auth_key = dreamhome_master->getAuthKey();
//  char headers[1+strlen(a_header)+strlen(auth_key)];
//  strcpy(headers, a_header);
//  strcat(headers, auth_key); 
//  uint16_t rc = dreamhome_master->doPost(URL, headers, "application/json", "{\"op_data\":"+op_data+"}", 20000, 20000);
//   if(rc == 200) {
//    // Success, output the data received on the serial
//    Serial.print(F("HTTP POST successful ("));
//    Serial.print(dreamhome_master->getDataSizeReceived());
//    Serial.println(F(" bytes)"));
//    Serial.print(F("Received : "));
//    recived = dreamhome_master->getDataReceived();
//    saveServerResponse(recived);
//    fail = 0;
//  } else {
//    // Failed...
//    Serial.print(F("HTTP POST error "));
//    Serial.println(rc);
//    Serial.println(URL);
//    if(rc>599){
//       fail++;
//    }else{
//      fail--;
//    }
//    if (fail>10){
//      dreamhome_master->setupSIM800L();
//    }
//  } 
//  }else if(!dreamhome_master->connectGPRS()){
//     dreamhome_master->setupSIM800L();
//  }
//}





void setup() {
//esp_task_wdt_init(20, true);
//esp_task_wdt_add(NULL);
  
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    while(!Serial);
    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }
    Serial.println("Welcome to Dream Home!");
    Serial.println("Setting up the System.");
    delay(1000);
    dreamhome_master = new DreamHomeMaster((Stream *)&Serial);
    dreamhome_master->setDebug(true);
    dreamhome_master->database->setDebug(DEBUG);
    dreamhome_master->ayncRequest.setDebug(DEBUG);
    dreamhome_master->ayncRequest.onReadyStateChange(ayncResponse);
    
    wsAP.begin();    
    wsAP.onEvent(wsAPEvent);
    wsAP.enableHeartbeat(15000, 3000, 2);
    
    wsOP.begin();    
    wsOP.onEvent(wsOPEvent);
    wsOP.enableHeartbeat(15000, 3000, 2);
   
    ticker2.attach(5, isAsyncAlive);
    start_time = millis();
    ayncHttprequest();
}

void loop() {
    wsAP.loop();
    wsOP.loop();
}
