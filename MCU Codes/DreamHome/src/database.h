#ifndef _Database_H_
#define _Database_H_

#include <FS.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>

#define SPIFFS_ALIGNED_OBJECT_INDEX_TABLES  true

enum DB {SERVER_OP, OP, AP, NA};
static const char *DB_STRING[] = {
    "SERVER_OP", "OP", "AP", "NA",
};

#define btoa(x) ((x)?"true":"false")



class Database{
	public:
		Database(Stream* _debugStream = NULL);
		~Database();
		void getValues(DB db, String mac, String port, char* buffer);
		JSONVar getJSON(DB db, String mac, String port);
		bool putValue(DB db, String mac, String port, String data);
		JSONVar listDir(const char * dirname);
		JSONVar listFiles(const char * filepath);
		void logger(String func_name, String text);
		void setDebug(bool endebug=true);			
		void ayncDelay(unsigned long duration);	
	private:
		Stream* debugStream = NULL;
		bool enableDebug = false;	
		bool debug = false;
		void myLogger(String func_name, String text);			
		String getFilename(DB db, String mac, String port);
		int bufferSize(DB db);
		void doBreakup(char s[], const char* delimit, char *ptr[]);
		void createNestedDir(char * dirname, size_t size);
};
#endif