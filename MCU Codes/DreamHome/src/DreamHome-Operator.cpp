#include "DreamHome-Operator.h"

DreamHomeOperator::DreamHomeOperator(Stream* _debugStream, bool write_creds)
{
  EEPROM.begin(4096);
  enableDebug = _debugStream != NULL;
  debugStream = _debugStream;
  database = new Database(_debugStream);
  debug = true;
  if(!write_creds){
    pinMap = new pinMapping();
    readCredentials();
    while(!connectWiFi(true)){
      database->ayncDelay(5000);
    }
  } 
  else{
    if(enableDebug){
      myLogger(FPSTR(__func__), "DreamHomeOperator[1] : Writing Mode.\nClearing EEPROM");
      for (int j = 0; j < 4096; j++) {
        EEPROM.write(j, 0);
      }
      myLogger(FPSTR(__func__), "DreamHomeOperator[2] : Formatting LittleFS.");
      while (!LittleFS.begin()) {
        database->ayncDelay(300);
        myLogger(FPSTR(__func__), "LittleFS mount failed. Formatting........");
      }      
      LittleFS.format();
      myLogger(FPSTR(__func__), "Formatting Completed........");
    }
  }
  blinker1();
  myLogger(FPSTR(__func__), "DreamHomeOperator[1] : Ready.");
  debug = false;
}

DreamHomeOperator::~DreamHomeOperator() {
  EEPROM.end();
}

void DreamHomeOperator::myLogger(String func_name, String text){
  if(debug && enableDebug){
    logger(func_name, text);
  }
}

void DreamHomeOperator::setDebug(bool endebug){
  debug = endebug;
}


void DreamHomeOperator::logger(String func_name, String text){
  database->logger(func_name, text);
}



void DreamHomeOperator::OnOff(bool state){
  if(enableBlinker){
   digitalWrite(LED_BUILTIN, !state); 
  }
}

// void DreamHomeOperator::blinker1(){
//   ticker1.attach(0.02, []{OnOff(true);});  
//   ticker2.attach(0.04, []{OnOff(false);});  
// }


void DreamHomeOperator::blinker1(){
  ticker1.attach(0.02, [=](){OnOff(true);});  
  ticker2.attach(0.04, [=](){OnOff(false);});  
}


