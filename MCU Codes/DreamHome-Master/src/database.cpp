#include "database.h"


Database::Database(Stream* _debugStream)
{
  enableDebug = _debugStream != NULL;
  debugStream = _debugStream;
  myLogger(FPSTR(__func__), "Start\nMounting LittleFS.....");
  while (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
	ayncDelay(300);
	myLogger(FPSTR(__func__), "LittleFS mount failed. Formatting........");
	LittleFS.format();
	myLogger(FPSTR(__func__), "Rebooting......");
	ESP.restart();
  }	
}

Database::~Database() {
	
}	

String Database::getFilename(DB db, String mac, String port){
	if(mac.length()>0){
		mac += "/";
	}
	return "/"+String(DB_STRING[db])+"/"+mac+port;
}

int Database::bufferSize(DB db){
	switch(db){
		case AP:
			return 40*sizeof(char);
		case OP:
			return 20*sizeof(char);
		case SERVER_OP:
			return 70*sizeof(char);					
	}
}



JSONVar Database::listDir(const char * dirname) { 
	myLogger(FPSTR(__func__), "Start");
	JSONVar json;
	File root = LittleFS.open(dirname);
	int i = 0;
	File file = root.openNextFile();
	while (file) {
		if(file.isDirectory()){
			String str = file.name();
			str.replace(dirname, "");
			str.replace("/", "");
			json[i++] = str;
		}
		file.close();
		file = root.openNextFile();
	}
	root.close();
	myLogger(FPSTR(__func__), "End");
    return json; 
} 

JSONVar Database::listFiles(const char * filepath){
	myLogger(FPSTR(__func__), "Start");
	JSONVar json;
	File root = LittleFS.open(filepath);
	int i = 0;
	File file = root.openNextFile();
	while (file) {
		if(!file.isDirectory()){
			String str = file.name();
			str.replace(filepath, "");
			str.replace("/", "");
			json[i++] = str;
		}
		file.close();
		file = root.openNextFile();
	}
	root.close();
	myLogger(FPSTR(__func__), "End");
    return json; 
}



void Database::getValues(DB db, String mac, String port, char* buffer){
	myLogger(FPSTR(__func__), "Start");
	String filename = getFilename(db, mac, port);
	myLogger(FPSTR(__func__), "Opening filename: "+ filename +" to read.");
	File fileDB = LittleFS.open(filename);
	if(fileDB.isDirectory() || !fileDB) {
		myLogger(FPSTR(__func__), "Failed to open file for reading.\ndb: "+String(DB_STRING[db])+"\nfilename: "+filename+"\n End");
		buffer[0] = '\0';
		fileDB.close();
		return;
	}
	int i = 0;
	while(fileDB.available()){
		char c = char(fileDB.read()); 
		buffer[i++] = c;
	}
	if(i==0){
		buffer = NULL;
	}else{
		buffer[i] = '\0';
	}
	fileDB.close();
	myLogger(FPSTR(__func__), "End");
}


JSONVar Database::getJSON(DB db, String mac, String port){
	myLogger(FPSTR(__func__), "Start");
	JSONVar json;
	size_t size = bufferSize(db);
	char* buffer = (char*)malloc(size * sizeof(char));
	getValues(db, mac, port, buffer);
	// myLogger(FPSTR(__func__), "buffer: " + String(buffer));
	if(buffer==NULL || strlen(buffer)==0){
		myLogger(FPSTR(__func__), "Nothing read.\nEnd");
		if(buffer!=NULL)free(buffer);
		return json;
	}
	json = JSON.parse(buffer);
	free(buffer);
	myLogger(FPSTR(__func__), "End");
	return json;
}


void Database::doBreakup(char s[], const char* delimit, char *ptr[])
{
	myLogger(FPSTR(__func__), "Start");
	char *p;
	int i = 0;
	p  = strtok(s, delimit);
	while (p) {
		ptr[i++] = p;
		p = strtok(NULL, delimit);
	}
	myLogger(FPSTR(__func__), "End");
}

void Database::createNestedDir(char * dirname, size_t size){
	myLogger(FPSTR(__func__), "Start");
	char* slots[size+1];
	bool done;
	doBreakup(dirname, "/", slots);
	String dir = "/";
	for(int i=0; i<size; i++){
		dir += String(slots[i]);
		myLogger(FPSTR(__func__), "Creating "+dir);	
		// if(!LittleFS.exists(dir)){
			myLogger(FPSTR(__func__), "Directory Created? " + String(btoa(LittleFS.mkdir(dir))));
		// }
		dir += "/";
	}
	myLogger(FPSTR(__func__), "End");
}

bool Database::putValue(DB db, String mac, String port, String data){
	myLogger(FPSTR(__func__), "Start");
	bool done = false;			
	JSONVar jsonVar = JSON.parse(data);			
	String filename = getFilename(db, mac, port);
	myLogger(FPSTR(__func__), "Creating Directory");
	size_t size = 2;
	if(mac.length()==0){
		size = 1;
	}
	createNestedDir(strdup(filename.c_str()), size);
	myLogger(FPSTR(__func__), "Opening file to write --> filename: "+filename);

	File fileDB = LittleFS.open(filename, FILE_WRITE);	
	if (!fileDB) {
		myLogger(FPSTR(__func__), "Failed to open file for writing\n db: "+String(DB_STRING[db])+"\nfilename: "+filename+"\n End");
		fileDB.close();
		return done;
	}
	myLogger(FPSTR(__func__), "Wrting.......");
	int count = jsonVar.printTo(fileDB);
	ayncDelay(10);
	fileDB.close();
	if(count>0){
		done = true;
	}else{
		myLogger(FPSTR(__func__), "Nothing written");
	}
	myLogger(FPSTR(__func__), "End");
	return done;
}

void Database::myLogger(String func_name, String text){
	if(debug && enableDebug){
		logger(func_name, text);
	}
}

void Database::setDebug(bool endebug){
	debug = endebug;
}

void Database::logger(String func_name, String text){
  if(enableDebug){
    debugStream->println();
    debugStream->println("######");
    debugStream->print(func_name);
    debugStream->print(" --> ");
    debugStream->println(text);
    debugStream->println("######");
  }
}

void Database::ayncDelay(unsigned long duration){
	unsigned long st = millis();
	for(;;){
		if((millis()-st)>=duration)break;
		delay(0);
	}
}

