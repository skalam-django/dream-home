#include "DreamHome-Master.h"

// ESP.getFreeHeap() < HEAP_SAFELIMIT

DreamHomeMaster::DreamHomeMaster(Stream* _debugStream, bool writeCreds){
  EEPROM.begin(4096);
  enableDebug = _debugStream != NULL;
  debugStream = _debugStream;
  database = new Database(_debugStream);
  debug = true;
  if(!writeCreds){
    initSIM800l();
    // setupSIM800L();
    readCredentials();
    startHotspot();
    connectWiFi(true);    
  }else{
    database = new Database(_debugStream);
    myLogger(FPSTR(__func__),"Writing Mode.\nClearing EEPROM");
    for (int j = 0; j < 4096; j++) {
      EEPROM.write(j, 0);
    }
    
    myLogger(FPSTR(__func__), "DreamHomeMaster[2] : Formatting LittleFS.");
    while (!LittleFS.begin()) {
      myLogger(FPSTR(__func__), "LittleFS mount failed. Formatting........");
    }      
    LittleFS.format();
    LittleFS.end();
    myLogger(FPSTR(__func__), "LittleFS mount failed. Formatting Completed........");
  }
  myLogger(FPSTR(__func__), "DreamHomeMaster[1] : Ready.");
  debug = false;
}


DreamHomeMaster::~DreamHomeMaster() {
  EEPROM.end();
}


void DreamHomeMaster::myLogger(String func_name, String text){
  if(debug && enableDebug){
    logger(func_name, text);
  }
}

void DreamHomeMaster::setDebug(bool endebug){
  debug = endebug;
}

void DreamHomeMaster::logger(String func_name, String text){
  database->logger(func_name, text);
}


