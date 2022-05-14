#ifndef _DreamHomeAppliance_H_
#define _DreamHomeAppliance_H_

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <AsyncJson.h>
#include <Hash.h>
#include <Arduino_JSON.h>
#include "prerequisite/pinMapping.h"
#include "database.h"


enum EEPROM_POOL_TYPE {BLOCK, POINT};
enum APPLIANCE_CATEGORY {PUMP_WITH_PANNEL, PUMP_WITHOUT_PANNEL};

#define CRED_IDX 20
#define PUMP_WITH_PANNEL_STATUS_IDX 100
#define PUMP_WITHOUT_PANNEL_STATUS_IDX 101

class DreamHomeAppliance
{
	public:
		DreamHomeAppliance(Stream* _debugStream = NULL, bool writeCreds=false);
		~DreamHomeAppliance();
		String getEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type);
		uint16_t putEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type, const char* value);
		String applianceDatabase();		
		void setPorts();
		bool connectWiFi(bool retry);
		String myMAC;
		Database* database;
		void writeCredentials(const String creds[], size_t creds_len);
		void logger(String func_name, String text);
		void setDebug(bool endebug=true);			
	protected:
		Stream* debugStream = NULL;
		bool enableDebug = false;
		bool debug = false;
		void myLogger(String func_name, String text);		
		pinMapping* pinMap;
		ESP8266WiFiMulti WiFiMulti;
		int16_t strIndex(const char* str, const char* findStr, uint16_t startIdx);
		void readCredentials();
		bool isWiFiConnected(bool retry=true);
		int find(int a[], int size, int el);
		void sort(int a[], int size, bool dsc);	
		void applianceLogic(int category, String port, int status);
	private:	
		String ssid = "";
		String pass = "";
		int16_t getCredsEndAddr();
		bool getEEPROMPermission(uint16_t _addrOffset);
		String _getEEPROM(uint16_t _addrOffset);
		uint16_t _putEEPROM(uint16_t _addrOffset, const char* value);

		String getWiFiSSID();
		String getWiFiPASS();
		
};

#endif