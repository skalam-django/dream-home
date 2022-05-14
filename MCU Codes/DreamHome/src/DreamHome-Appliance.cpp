#include "DreamHome-Appliance.h"

DreamHomeAppliance::DreamHomeAppliance(Stream* _debugStream, bool writeCreds)
{
  EEPROM.begin(4096);
  enableDebug = _debugStream != NULL;
  debugStream = _debugStream;
  database = new Database(_debugStream);
  debug = true;
  if(!writeCreds){
    pinMap = new pinMapping();
    readCredentials();
    while(!connectWiFi(true)){
      database->ayncDelay(5000);
    }
  } 
  else{
    if(enableDebug){
      debugStream->println(F("DreamHomeAppliance[1] : Writing Mode."));
      debugStream->println(F("DreamHomeAppliance[2] : Clearing EEPROM."));
    }
      for (int j = 0; j < 4096; j++) {
        EEPROM.write(j, 0);
      }
      if(enableDebug)debugStream->println(F("DreamHomeAppliance[2] : Formatting LittleFS."));
      while (!LittleFS.begin()) {
        database->ayncDelay(300);
        if(enableDebug)debugStream->println("LittleFS mount failed. Formatting........");
      }      
      LittleFS.format();
      LittleFS.end();
      if(enableDebug)debugStream->println("LittleFS mount failed. Formatting Completed........");
    
  }
  if(enableDebug)debugStream->println(F("DreamHomeAppliance[1] : Ready."));
  debug = false;
}

DreamHomeAppliance::~DreamHomeAppliance() {
  EEPROM.end();
}


void DreamHomeAppliance::myLogger(String func_name, String text){
  if(debug && enableDebug){
    logger(func_name, text);
  }
}

void DreamHomeAppliance::setDebug(bool endebug){
  debug = endebug;
}

void DreamHomeAppliance::logger(String func_name, String text){
  database->logger(func_name, text);
}


int16_t DreamHomeAppliance::strIndex(const char* str, const char* findStr, uint16_t startIdx) {
  int16_t firstIndex = -1;
  int16_t sizeMatch = 0;
  for(int16_t i = startIdx; i < strlen(str); i++) {
    if(sizeMatch >= strlen(findStr)) {
      break;
    }
    if(str[i] == findStr[sizeMatch]) {
      if(firstIndex < 0) {
        firstIndex = i;
      }
      sizeMatch++;
    } else {
      firstIndex = -1;
      sizeMatch = 0;
    }
  }

  if(sizeMatch >= strlen(findStr)) {
    return firstIndex;
  } else {
    return -1;
  }
}


void DreamHomeAppliance::readCredentials(){
  myLogger(FPSTR(__func__), "Start");
  int l = EEPROM.read(0);
  if (l>100){
    return;
  }

  database->ayncDelay(30);
  int i = 0;
  for (int j=1; j<=l; j++){
    int idx = EEPROM.read(j);
    database->ayncDelay(30);
    String str = _getEEPROM(idx); 
    if(i==0){
      ssid =  str;
    }else if(i==1){
      pass =  str;
    }
    i++;
  }  
  myLogger(FPSTR(__func__), "End.");
}

void DreamHomeAppliance::writeCredentials(const String creds[], size_t creds_len){
  myLogger(FPSTR(__func__),"Start");
  uint16_t start_idx = CRED_IDX;
  const char* data;
  int data_len = 0;

  if (creds_len!=2){
    return;
  }
  
  for (int i=0; i<creds_len; i++){
    data = creds[i].c_str();
    data_len = strlen(data);
    if (strIndex(data, "Dream Home", 0)>=0){
      if(i==0){
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_STA);
        WiFiMulti.addAP(creds[i].c_str(), creds[i+1].c_str());
      }  
    }    
    _putEEPROM(start_idx, data);
    EEPROM.write(0, i+1);
    database->ayncDelay(30);
    EEPROM.write(i+1, start_idx);
    database->ayncDelay(30);
    start_idx += data_len + 1;
  }
  EEPROM.commit();
  myLogger(FPSTR(__func__), "Written succesfully.");
  readCredentials();     
  myLogger(FPSTR(__func__), "Reading --> SSID: " + ssid + ", PASS: " + pass + "\nEnd");
}

int16_t DreamHomeAppliance::getCredsEndAddr(){
  int16_t cred_address = 0;
  int howmany = EEPROM.read(0);
  int j = 0;  
  for (int i = 1; i < howmany; ++i){
    j = EEPROM.read(i);
    cred_address += j + EEPROM.read(j);
  }

  return cred_address+howmany;  
}

