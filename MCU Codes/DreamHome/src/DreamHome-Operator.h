#ifndef _DreamHomeOperator_H_
#define _DreamHomeOperator_H_

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>
#include <AsyncJson.h>
#include <Hash.h>
#include "prerequisite/pinMapping.h"
#include "database.h"
#include <Arduino_JSON.h>
#include <Ticker.h>

enum EEPROM_POOL_TYPE {BLOCK, POINT};
enum AUTH_PROTOCOL {WIFI};
enum OPERATOR_CATEGORY {WATER_TANK};
// enum DB {MAC_PORT, OP, AP, NA};

#define CRED_IDX 20
#define WATER_SENSOR_PREV_VALUE_IDX 100
#define WATER_SENSOR_PREV_STATUS_IDX 101
#define LOW_REF_START_IDX 70
#define HIGH_REF_START_IDX 80
#define WIFI_CHANNEL 1
#define MAX_LOOP 10


class DreamHomeOperator
{
	public:
		DreamHomeOperator(Stream* _debugStream, bool write_creds=false);
		~DreamHomeOperator();
		String getEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type);
		uint16_t putEEPROM(uint16_t _addrOffset, EEPROM_POOL_TYPE pool_type, const char* value);
		String operatorDatabase();	
		bool operatorLogic(String opPort);
		bool connectWiFi(bool retry);
		String myMAC;
		bool httpResponseOK;
		uint16_t httpResponseSize;	
		void writeCred(const String creds[], size_t creds_len);
		Database* database;
		void logger(String func_name, String text);
		void setDebug(bool endebug=true);		
		bool calibrate(int op_cat, String opPort, bool level);
		void calibrationDone(String opPort);
	protected:
		Stream* debugStream = NULL;
		bool enableDebug = false;
		bool debug = false;
		void myLogger(String func_name, String text);		
		bool enableBlinker = false;
		Ticker ticker1;
		Ticker ticker2;
		bool connected = false;
		pinMapping* pinMap;
		ESP8266WiFiMulti WiFiMulti;
		AUTH_PROTOCOL protocol;
		JSONVar sensor_value_arr;
		int total_loop=0;
		int loop=0;
		void OnOff(bool state);
		void blinker1();
		int16_t strIndex(const char* str, const char* findStr, uint16_t startIdx);
		void readCredentials();
		bool isWiFiConnected(bool retry=true);		
		int find(int a[], int size, int el);
		void sort(int a[], int size, bool dsc);	
		bool updateSlaveOperatorDatabaseFromSensor(String opPort, double sensor_value, bool status);
		double readPorts(OPERATOR_CATEGORY op_cat, String opPort);
		int portToIdx(String opPort, bool level);
		bool opDatabaseUpdate(JSONVar jsonVar, String opPort);

	private:	
		String ssid = "";
		String pass = "";
		String imei = "";	
		int16_t getCredsEndAddr();
		bool getEEPROMPermission(uint16_t _addrOffset);
		String _getEEPROM(uint16_t _addrOffset);
		uint16_t _putEEPROM(uint16_t _addrOffset, const char* value);
		String getWiFiSSID();
		String getWiFiPASS();
};

#endif