int16_t DreamHomeMaster::strIndex(const char* str, const char* findStr, uint16_t startIdx) {
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

void DreamHomeMaster::readCredentials(){
  myLogger(FPSTR(__func__), "DreamHomeMaster[2] : Preparing credentials from EEPROM");
  int l = EEPROM.read(0);

  database->ayncDelay(30);
  if (l>100){
    return;
  }
  int i = 0;
  for (int j=1; j<=l; j++){
    int idx = EEPROM.read(j);
    database->ayncDelay(30);
    String str = _getEEPROM(idx); 
    if(i==0){
      token_key =  str;
    }else if(i==1){
      ssid =  str;
    }else if(i==2){
      pass =  str;
    }
    i++;
  }  
  
  myLogger(FPSTR(__func__), "Read succesfully.");
}

void DreamHomeMaster::writeCredentials(const String creds[], size_t creds_len){
  myLogger(FPSTR(__func__), "DreamHomeMaster[3] : Preparing credentials to EEPROM");
  uint16_t start_idx = CRED_IDX;
  const char* data;
  int data_len = 0;
  if (creds_len!=3){
    return;
  }
  for (int i=0; i<creds_len; i++){
    data = creds[i].c_str();
    data_len = strlen(data);
    if(enableDebug){
    //   debugStream->print("Data at ");
    //   debugStream->print(i);
    //   debugStream->print(" : ");
    //   debugStream->println(data);      
    }

    if (strIndex(data, "Dream Home", 0)>=0){
      if (i==1){
        IPAddress local_IP(192,168,4,1);
        IPAddress gateway(192,168,4,9);
        IPAddress subnet(255,255,255,0);
        WiFi.softAPConfig(local_IP, gateway, subnet);
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(creds[i].c_str(), creds[i+1].c_str(), AP_CHANNEL, AP_HIDDEN, AP_MAX_CON);
      }else{
        for (int j = 0; j < 4096; j++) {
          EEPROM.write(j, 0);
        }        
        return;
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
  myLogger(FPSTR(__func__), "Written succesfully.\nReading:");
  readCredentials();
  myLogger(FPSTR(__func__), "token_key: "+token_key+", ssid: "+ssid+", pass: "+pass);        
}


int16_t DreamHomeMaster::getCredsEndAddr(){
  int16_t cred_address = 0;
  int howmany = EEPROM.read(0);
  int j = 0;  
  for (int i = 1; i < howmany; ++i){
    j = EEPROM.read(i);
    cred_address += j + EEPROM.read(j);
  }

  return cred_address+howmany;  
}

bool DreamHomeMaster::getEEPROMPermission(uint16_t _addrOffset){
  if(_addrOffset>getCredsEndAddr()){
    return true;
  }
  myLogger(FPSTR(__func__), "This section of EEPROM is restricted.");
  return false;
}

uint16_t DreamHomeMaster::putEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type, const char* value){
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

uint16_t DreamHomeMaster::_putEEPROM(uint16_t _addrOffset, const char* value){
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

String DreamHomeMaster::getEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type) {
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

String DreamHomeMaster::_getEEPROM(uint16_t _addrOffset) {
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


String DreamHomeMaster::getWiFiSSID() {
  if (strIndex(ssid.c_str(), "Dream Home", 0)<0){
    myLogger(FPSTR(__func__), "SSID: "+ssid+"\nEnd");
    return "";
  }
  myLogger(FPSTR(__func__), "SSID: "+ssid+"\nEnd");
  return ssid;
}

String DreamHomeMaster::getWiFiPASS() {
  if (strIndex(pass.c_str(), "Dream Home", 0)>=0){
    return "";
  }  
  return pass;
}


String DreamHomeMaster::getAuthKey() {
  myLogger(FPSTR(__func__), "Start");
  if(protocol==WIFI){
    if(myMAC==""){
      myLogger(FPSTR(__func__), "MAC is not set. Please run restart the device.\nEnd");
      return "";    
    }
  }
  else if(protocol==SIM){
    if(imei==""){
      if(sim800lReady()){
        myLogger(FPSTR(__func__), "IMEI is not set. Please run DreamHomeMaster->setupSIM800L() before accessing this functionality.\nEnd");
        return "";
      }
      myLogger(FPSTR(__func__), "IMEI is not set. Please attach SIM800L before accessing this functionality.\nEnd");
      return "";      
    }
  }
  String str;
  if(protocol==WIFI){
    str = myMAC;
  }else if(protocol==SIM){
    str = imei;
  }
  String auth_key;
  if(str.length()>0 && token_key.length()>0){
    auth_key = base64::encode(str + "|" + token_key);
  }
  myLogger(FPSTR(__func__), "End");
  return auth_key;
}



void DreamHomeMaster::saveMasterWiFiCreds(){
  myLogger(FPSTR(__func__), "Start");
  const char* wifiSSID = wifiSSIDbuff[0].c_str();
  const char* wifiPASS = wifiPASSbuff[0].c_str();  
  uint16_t l;
  uint16_t l1;
  if (wifiSSID!=NULL && wifiSSID!="" && strlen(wifiSSID)>0){
    int16_t addr = getCredsEndAddr() + 1;
    l = putEEPROM(addr, BLOCK, "Y") + 1;
    EEPROM.write(l, 2);
    database->ayncDelay(30);
    l += 1;
    EEPROM.write(l, l + 2);
    database->ayncDelay(30);
    l1 = putEEPROM(l + 2, BLOCK, wifiSSID) + 1;
    EEPROM.write(l+1, l1);
    database->ayncDelay(30);
    putEEPROM(l1, BLOCK, wifiPASS);
    EEPROM.commit();
  }
  myLogger(FPSTR(__func__), "End");
}

void DreamHomeMaster::retriveMasterWiFiCreds(){
  myLogger(FPSTR(__func__), "Start");
  int16_t addr = getCredsEndAddr() + 1;
  String yes = getEEPROM(addr, BLOCK);
  if (yes=="Y"){
    int16_t l =  EEPROM.read(addr+1);
    database->ayncDelay(30);
    for(int i=0; i< l; i++){
      int16_t l1 =  EEPROM.read(addr+2+i);
      String val = getEEPROM(l1, BLOCK);
      if(i==0){
        wifiSSIDbuff[0] = val;
      }else if(i==1){
        wifiPASSbuff[0] = val;
      }
    }
  }
  myLogger(FPSTR(__func__), "End");
}


// char* DreamHomeMaster::_formatMAC(const unsigned char* mac) {
//   char buf[18];
//   snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//   return buf;
// }


String DreamHomeMaster::getMACfromIP(IPAddress ip){
  myLogger(FPSTR(__func__), "Start");
  wifi_sta_list_t wifi_sta_list;
  tcpip_adapter_sta_list_t adapter_sta_list;
  esp_wifi_ap_get_sta_list(&wifi_sta_list);
  tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);
  String station_mac;
  for(int i = 0; i < adapter_sta_list.num; i++) {
    tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
    char* address = ip4addr_ntoa(&(station.ip));
    if(String(address)==ip.toString()){
      for(int j = 0; j< 6; j++){
        char str[3];
        sprintf(str, "%02X", (int)station.mac[j]);
        station_mac += str;
        if(j<5){
          station_mac += ":";
        }
      }
    }
  }
  myLogger(FPSTR(__func__), "End");
  return station_mac;
}

void DreamHomeMaster::startHotspot(){
  myLogger(FPSTR(__func__), "Start");
  IPAddress local_IP(192,168,4,1);
  IPAddress gateway(192,168,4,9);
  IPAddress subnet(255,255,255,0);

  WiFi.softAPConfig(local_IP, gateway, subnet);

  bool configured = false;

  WiFi.disconnect();
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.mode(WIFI_AP_STA);
  while(!configured){
    if(enableDebug){
    //   debugStream->print("SSID: ");
    //   debugStream->println(getWiFiSSID());
    }
    database->ayncDelay(300);
    WiFi.softAP(getWiFiSSID().c_str(), getWiFiPASS().c_str(), AP_CHANNEL, AP_HIDDEN, AP_MAX_CON);
    IPAddress IP = WiFi.softAPIP();
    if (IP!=NULL){
      configured = true;
      myMAC = WiFi.softAPmacAddress();
      myLogger(FPSTR(__func__), "AP configured.\nAP IP address: "+IP.toString()+"\nMAC: "+myMAC+"\nEnd");
    }
  }
}

  
int DreamHomeMaster::find(int a[], int size, int el){
  for(int i=0; i<(size-1); i++) {
    if(a[i]==el){
      return i;
    }
  }
  return 0;
}

void DreamHomeMaster::sort(int a[], int size, bool dsc) {
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

bool DreamHomeMaster::scanWiFiConnect(){
  myLogger(FPSTR(__func__), "Start");
  WiFi.disconnect();
  uint8_t n = WiFi.scanNetworks();
  int32_t rssi[n];
  int idx[n]; 
  int j = 0;
  for(int i=0; i<n; i++){
  //   debugStream->print(".");
    if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN){
      rssi[j] = WiFi.RSSI(i);
      idx[j] = i;
      j++;
    }
  }
  int32_t* rssi1 = rssi;
  int s = sizeof(rssi) / sizeof(rssi[0]); 
  sort(rssi1, s, true);
  int i = find(rssi, s, rssi1[0]);
  wifiSSIDbuff[0] = WiFi.SSID(idx[i]);
  wifiPASSbuff[0] = "alamckt123";
  myLogger(FPSTR(__func__), "Available Open WiFi: "+wifiSSIDbuff[0]);
  connectWiFi(false);
  myLogger(FPSTR(__func__), "End");
}


bool DreamHomeMaster::isWiFiConnected(bool retry){
  myLogger(FPSTR(__func__), "Start\nChecking WiFi connection...");
  if (!retry){
    return WiFi.status() == WL_CONNECTED;
  }else{
    int i = 0;
    bool status=false;
    while(!status && i<10){
        database->ayncDelay(1000);
        if(enableDebug && debug)debugStream->print(".");
        status = WiFi.status() == WL_CONNECTED;
        i++;
    }
    myLogger(FPSTR(__func__), "Status: "+String(btoa(status))+"\nEnd");
    return status;
  }
}

bool DreamHomeMaster::connectWiFi(bool retry){
  myLogger(FPSTR(__func__), "Start");
  bool wifiConnected = false;
  String wifiSSID = "";
  String wifiPASS = "";


  WiFi.disconnect() ;
  WiFi.persistent(false);
  WiFi.setAutoConnect(true);

  for(int i=0; i<10 && !wifiConnected; i++){
    wifiSSID = wifiSSIDbuff[i];
    wifiPASS = wifiPASSbuff[i];
    if (wifiSSID.length()>0){
      WiFi.mode(WIFI_OFF);
      WiFi.mode(WIFI_AP_STA);
      WiFi.begin(wifiSSID.c_str(), wifiPASS.c_str(), WIFI_CHANNEL);
      database->ayncDelay(1000);
      wifiConnected = isWiFiConnected(true);       
    }
  }
  myLogger(FPSTR(__func__), "Connection Status: "+ String(btoa(wifiConnected)));
  if (wifiConnected){
    myLogger(FPSTR(__func__), "Connected to "+wifiSSID+"\nIP address: "+WiFi.localIP()+"\nMAC: "+myMAC);
  }else if(retry==true){
    wifiConnected = scanWiFiConnect();
  }
  myLogger(FPSTR(__func__), "End");
  return wifiConnected;
}

bool DreamHomeMaster::connectInternet(){
  myLogger(FPSTR(__func__), "Start");
  if(isWiFiConnected(true)){
    protocol = WIFI;
    myLogger(FPSTR(__func__), "Selected auth protocol: WIFI\nEnd");
    return true;
  }  
  else if(connectGPRS()){
    protocol = SIM;
    myLogger(FPSTR(__func__), "Selected Auth protocol: SIM\nEnd");
    return true;
  }
  myLogger(FPSTR(__func__), "NO Auth protocol Selected!\nEnd");
  return false;
}


void DreamHomeMaster::ayncHttpPost(const char* URL){
  ayncHttpPost(URL, NULL);
}

void DreamHomeMaster::ayncHttpPost(const char* URL, const char* body){
  myLogger(FPSTR(__func__), "Start\n[Aync HTTP] POST...");
  if(initAyncHTTP("POST", URL, true)){
    if(body!=NULL){
      ayncRequest.send(body);
      myLogger(FPSTR(__func__), "Success1\nEnd");
    }else{
      ayncRequest.send();
      myLogger(FPSTR(__func__), "Success2\nEnd");
    }
  }else{
    myLogger(FPSTR(__func__), "Fail\nEnd");
  }
}


bool DreamHomeMaster::initAyncHTTP(const char* method, const char* URL, bool retry){
  myLogger(FPSTR(__func__), "Start\n[Aync HTTP] begin...");
  httpResponseSize = 0;
  httpResponseOK = false;
  if (connectInternet()) {
    myLogger(FPSTR(__func__), "readyState: "+String(ayncRequest.readyState()));
    ayncRequest.setTimeout(5);
    if(ayncRequest.readyState() == 0 || ayncRequest.readyState() == 4){
      ayncRequest.open(method, URL);
      ayncRequest.setReqHeader("App", "Device");
      ayncRequest.setReqHeader("Content-Type", "application/json");  
      ayncRequest.setReqHeader("timestamp", millis());       
      String auth_key = "dh "+getAuthKey();  
      ayncRequest.setReqHeader("Authorization", auth_key.c_str());
      myLogger(FPSTR(__func__), "End");
      return true;
    }
  }
  else if(retry){
      myLogger(FPSTR(__func__), "No Internet connection.... Retrying initAyncHTTP.....");
      return initAyncHTTP(method, URL, false);
  }
  myLogger(FPSTR(__func__), "initAyncHTTP failed.....\nEnd"); 
  return false;  
}


String DreamHomeMaster::applianceDatabaseByMAC(String apMAC){
  myLogger(FPSTR(__func__), "Start");
  JSONVar apObject;
  bool skip = false;
  String filepath = "/"+String(DB_STRING[AP])+"/"+apMAC;
  JSONVar fileArr = database->listFiles(filepath.c_str());
  for(int j=0; j<fileArr.length(); j++) {
    String port = (const char*)fileArr[j];
    myLogger(FPSTR(__func__), "Port: "+ port);
    JSONVar jsonVar = database->getJSON(AP, apMAC, port);
    if(JSON.typeof(jsonVar)=="undefined" || JSON.typeof(jsonVar[0])=="undefined"){
      myLogger(FPSTR(__func__), "End");
      return JSON.stringify(apObject);
    }
    int category = (int)jsonVar[0];
    int is_active = (int)jsonVar[1];
    int manual = (int)jsonVar[2];
    int status = (int)jsonVar[3];
    myLogger(FPSTR(__func__), "category: "+String(category)+", is_active: "+String(is_active)+", manual: "+String(manual)+", status: "+ String(status));
    if (is_active){
      if(!manual){
        String opMac = (const char*)jsonVar[4];
        String opPort = (const char*)jsonVar[5]; 
        JSONVar opjsonVar = database->getJSON(OP, opMac, opPort);
        if(JSON.typeof(opjsonVar)=="undefined" || JSON.typeof(opjsonVar[1])=="undefined"){
          myLogger(FPSTR(__func__), "End");
          return JSON.stringify(apObject);
        }
          status = (int)opjsonVar[1];
      }
    }else{
      status = false;
    }
    apObject[apMAC][port][0] = category;
    apObject[apMAC][port][1] = is_active;
    apObject[apMAC][port][2] = manual;
    apObject[apMAC][port][3] = status;
    
  }
  
  myLogger(FPSTR(__func__), "End");
  return JSON.stringify(apObject);
}


String DreamHomeMaster::applianceDatabase(){
  myLogger(FPSTR(__func__), "Start");
  JSONVar apObject;
  String dirname = "/"+String(DB_STRING[AP]);
  JSONVar dirArr = database->listDir(dirname.c_str());
  for(int i=0; i<dirArr.length(); i++){ 
    String mac = (const char*)dirArr[i];
    myLogger(FPSTR(__func__), "MAC: "+ mac);
    String filepath = dirname+"/"+mac;
    JSONVar fileArr = database->listFiles(filepath.c_str());
    for(int j=0; j<fileArr.length(); j++) {
      String port = (const char*)fileArr[j];
      myLogger(FPSTR(__func__), "Port: "+ port);
      JSONVar jsonVar = database->getJSON(AP, mac, port);
      int category = (int)jsonVar[0];
      int is_active = (int)jsonVar[1];
      int manual = (int)jsonVar[2];
      int status = (int)jsonVar[3];
      myLogger(FPSTR(__func__), "category: "+String(category)+", is_active: "+String(is_active)+", manual: "+String(manual)+", status: "+ String(status)); 
      if (is_active){
        if(!manual){
          String opMac = (const char*)jsonVar[4];
          String opPort = (const char*)jsonVar[5]; 
          jsonVar = database->getJSON(OP, opMac, opPort);
          status = (int)jsonVar[1];
        }
      }else{
        status = false;
      }
      apObject[mac][port][0] = category;
      apObject[mac][port][1] = is_active;
      apObject[mac][port][2] = manual;
      apObject[mac][port][3] = status;
    }
  }
  myLogger(FPSTR(__func__), "End");
  return JSON.stringify(apObject);
}

String DreamHomeMaster::operatorDatabaseByMAC(String opMAC){
  myLogger(FPSTR(__func__), "Start");
  JSONVar opObject;
  bool skip = false;
  String filepath = "/"+String(DB_STRING[SERVER_OP])+"/"+opMAC;
  JSONVar fileArr = database->listFiles(filepath.c_str());
  for(int j=0; j<fileArr.length(); j++) {
    String port = (const char*)fileArr[j];
    myLogger(FPSTR(__func__), "Port: "+ port);
    JSONVar jsonVar = database->getJSON(SERVER_OP, opMAC, port);
    if(JSON.typeof(jsonVar)=="undefined" || JSON.typeof(jsonVar[0])=="undefined"){
      myLogger(FPSTR(__func__), "End");
      return JSON.stringify(opObject);
    }
    int category = (int)jsonVar[0];
    int is_active = (int)jsonVar[1];
    double sensor_max_val = (double)jsonVar[2];
    double sensor_min_val = (double)jsonVar[3];
    int calibration_ref1 = (int)jsonVar[4];
    int calibration_ref2 = (int)jsonVar[5];
    double capacity = (double)jsonVar[8];
    opObject[opMAC][port][0] = category;
    opObject[opMAC][port][1] = is_active;
    opObject[opMAC][port][2] = sensor_max_val;
    opObject[opMAC][port][3] = sensor_min_val;
    opObject[opMAC][port][4] = calibration_ref1;
    opObject[opMAC][port][5] = calibration_ref2;
    opObject[opMAC][port][6] = capacity;
    if(category==WATER_TANK){
      opObject[opMAC][port][7]  = (double)jsonVar[9];
      opObject[opMAC][port][8]  = (double)jsonVar[10];
    }
  }
//   debugStream->println(opObject);
  myLogger(FPSTR(__func__), "End");
  String opValue = JSON.stringify(opObject);
//   debugStream->print("opValue: ");
//   debugStream->println(opValue);
  return opValue;
}



String DreamHomeMaster::operatorDatabase(){
  myLogger(FPSTR(__func__), "Start");
  JSONVar opObject;
  String dirname = "/"+String(DB_STRING[OP]);
  JSONVar dirArr = database->listDir(dirname.c_str());
  for(int i=0; i<dirArr.length(); i++){ 
    String mac = (const char*)dirArr[i];
    myLogger(FPSTR(__func__), "Mac: " + mac);
    String filepath = dirname+"/"+mac;
    JSONVar fileArr = database->listFiles(filepath.c_str());
    for(int j=0; j<fileArr.length(); j++) {
      String port = (const char*)fileArr[j];
      myLogger(FPSTR(__func__), "Port: " +port);
      JSONVar jsonVar = database->getJSON(OP, mac, port);
      if(JSON.typeof(jsonVar)=="undefined" || JSON.typeof(jsonVar[0])=="undefined"){
        myLogger(FPSTR(__func__), "End");
        return JSON.stringify(opObject);
      }
      opObject[mac][port][0] = (double)jsonVar[0];
      opObject[mac][port][1] = (int)jsonVar[1];
      opObject[mac][port][2] = (int)jsonVar[2];
      opObject[mac][port][3] = (int)jsonVar[3];
    }
  }
  myLogger(FPSTR(__func__), "End");
  return JSON.stringify(opObject);
}



//-----------------------------------------SIM800L---------------------------------------------


void DreamHomeMaster::initSIM800l(){
  myLogger(FPSTR(__func__), "Initializing SIM800L.");
  Serial2.begin(4800, SERIAL_8N1, SIM800_RX_PIN, SIM800_TX_PIN);
  sim800l = new SIM800L((Stream *)&Serial2, SIM800_RST_PIN, 2048, 2048, debugStream);
}

bool DreamHomeMaster::sim800lAttached(bool retry){
  debugStream->print("Serial2: ");
  debugStream->println(Serial2);
  if(Serial2!=NULL){
    myLogger(FPSTR(__func__), "SIM800L is attached.");
    return true;
  }
  if(retry){
    initSIM800l();
    return sim800lAttached(false);
  }
  myLogger(FPSTR(__func__), "SIM800L is not attached.");
  return false;
}


void DreamHomeMaster::setupSIM800L() {
  if(!sim800lAttached())return;
  myLogger(FPSTR(__func__), "Reseting the SIM800L module.....");  
//  setPowerMode(MINIMUM);
  sim800l->reset();
  database->ayncDelay(3000);     
  // Wait until the module is ready to accept AT commands 
  myLogger(FPSTR(__func__), "Setting up the SIM800L module!");    

    if(sim800lReady()){
      if(setPowerMode(NORMAL)){
        if(checkSignal()){
          if(registerNetwork()){
            if(configGPRS()){
              if(connectGPRS()){
                myLogger(FPSTR(__func__), "SIM800L is Ready To GO");
                return;                   
              }
            }
          }
        }
      } 

    } 
   setupSIM800L();
}


bool DreamHomeMaster::sim800lReady(){
  if(!sim800lAttached(true))return false;
  bool isReady = false;
  while(!isReady){
  for(uint8_t i = 0; i < 5 && !isReady; i++) {
    myLogger(FPSTR(__func__), "Initializing AT command");
    database->ayncDelay(1000);
    isReady = sim800l->isReady();
  }
  // Check if connected, if not reset the module and setup the config again
  if(isReady) {
    myLogger(FPSTR(__func__), "SIM800L Ready !");
  } else {
    myLogger(FPSTR(__func__), "SIM800L NOT Ready !");
    setupSIM800L();
    } 
  }
  
  imei = sim800l->getIMEI();
  myLogger(FPSTR(__func__), imei);
  return isReady;
}


bool DreamHomeMaster::setPowerMode(PowerMode powerMode){
    bool success = false;
    String strPowerMode = "";
    if(powerMode == MINIMUM){
      strPowerMode = "MINIMUM";
    }else if(powerMode == NORMAL){
      strPowerMode = "NORMAL";
    }else if(powerMode==SLEEP){
      strPowerMode = "SLEEP";
    }
    
    while(!success){
      for(uint8_t i = 0; i < 5 && !success; i++) {
          myLogger(FPSTR(__func__), "Swithing SIM800L in "+strPowerMode+" power mode.....");
          database->ayncDelay(1000);
          success = sim800l->setPowerMode(powerMode);
      }

      if(success) {
        myLogger(FPSTR(__func__), "SIM800L in "+strPowerMode+" power mode");
      } else {
        myLogger(FPSTR(__func__), "Failed to switch SIM800L to "+strPowerMode+" power mode");
        setupSIM800L();
      }        
    }
    database->ayncDelay(3000);
    return success;      
}

bool DreamHomeMaster::checkSignal(){
  bool signalOk = false; 
  uint8_t signal = 0;
  while(!signalOk) {
      for(uint8_t i = 0; i < 15 && !signalOk; i++) {
       database->ayncDelay(1000);
       myLogger(FPSTR(__func__), "Checking Signal strength: ");
       signal = sim800l->getSignal();
       myLogger(FPSTR(__func__), String((char *)signal));
       signalOk = signal>5;     
      }

    if(signalOk) {
      myLogger(FPSTR(__func__), "Signal OK (strength: )\nEnd" );
    } else {
      myLogger(FPSTR(__func__), "Weak Signal (strenght: )\nEnd" );
      return false;
    }   
  }
  database->ayncDelay(10);  
  return signalOk;
}

bool DreamHomeMaster::registerNetwork(){
  bool networkOK = false;
  NetworkRegistration network;
  while(!networkOK) {
    if(enableDebug){
    //   debugStream->println(F("Registering to network with either : "));
    //   debugStream->print(REGISTERED_HOME);
    //   debugStream->print(" OR ");
    //   debugStream->println(REGISTERED_ROAMING);      
    }


    for(uint8_t i = 0; i < 10 && !networkOK; i++) {
      database->ayncDelay(1000);  
      network = sim800l->getRegistrationStatus();
      networkOK = network == REGISTERED_HOME || network == REGISTERED_ROAMING;
      if(enableDebug){
      //   debugStream->print(F("Current network: "));
      //   debugStream->println(network);
     }
      
    }
    if(networkOK) {
      myLogger(FPSTR(__func__), "Network registration OK!");
    } else {
      myLogger(FPSTR(__func__), "Network registration Failed !");
      if (checkSignal()){
        return registerNetwork();
      }
      return false;
    }   
  }  
  database->ayncDelay(1000);
  return networkOK;
}

bool DreamHomeMaster::configGPRS(){
  bool success = false;
  while(!success) {
    for(uint8_t i = 0; i < 10 && !success; i++) {
      database->ayncDelay(5000);
      success = sim800l->setupGPRS(APN);
    }
    if(success) {
      myLogger(FPSTR(__func__), "GPRS config OK");
    }else{
      myLogger(FPSTR(__func__), "GPRS configuration failed");  
      if (checkSignal() && registerNetwork()){
        return configGPRS();
      }
      return false;
    }
  }
  database->ayncDelay(100);
  return success;  
}

bool DreamHomeMaster::disconnectGPRS(){
  bool disconnected = false;
  while(!disconnected){
  for(uint8_t i = 0; i < 10 && !disconnected; i++) {
    myLogger(FPSTR(__func__), F("Disconnecting to GPRS: "));
    database->ayncDelay(1000);
    disconnected = sim800l->disconnectGPRS();
    myLogger(FPSTR(__func__), String(btoa(disconnected)));
  }
  
  if(disconnected) {
    myLogger(FPSTR(__func__), F("GPRS disconnected !"));
    disconnected = true;
  } else {
    myLogger(FPSTR(__func__), F("GPRS still connected !"));
    return false;
  }      
  }
  database->ayncDelay(100);
  return disconnected;
}

bool DreamHomeMaster::connectGPRS(){
  if(!sim800lAttached(true)) return false;
  connected = false;
//  disconnectGPRS();
  while(!connected){
  for(uint8_t i = 0; i < 10 && !connected; i++) {
    myLogger(FPSTR(__func__), F("Connecting to GPRS: "));
    database->ayncDelay(1000);
    connected = sim800l->connectGPRS();
    myLogger(FPSTR(__func__), String(btoa(connected)));
  }

  // Check if connected, if not reset the module and setup the config again
  if(connected) {
    myLogger(FPSTR(__func__), F("GPRS connected !"));
//  //   debugStream->println(sim800l->getIP());
  } else {
    myLogger(FPSTR(__func__), F("GPRS not connected !"));
    if (sim800lReady() && checkSignal() && registerNetwork() && configGPRS()){
      if (disconnectGPRS()){
        return connectGPRS();
      }
    }
    return false;
  } 
  } 
  return connected; 
}

// uint16_t DreamHomeMaster::doGet(const char* url, const char* headers, uint16_t serverReadTimeoutMs){
//   if(mode!=MASTER){
//     myLogger(FPSTR(__func__), F("DreamHomeMaster : SLAVE doesn't have this functionality."));
//     return 0;
//   }  
//   return sim800l->doGet(url, headers, serverReadTimeoutMs);
// }

// uint16_t DreamHomeMaster::doPost(const char* url, const char* headers, const char* contentType, String payload, uint16_t clientWriteTimeoutMs, uint16_t serverReadTimeoutMs){
//   if(mode!=MASTER){
//     myLogger(FPSTR(__func__), F("DreamHomeMaster : SLAVE doesn't have this functionality."));
//     return 0;
//   }  
//   return sim800l->doPost(url, headers, contentType, payload, clientWriteTimeoutMs, serverReadTimeoutMs);
// }

// uint16_t DreamHomeMaster::getDataSizeReceived(){
//   if(mode!=MASTER){
//     myLogger(FPSTR(__func__), F("DreamHomeMaster : SLAVE doesn't have this functionality."));
//     return 0;
//   }  
//   return sim800l->getDataSizeReceived();
// }

// char* DreamHomeMaster::getDataReceived(){
//   if(mode!=MASTER){
//     myLogger(FPSTR(__func__), F("DreamHomeMaster : SLAVE doesn't have this functionality."));
//     return NULL;
//   }  
//   return sim800l->getDataReceived();
// }

const char* DreamHomeMaster::simHttpPost(const char* url, const String& payload){
  myLogger(FPSTR(__func__), F("Start HTTP POST..."));
  const char* recived;
  if(connectGPRS()){    
    String headers = "App: Device,\r\nAuthorization: dh "+getAuthKey();
    uint16_t rc = sim800l->doPost(url, headers.c_str(), "application/json", payload, 20000, 20000);
    recived = simHttpResponse(rc);
  } 
  
  // if (simHttpfail>10){
  //   setupSIM800L();
  // }
  return recived;
}


const char* DreamHomeMaster::simHttpResponse(uint16_t rc){
  httpResponseSize = 0;
  httpResponseOK = false;  
  const char* recived;
  if(rc == 200) {
    httpResponseOK = true;
    httpResponseSize = sim800l->getDataSizeReceived();
    recived = sim800l->getDataReceived();
    simHttpfail = 0;
    return recived;
  }
  else if(rc>599){
    simHttpfail++;
  }else{
      simHttpfail--;
  }  
  return NULL;
}