bool DreamHomeAppliance::getEEPROMPermission(uint16_t _addrOffset){
  if(_addrOffset>getCredsEndAddr()){
    return true;
  }
  myLogger(FPSTR(__func__), "This section of EEPROM is restricted.");
  return false;
}

uint16_t DreamHomeAppliance::putEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type, const char* value){
  myLogger(FPSTR(__func__), "Start");
  uint16_t addr= _addrOffset;
  if(!getEEPROMPermission(_addrOffset)){
    return -1;
  }
  switch(pool_type){
    case BLOCK:
      addr = _putEEPROM(_addrOffset, value);
      break;  
    case POINT:
      EEPROM.write(_addrOffset, (uint8_t)atoi(value));
      break;  
    default:
      addr = _putEEPROM(_addrOffset, value);
      break;  
  }
  EEPROM.commit();
  myLogger(FPSTR(__func__), "End");
  return addr;
}

uint16_t DreamHomeAppliance::_putEEPROM(uint16_t _addrOffset, const char* value){
  myLogger(FPSTR(__func__), "Start");
  uint16_t addr;
  byte len = strlen(value);
  EEPROM.write(_addrOffset, len);
  database->ayncDelay(30);
  for (int i = 0; i < len; i++)
  {
    addr = _addrOffset + 1 + i;
    EEPROM.write(addr, value[i]);
    database->ayncDelay(30);
  }
  myLogger(FPSTR(__func__), "End");
  return addr;
}

String DreamHomeAppliance::getEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type) {
  if(!getEEPROMPermission(_addrOffset)){
    return "";
  }
  String str_data;
  switch(pool_type){
    case BLOCK:
      return _getEEPROM(_addrOffset);
    case POINT:
      str_data = EEPROM.read(_addrOffset);
      database->ayncDelay(30);
      return str_data;        
    default:
      return _getEEPROM(_addrOffset);
  }
}

String DreamHomeAppliance::_getEEPROM(uint16_t _addrOffset) {
  int newStrLen = EEPROM.read(_addrOffset);
  database->ayncDelay(30);
  char* data = (char*) malloc(newStrLen+1);
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(_addrOffset + 1 + i);
    database->ayncDelay(30);    
  }
  data[newStrLen] = '\0';
  String str_data = data;
  free(data);
  return str_data;  
}


String DreamHomeAppliance::getWiFiSSID() {
  if (strIndex(ssid.c_str(), "Dream Home", 0)<0){
    myLogger(FPSTR(__func__), "SSID: "+ssid+"\nEnd");
    return "";
  }
  myLogger(FPSTR(__func__), "SSID: "+ssid+"\nEnd");
  return ssid;
}

String DreamHomeAppliance::getWiFiPASS() {
  if (strIndex(pass.c_str(), "Dream Home", 0)>=0){
    return "";
  }  
  return pass;
}

  
int DreamHomeAppliance::find(int a[], int size, int el){
  for(int i=0; i<(size-1); i++) {
    if(a[i]==el){
      return i;
    }
  }
  return 0;
}

void DreamHomeAppliance::sort(int a[], int size, bool dsc) {
  for(int i=0; i<(size-1); i++) {
    for(int o=0; o<(size-(i+1)); o++) {
      if((!dsc && (a[o] > a[o+1])) || (dsc && (a[o] < a[o+1]))){
        int t = a[o];
        a[o] = a[o+1];
        a[o+1] = t;
      }
    }
  }
}

bool DreamHomeAppliance::isWiFiConnected(bool retry){
  myLogger(FPSTR(__func__), "Start\nChecking WiFi connection...");
  bool connected = false;
  if (!retry){
    connected =  WiFiMulti.run() == WL_CONNECTED;
  }else{
    int i = 0;
    while(!connected && i<10){
        database->ayncDelay(500);
        if(enableDebug)debugStream->print(".");
        connected = WiFiMulti.run() == WL_CONNECTED;
        i++;
    }
  }
  myLogger(FPSTR(__func__), "Status: "+String(btoa(connected))+"\nEnd");
  return connected;
}