int16_t DreamHomeOperator::strIndex(const char* str, const char* findStr, uint16_t startIdx) {
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

void DreamHomeOperator::readCredentials(){
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

void DreamHomeOperator::writeCred(const String creds[], size_t creds_len){
  myLogger(FPSTR(__func__), "Start");
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


int16_t DreamHomeOperator::getCredsEndAddr(){
  int16_t cred_address = 0;
  int howmany = EEPROM.read(0);
  int j = 0;  
  for (int i = 1; i < howmany; ++i){
    j = EEPROM.read(i);
    cred_address += j + EEPROM.read(j);
  }

  return cred_address+howmany;  
}

bool DreamHomeOperator::getEEPROMPermission(uint16_t _addrOffset){
  if(_addrOffset>getCredsEndAddr()){
    return true;
  }
  myLogger(FPSTR(__func__), "This section of EEPROM is restricted.");
  return false;
}

uint16_t DreamHomeOperator::putEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type, const char* value){
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

uint16_t DreamHomeOperator::_putEEPROM(uint16_t _addrOffset, const char* value){
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

String DreamHomeOperator::getEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type) {
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

String DreamHomeOperator::_getEEPROM(uint16_t _addrOffset) {
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


String DreamHomeOperator::getWiFiSSID() {
  if (strIndex(ssid.c_str(), "Dream Home", 0)<0){
    myLogger(FPSTR(__func__), "SSID: "+ssid+"\nEnd");
    return "";
  }
  myLogger(FPSTR(__func__), "SSID: "+ssid+"\nEnd");
  return ssid;
}

String DreamHomeOperator::getWiFiPASS() {
  if (strIndex(pass.c_str(), "Dream Home", 0)>=0){
    return "";
  }  
  return pass;
}

  
int DreamHomeOperator::find(int a[], int size, int el){
  for(int i=0; i<(size-1); i++) {
    if(a[i]==el){
      return i;
    }
  }
  return 0;
}

void DreamHomeOperator::sort(int a[], int size, bool dsc) {
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


bool DreamHomeOperator::isWiFiConnected(bool retry){
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

bool DreamHomeOperator::connectWiFi(bool retry){
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



String DreamHomeOperator::operatorDatabase(){
  myLogger(FPSTR(__func__), "Start");
  JSONVar opObject;
  String filepath = "/"+String(DB_STRING[OP])+"/";
  debugStream->println(filepath);
  JSONVar fileArr = database->listFiles(filepath.c_str());
  debugStream->println(fileArr);
  for(int j=0; j<fileArr.length(); j++) {
    String port = (const char*)fileArr[j];
    JSONVar jsonVar = database->getJSON(OP, "", port);
    if(JSON.typeof(jsonVar)=="undefined" || JSON.typeof(jsonVar[0])=="undefined"){
      myLogger(FPSTR(__func__), "End");
      return JSON.stringify(opObject);
    }
    double sensor_value = (double)jsonVar[0];
    int status = (int)jsonVar[1];
    int calibration_ref1 = (int)jsonVar[2];
    int calibration_ref2 = (int)jsonVar[3];
    myLogger(FPSTR(__func__), "port: " + port + " --> sensor_value: " + String(sensor_value) + ", status: " + String(status)+", calibration_ref1: "+String(calibration_ref1)+", calibration_ref2: "+String(calibration_ref2));
    opObject[port][0] = sensor_value;
    opObject[port][1] = status;
    opObject[port][2] = calibration_ref1;
    opObject[port][3] = calibration_ref2;
  }
  myLogger(FPSTR(__func__), "End");
  return JSON.stringify(opObject);
}

bool DreamHomeOperator::updateSlaveOperatorDatabaseFromSensor(String opPort, double sensor_value, bool status){  //operator 
  myLogger(FPSTR(__func__), "Start");
  bool logic = false;
  JSONVar jsonVar = database->getJSON(OP, "", opPort);
  if(JSON.typeof(jsonVar)=="undefined" || JSON.typeof(jsonVar[0])=="undefined"){  
    jsonVar[0] = 0;
    jsonVar[1] = 0;
    jsonVar[2] = 0; 
    jsonVar[3] = 0; 
    logic = true;
  }
  if(sensor_value != (double)jsonVar[0])logic = true;
  jsonVar[0] = sensor_value;     
  if(int(status)!=(int)jsonVar[1])logic = true;
  jsonVar[1] = int(status);
  myLogger(FPSTR(__func__), "sensor_value: "+String(sensor_value)+", status: "+String(status));
  bool done = opDatabaseUpdate(jsonVar, opPort);
  myLogger(FPSTR(__func__), "logic: "+String(logic)+", Done: "+String(btoa(done))+"\nEnd");
  if(done) return logic;
  return false;
}



double DreamHomeOperator::readPorts(OPERATOR_CATEGORY op_cat, String opPort){   
  myLogger(FPSTR(__func__), "Start");
  double sensor_value = 0;
  switch(op_cat){
    case WATER_TANK:{
      uint16_t op_pin = pinMap->NodeMCU_D1_Mini_Single_Pin(opPort);
      sensor_value = analogRead(op_pin);
      sensor_value_arr[opPort][loop] = sensor_value;
      for(int i=MAX_LOOP-1; i>=0; i--){
        double avg_sensor_val = 0;
        int counter = 0;
        for(int j=i; j<MAX_LOOP; j++){
          JSONVar jsonVar = sensor_value_arr[opPort][j];
          if(JSON.typeof(jsonVar)!="undefined" && JSON.typeof(jsonVar)!="null" && JSON.typeof(jsonVar)!="nan"){
            avg_sensor_val += (double)jsonVar;
            counter++;
          }
          delay(0);
        }
        if(counter==0)counter=1;
        avg_sensor_val = double(avg_sensor_val/counter);
        sensor_value_arr[opPort][i] = avg_sensor_val;
        delay(0);
      }      
      sensor_value = 0;
      int counter = 0;
      for(int i=0; i<MAX_LOOP; i++){
        JSONVar jsonVar = sensor_value_arr[opPort][i];
        if(JSON.typeof(jsonVar)!="undefined" && JSON.typeof(jsonVar)!="null" && JSON.typeof(jsonVar)!="nan"){
          sensor_value += (double)jsonVar; 
          counter++;
        }
        delay(0);
      }
      if(counter==0)counter=1;
      sensor_value = double(sensor_value/counter);
    }  
  }
  loop++;
  if(loop>=MAX_LOOP){
    loop = 0;
  }
  myLogger(FPSTR(__func__), "sensor_value: "+ String(sensor_value) +"\nEnd");
  return sensor_value;
}


bool DreamHomeOperator::operatorLogic(String opPort){
  myLogger(FPSTR(__func__), "Start");
  JSONVar jsonVar =  database->getJSON(SERVER_OP, "", opPort);
  if(JSON.typeof(jsonVar)=="undefined" || JSON.typeof(jsonVar[0])=="undefined"){
    myLogger(FPSTR(__func__), "End");
    return false;
  }
  int category = (int)jsonVar[0];
  int is_active = (int)jsonVar[1];
  double sensor_max_value = (double)jsonVar[2];
  double sensor_min_value = (double)jsonVar[3];
  double capacity = (double)jsonVar[6];
  double sensor_value;
  bool status;
  switch(category){
    case WATER_TANK:
      double height = (double)jsonVar[7];
      double circumstances = (double)jsonVar[8];
      if(capacity<=0 || circumstances<=0){
        return false;
      }
      double tolerance = map(2*4*22*height/(7*capacity*pow(circumstances, 2)), 0, height, 0, 1024); //0.0061*2 = 0.0122*4*22/(7*0.8486)  2*(height/capacity)*4*22/(7*circumstances)
      String val = getEEPROM(WATER_SENSOR_PREV_VALUE_IDX, POINT);
      String status_str = getEEPROM(WATER_SENSOR_PREV_STATUS_IDX, POINT);
      double prev_value = map(val.toDouble(), 0, 255, 0, 1024); //strtod(val, NULL)
      bool prev_status = status_str.toInt();
      sensor_value = readPorts(WATER_TANK, opPort);
      int low_ref = getEEPROM(portToIdx(opPort, false), POINT).toInt();
      int high_ref = getEEPROM(portToIdx(opPort, true), POINT).toInt();
      low_ref = map(int(low_ref), 0, 255, 0, 1024);
      high_ref = map(int(high_ref), 0, 255, 0, 1024);
      sensor_value = map(int(sensor_value), low_ref, high_ref, 0, 1024);
      if(sensor_value>1024)
        sensor_value = 1024;
      else if(sensor_value<0)
        sensor_value   = 0;
      myLogger(FPSTR(__func__), "sensor_value: "+String(sensor_value)+", low_ref: "+String(low_ref)+", high_ref: "+String(high_ref));
      if(sensor_value-tolerance <= sensor_min_value){
        status = true;
      }else if(sensor_value > sensor_min_value && sensor_value < sensor_max_value){
        status = prev_status && abs(sensor_max_value - prev_value)>((12*capacity)/500);
      }else if(sensor_value+tolerance>=sensor_max_value){
        status = false;        
      }
      else{
        status = false;
      }
  }
  putEEPROM(WATER_SENSOR_PREV_VALUE_IDX, POINT, String(map(int(sensor_value), 0, 1024, 0, 255), DEC).c_str());
  putEEPROM(WATER_SENSOR_PREV_STATUS_IDX, POINT, String(status, DEC).c_str());
  bool logic =  updateSlaveOperatorDatabaseFromSensor(opPort, sensor_value, status);
  myLogger(FPSTR(__func__), "status: "+ String(btoa(status)) +", logic: " + String(btoa(logic)) + "\nEnd");
  return logic;
}


bool DreamHomeOperator::calibrate(int op_cat, String opPort, bool level){
  myLogger(FPSTR(__func__), "Start");
  enableBlinker = true;
  double calibrated_value=0;
  JSONVar tmp_sensor_value_arr;
  switch(op_cat){
    case WATER_TANK:{
      for(int k=0; k<3; k++){
        for(int l=0; l<MAX_LOOP; l++){
          uint16_t op_pin = pinMap->NodeMCU_D1_Mini_Single_Pin(opPort);
          double sensor_value = analogRead(op_pin);
          tmp_sensor_value_arr[opPort][l] = sensor_value;
          for(int i=MAX_LOOP-1; i>=0; i--){
            double avg_sensor_val = 0;
            int counter = 0;
            for(int j=i; j<MAX_LOOP; j++){
              JSONVar jsonVar = tmp_sensor_value_arr[opPort][j];
              if(JSON.typeof(jsonVar)!="undefined" && JSON.typeof(jsonVar)!="null" && JSON.typeof(jsonVar)!="nan"){
                avg_sensor_val += (double)jsonVar;
                counter++;
              }
              delay(0);
            }
            if(counter==0)counter=1;
            avg_sensor_val = double(avg_sensor_val/counter);
            tmp_sensor_value_arr[opPort][i] = avg_sensor_val;
          }      
          sensor_value = 0;
          int counter = 0;
          for(int i=0; i<MAX_LOOP; i++){
            JSONVar jsonVar = tmp_sensor_value_arr[opPort][i];
            if(JSON.typeof(jsonVar)!="undefined" && JSON.typeof(jsonVar)!="null" && JSON.typeof(jsonVar)!="nan"){
              sensor_value += (double)jsonVar; 
              counter++;
            }
            delay(0);
          }
          if(counter==0)counter=1;
          calibrated_value = double(sensor_value/counter);
          delay(10);
        }
        delay(1);
      }
    }
  }
  debugStream->println(calibrated_value);
  putEEPROM(portToIdx(opPort, level), POINT, String(map(int(calibrated_value), 0, 1024, 0, 255), DEC).c_str());

  JSONVar jsonVar = database->getJSON(OP, "", opPort);
  if(JSON.typeof(jsonVar)=="undefined"  || JSON.typeof(jsonVar[0])=="undefined"){  
    jsonVar[0] = 0;
    jsonVar[1] = 0;
    jsonVar[2] = 0; 
    jsonVar[3] = 0; 
  }
  if(level){
    jsonVar[2] = 1;     
  }else{
    jsonVar[3] = 1;
  }
  bool done = opDatabaseUpdate(jsonVar, opPort);
  
  enableBlinker = false;
  digitalWrite(LED_BUILTIN, HIGH);
  myLogger(FPSTR(__func__), "End");
  return done;
}


int DreamHomeOperator::portToIdx(String opPort, bool level){
  myLogger(FPSTR(__func__), "Start");
  String loc;
  const char* p = opPort.c_str();
  for (int i=1; i<3; i++){
    loc += p[i];
  }
  p = NULL;
  int idx = LOW_REF_START_IDX;
  if(level){
    idx = HIGH_REF_START_IDX;
  }
  idx += loc.toInt();
  myLogger(FPSTR(__func__), "idx: "+String(idx)+"\nEnd");
  return idx;
}


void DreamHomeOperator::calibrationDone(String opPort){
  JSONVar jsonVar = database->getJSON(OP, "", opPort);
  if(JSON.typeof(jsonVar)=="undefined"  || JSON.typeof(jsonVar[0])=="undefined"){  
    jsonVar[0] = 0;
    jsonVar[1] = 0;
  }
  jsonVar[2] = 0; 
  jsonVar[3] = 0; 
  opDatabaseUpdate(jsonVar, opPort);
}

bool DreamHomeOperator::opDatabaseUpdate(JSONVar jsonVar, String opPort){
  myLogger(FPSTR(__func__), "Start");
  debugStream->println();
  double sensor_value = 0;
  int status = 0;
  int calibration_ref1 = 0;
  int calibration_ref2 = 0;  
  if(JSON.typeof(jsonVar)!="undefined"){
    if(JSON.typeof(jsonVar[0])!="undefined"){
      sensor_value = (double)jsonVar[0];
    }
    if(JSON.typeof(jsonVar[1])!="undefined"){
      status = (int)jsonVar[1];
    }          
    if(JSON.typeof(jsonVar[2])!="undefined"){
      calibration_ref1 = (int)jsonVar[2];
    } 
    if(JSON.typeof(jsonVar[3])!="undefined"){
      calibration_ref2 = (int)jsonVar[3];
    } 
  }  
  myLogger(FPSTR(__func__), "sensor_value: "+String(sensor_value)+", status: "+String(status)+", calibration_ref1: "+String(calibration_ref1)+", calibration_ref2: "+String(calibration_ref2)+"\nEnd");
  return database->putValue(OP, "", opPort, "["+String(sensor_value)+","+String(status)+","+String(calibration_ref1)+","+String(calibration_ref2)+"]");
}