#ifndef _DreamHomeMaster_H_
#define _DreamHomeMaster_H_


#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <EEPROM.h>
// #include <SoftwareSerial.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <asyncHTTPrequest.h>

#include <AsyncTCP.h>
// #include <Hash.h>
#include <Arduino_JSON.h>
#include "prerequisite/SIM800L.h"
#include <base64.h>
#include "database.h"


enum EEPROM_POOL_TYPE {BLOCK, POINT};
enum AUTH_PROTOCOL {WIFI, SIM};
enum OPERATOR_CATEGORY {WATER_TANK};


// #define RXD2 16
// #define TXD2 17

#define SIM800_RX_PIN 16
#define SIM800_TX_PIN 17
#define SIM800_RST_PIN 5
#define APN  "Internet.be"
#define CRED_IDX 20

#define AP_CHANNEL 1
#define AP_HIDDEN 0
#define AP_MAX_CON 20
#define WIFI_CHANNEL 13


class DreamHomeMaster
{
	public:
		DreamHomeMaster(Stream* _debugStream = NULL, bool writeCreds=false);
		~DreamHomeMaster();
		String getEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type);
		uint16_t putEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type, const char* value);
		String getMACfromIP(IPAddress ip);
		void ayncHttpGet(const char* URL);
		void ayncHttpPost(const char* URL);
		void ayncHttpPost(const char* URL, const char* body);
		String operatorDatabase();
		String operatorDatabaseByMAC(String opMAC);
		String applianceDatabase();		
		String applianceDatabaseByMAC(String apMAC);
		bool operatorLogic(String opPort);
		void setupSIM800L();
		// uint16_t doGet(const char* url, const char* headers, uint16_t serverReadTimeoutMs);
		// uint16_t doPost(const char* url, const char* headers, const char* contentType, String payload, uint16_t clientWriteTimeoutMs, uint16_t serverReadTimeoutMs);
		bool connectWiFi(bool retry);
		String wifiSSIDbuff[10];
		String wifiPASSbuff[10];
		String myMAC;
		bool httpResponseOK;
		uint16_t httpResponseSize;
		const char* simHttpPost(const char* url, const String& payload);
		asyncHTTPrequest ayncRequest;		
		void writeCredentials(const String creds[], size_t creds_len);
		Database* database;
		void logger(String func_name, String text);
		void setDebug(bool endebug=true);

	protected:
		Stream* debugStream = NULL;
		bool enableDebug = false;
		bool debug = false;
		void myLogger(String func_name, String text);
		bool connected = false;
		SIM800L* sim800l;
		WiFiClient client;
		uint16_t simHttpfail=0;
		AUTH_PROTOCOL protocol;
		int16_t strIndex(const char* str, const char* findStr, uint16_t startIdx);
		// char* _formatMAC(const unsigned char* mac);
		void readCredentials();
		bool connectInternet();
		bool scanWiFiConnect();
		void initSIM800l();
		bool sim800lAttached(bool retry=true);
		bool sim800lReady();
		bool setPowerMode(PowerMode powerMode);
		bool checkSignal();
		bool registerNetwork();
		bool configGPRS();
		bool disconnectGPRS();

		void startHotspot();
		bool isWiFiConnected(bool retry=false);
		bool connectGPRS();
		// uint16_t getDataSizeReceived();
		// char* getDataReceived();		
		const char* simHttpResponse(uint16_t rc);
		int find(int a[], int size, int el);
		void sort(int a[], int size, bool dsc);	
		bool initAyncHTTP(const char* method, const char* URL, bool retry);
	private:	
		String ssid = "";
		String pass = "";
		String token_key = "";	
		String imei = "";	
		int16_t getCredsEndAddr();
		bool getEEPROMPermission(uint16_t _addrOffset);
		String _getEEPROM(uint16_t _addrOffset);
		uint16_t _putEEPROM(uint16_t _addrOffset, const char* value);
		
		String getAuthKey();
		String getWiFiSSID();
		String getWiFiPASS();
		void saveMasterWiFiCreds();
		void retriveMasterWiFiCreds();

};
#endif