bool DreamHomeAppliance::connectWiFi(bool retry){
  myLogger(FPSTR(__func__), "Start");
  bool wifiConnected = false;
  String wifiSSID = "";
  String wifiPASS = "";
  WiFi.disconnect() ;
  WiFi.persistent(false);
  WiFi.setAutoConnect(true);
  wifiSSID =  getWiFiSSID();
  wifiPASS =  getWiFiPASS();
  if (wifiSSID.length()>0){
    WiFi.mode(WIFI_OFF);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    WiFi.setOutputPower(19.5);
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(wifiSSID.c_str(), wifiPASS.c_str());
    wifiConnected = isWiFiConnected(true);     
  }
  myLogger(FPSTR(__func__), "Connection Status: "+String(btoa(wifiConnected)));
  if (wifiConnected){
    myMAC = WiFi.macAddress();
    myLogger(FPSTR(__func__), "Connected to "+wifiSSID+"\nIP address: "+WiFi.localIP().toString()+"\nMAC: "+myMAC);
  }else if(retry){
    return connectWiFi(retry);
  }
  myLogger(FPSTR(__func__), "End");
  return wifiConnected;
}


void DreamHomeAppliance::setPorts(){
  myLogger(FPSTR(__func__), "Start\nSetting the ports..........."); 
  String filepath = "/"+String(DB_STRING[AP])+"/";
  JSONVar fileArr = database->listFiles(filepath.c_str());
  for(int j=0; j<fileArr.length(); j++) {
    String port = (const char*)fileArr[j];
    debugStream->print(port); 
    debugStream->print(" --> ");

    JSONVar jsonVar = database->getJSON(AP, "", port);

    if(JSON.typeof(jsonVar)=="undefined" || JSON.typeof(jsonVar[0])=="undefined"){
      myLogger(FPSTR(__func__), "End");
      return;
    }

    int category = (int)jsonVar[0];
    int is_active = (int)jsonVar[1];
    int manual = (int)jsonVar[2];
    int status = (int)jsonVar[3];
    myLogger(FPSTR(__func__), "category: " + String(category) + ", is_active: " + String(is_active) + ", manual: " + String(manual) + ", status: " + String(status));
    if(is_active)applianceLogic(category, port, status);
  }
  myLogger(FPSTR(__func__), "End");
}

void DreamHomeAppliance::applianceLogic(int category, String port, int status){
  myLogger(FPSTR(__func__), "Start");
  uint16_t app_pin;
  uint16_t* app_pin_arr;
  uint16_t app_pin_start;
  uint16_t app_pin_stop;
  bool prev_status;
  String status_str;

  switch(category){
    case PUMP_WITH_PANNEL:
      app_pin_arr = pinMap->NodeMCU_D1_Mini_Dual_Pins(port);
      app_pin_start = app_pin_arr[0];
      app_pin_stop =  app_pin_arr[1];
      free(app_pin_arr);
      pinMode(app_pin_start, OUTPUT);
      pinMode(app_pin_stop, OUTPUT);
      status_str = getEEPROM(PUMP_WITH_PANNEL_STATUS_IDX, POINT);
      prev_status = status_str.toInt();          
      myLogger(FPSTR(__func__), "app_pin_start: " + String((int)app_pin_start) + ",   app_pin_stop: " + String((int)app_pin_stop));
     
      if(prev_status!=status){
        if(status){
          digitalWrite(app_pin_stop, false);
          database->ayncDelay(20);
          digitalWrite(app_pin_start, true);
          database->ayncDelay(2000);
          digitalWrite(app_pin_start, false);
          database->ayncDelay(20);
        }else{
          digitalWrite(app_pin_start, false);
          database->ayncDelay(20);
          digitalWrite(app_pin_stop, true);
          database->ayncDelay(20);
          digitalWrite(app_pin_stop, false);
          database->ayncDelay(20);
        }
      }
      putEEPROM(PUMP_WITH_PANNEL_STATUS_IDX, POINT, String(status, DEC).c_str());
      break;
    case PUMP_WITHOUT_PANNEL:
      status_str = getEEPROM(PUMP_WITHOUT_PANNEL_STATUS_IDX, POINT);
      prev_status = status_str.toInt();          
      if(status!=prev_status){
        app_pin = pinMap->NodeMCU_D1_Mini_Single_Pin(port);
        pinMode(app_pin, OUTPUT);
        digitalWrite(app_pin, status);          
        putEEPROM(PUMP_WITH_PANNEL_STATUS_IDX, POINT, String(status, DEC).c_str());
      }
      break;
    default:
      break;  
  }
  myLogger(FPSTR(__func__), "End");
}

