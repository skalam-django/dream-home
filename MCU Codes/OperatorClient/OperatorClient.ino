#include <DreamHome-Operator.h>

#define DEBUG true

DreamHomeOperator* dreamhome_operator;

WebSocketsClient webSocket;

Ticker ticker1;
Ticker ticker2;
Ticker ticker3;
Ticker ticker4;


bool opHandshake = false;
bool last_opHandshake = false;
bool enable=false;
int count=0;
bool calibration_lock = false;
bool calibration_done = false;

void myLogger(String func_name, String text){
  if(DEBUG){
    dreamhome_operator->logger(func_name, text);
  }
}

void operatorLogic(){   
  myLogger(FPSTR(__func__), "Start");
  bool handshake = !dreamhome_operator->operatorLogic("A0");
  if(handshake){
    opHandshake=true;
  }else if(last_opHandshake){
    opHandshake=false;
    last_opHandshake = false;
  }else{
    opHandshake=false;
  }
  
  myLogger(FPSTR(__func__), "End");
}


void sendOperatorDatabase(){
  myLogger(FPSTR(__func__), "Start");
  if(opHandshake && !calibration_done){
    if(!last_opHandshake){
      last_opHandshake = true;
    }else{
      return;
    }
  }else if(calibration_done){
    calibration_done = false;
  }
  String op_data = dreamhome_operator->operatorDatabase();
  if(op_data.length()>0){
    op_data = "{\"op_data\": "+op_data+"}";
    myLogger(FPSTR(__func__), op_data);
    webSocket.sendTXT(op_data);
    opHandshake = false;
  }    
  myLogger(FPSTR(__func__), "End");
}


void OnOff(bool state){
  if(enable){
   digitalWrite(LED_BUILTIN, !state); 
  }
}

void blinker1(){
  enable = true;
  ticker1.attach(1, []{OnOff(true);});  
  ticker2.attach(2, []{OnOff(false);});  
}


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:{
        myLogger(FPSTR(__func__), "Disconnected");
        enable = true;
      }
      break;
    case WStype_CONNECTED: {
    
      myLogger(FPSTR(__func__), "Start\nConnected to url: "+ String((char*)payload));
      enable = false;
      digitalWrite(LED_BUILTIN, HIGH); 
      opHandshake = false;
      sendOperatorDatabase();
      myLogger(FPSTR(__func__), "End");
    }
      break;
    case WStype_TEXT:{
        myLogger(FPSTR(__func__), "Start\nget text: "+String((char*)payload));
        String response = (char*)payload;
        if(response.indexOf("OK", 0)==0){
          opHandshake = true;
          myLogger(FPSTR(__func__), "handshake : OK\nEnd");
          return;
        }    
        setOperatorDatabase(response);
        webSocket.sendTXT("OK");
        myLogger(FPSTR(__func__), "End");
      }
      break;
    case WStype_BIN:
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
      case WStype_PING:
          myLogger(FPSTR(__func__), "<PING PONG>");
          break;
      case WStype_PONG:
          myLogger(FPSTR(__func__), "<PONG PING>");
          break;
    }
}


void setOperatorDatabase(String response){   
  myLogger(FPSTR(__func__), "Start\nresponse: "+response);
//  
  if(response.length()<=0){
    return;
  }
  JSONVar docMaster = JSON.parse(response);
  
  if (JSON.typeof(docMaster) == "undefined" || JSON.typeof(docMaster) == "null") {
    myLogger(FPSTR(__func__),"Parsing input failed!");
    return;
  }
  
  JSONVar opData = docMaster["op_data"];
  if (JSON.typeof(opData) == "undefined" || JSON.typeof(opData) == "null") {
    myLogger(FPSTR(__func__),"Parsing input failed! key : op_data.");
    return;
  }
  
  JSONVar op_keys = opData.keys();
  bool done;     
  for (int i = 0; i < op_keys.length(); i++) {
    String opMac = JSON.stringify(op_keys[i]);
    opMac.replace("\"", "");
    myLogger(FPSTR(__func__),"myMAC --> "+ dreamhome_operator->myMAC);
    if(dreamhome_operator->myMAC==opMac){
      JSONVar opMacValue = opData[op_keys[i]];
      JSONVar op_mac_keys = opMacValue.keys();
      for (int j = 0; j < op_mac_keys.length(); j++) {
        String opPort = JSON.stringify(op_mac_keys[j]);
        opPort.replace("\"", "");   
        JSONVar jsonVar = opMacValue[op_mac_keys[j]];
        if(JSON.typeof(jsonVar)!="undefined" && JSON.typeof(jsonVar)!="null"){
          String opPortValue = JSON.stringify(jsonVar);
          myLogger(FPSTR(__func__), "opPortValue: "+opPortValue);
          done = dreamhome_operator->database->putValue(SERVER_OP, "", opPort, opPortValue);
          myLogger(FPSTR(__func__), "done: "+String(btoa(done)));          
          if((int)jsonVar[4]==1 && (int)jsonVar[1]==1){
            if(!calibration_lock){
              calibration_lock = true;
              calibration_done = dreamhome_operator->calibrate((int)jsonVar[0], opPort, true);  
              sendOperatorDatabase();              
            }
          }else if((int)jsonVar[5]==1 && (int)jsonVar[1]==1){
            if(!calibration_lock){
              calibration_lock = true;
              calibration_done = dreamhome_operator->calibrate((int)jsonVar[0], opPort, false);
              sendOperatorDatabase();               
            }
          }else{
            if(calibration_lock){
              dreamhome_operator->calibrationDone(opPort);
              calibration_lock = false; 
            }
          }
          
        }
      }
    }      
  }  
  myLogger(FPSTR(__func__), "End");
}


void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(DEBUG);

  while(!Serial);
    
  Serial.println();
  Serial.println();
  Serial.println();

  for(uint8_t t = 4; t > 0; t--) {
      Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
      Serial.flush();
      delay(1);
  }
  delay(3);
  Serial.println("Welcome to Dream Home!");
  Serial.println("Setting up the System.");
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
//  blinker1();
  dreamhome_operator = new DreamHomeOperator((Stream *)&Serial);
  dreamhome_operator->setDebug(DEBUG);
  dreamhome_operator->database->setDebug(DEBUG);
  webSocket.begin("192.168.4.1", 80, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(2000);
  webSocket.enableHeartbeat(15000, 6000, 2);
  ticker3.attach(1, operatorLogic); 
  ticker4.attach(5, sendOperatorDatabase); 
}
void loop() {
  webSocket.loop();
}
