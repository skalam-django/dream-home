#include "database.h"


Database::Database(Stream* _debugStream)
{
  enableDebug = _debugStream != NULL;
  debugStream = _debugStream;
  debugStream->println("Mount LittleFS");
  // LittleFS.format();
  while (!LittleFS.begin()) {
  	ayncDelay(300);
    debugStream->println("LittleFS mount failed. Formatting........");
    LittleFS.format();
    while(1);
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
			return 50*sizeof(char);					
	}
}



JSONVar Database::listDir(const char * dirname) { 
	myLogger(FPSTR(__func__), "Start");
	JSONVar json;
	// if (!getAcessDB()){
	// 	return json;
	// }		
	// updateAcessDB(true);
	Dir root = LittleFS.openDir(dirname);
	// updateAcessDB(false);
	int i = 0;
	while (root.next()) {
		// updateAcessDB(true);
		File fileDB = root.openFile("r");
		if(fileDB.isDirectory()){
			String str = root.fileName();
			json[i++] = str;
		}
		fileDB.close();
		// updateAcessDB(false);
	}
	myLogger(FPSTR(__func__), "End");
    return json; 
} 

JSONVar Database::listFiles(const char * filepath){
	myLogger(FPSTR(__func__), "Start");
	JSONVar json;
	// if (!getAcessDB()){
	// 	return json;
	// }	
	// updateAcessDB(true);
	Dir root = LittleFS.openDir(filepath);
	// updateAcessDB(false);
	int i = 0;
	while (root.next()) {
		// updateAcessDB(true);
		File fileDB = root.openFile("r");
		if(fileDB.isFile()){
			String str = root.fileName();
			json[i++] = str;
		}
		fileDB.close();
		// updateAcessDB(false);
	}
	myLogger(FPSTR(__func__), "End");
    return json; 
}



void Database::getValues(DB db, String mac, String port, char* buffer){
	myLogger(FPSTR(__func__), "Start");
	String filename = getFilename(db, mac, port);
	myLogger(FPSTR(__func__), "Opening filename: "+ filename +" to read.");
	// if (!getAcessDB())return;
	// updateAcessDB(true);
	File fileDB = LittleFS.open(filename, "r");
	if(!fileDB.isFile() || !fileDB) {
		myLogger(FPSTR(__func__), "Failed to open file for reading.\ndb: "+String(DB_STRING[db])+"\nfilename: "+filename+"\n End");
		buffer[0] = '\0';
		fileDB.close();
		// updateAcessDB(false);
		return;
	}
	int i = 0;
	while(fileDB.available()){
		buffer[i++] = fileDB.read();
	}
	buffer[i] = '\0';
	fileDB.close();
	myLogger(FPSTR(__func__), "End");
	// updateAcessDB(false);
}


JSONVar Database::getJSON(DB db, String mac, String port){
	myLogger(FPSTR(__func__), "Start");
	JSONVar json;
	size_t size = bufferSize(db);
	char* buffer = (char*)malloc(size * sizeof(char));
	getValues(db, mac, port, buffer);
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
  char *p;
  int i = 0;
  p  = strtok(s, delimit);
  while (p) {
    ptr[i++] = p;
    p = strtok(NULL, delimit);
  }
  // memset(slots, 0, sizeof(slots));
}

void Database::createNestedDir(char * dirname, size_t size){
	myLogger(FPSTR(__func__), "Start");
	char* slots[size+1];
	bool done;
	doBreakup(dirname, "/", slots);
	String dir = "/";
	for(int i=0; i<size; i++){
		dir += String(slots[i]);
		debugStream->println(dir);	
		if(!LittleFS.exists(dir)){
			// updateAcessDB(true);
			myLogger(FPSTR(__func__), "Creating "+dir);
			myLogger(FPSTR(__func__), "Directory Created? " + String(btoa(LittleFS.mkdir(dir))));
			// updateAcessDB(false);
		}else{
			myLogger(FPSTR(__func__), "Directory already exists.");
		}
		dir += "/";
		debugStream->println(dir);
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
	// if (!getAcessDB()){
	// 	return done;
	// }
	// updateAcessDB(true);
	File fileDB = LittleFS.open(filename, "w");	
	if (!fileDB) {
		myLogger(FPSTR(__func__), "Failed to open file for writing\n db: "+String(DB_STRING[db])+"\nfilename: "+filename+"\n End");
		// updateAcessDB(false);
		fileDB.close();
		myLogger(FPSTR(__func__), "End");
		return done;
	}

	int count = jsonVar.printTo(fileDB);
	ayncDelay(10);
	fileDB.close();
	// updateAcessDB(false);
	if(count>0){
		done = true;
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
	// logger(FPSTR(__func__), "Start\n"+String(duration));
	unsigned long st = millis();
	for(;;){
		if((millis()-st)>=duration)break;
		delay(0);
	}
	// logger(FPSTR(__func__), "End");
}

// bool Database::getAcessDB(){
// 	int max_try = 3000/300;
// 	int count  = 0;
// 	while(dbLock && count<max_try){
// 		delay(300);
// 		count++;
// 	}
// 	if (count<max_try){
// 		return true;
// 	}
// 	debugStream->print("DB access blocked. count : ");
// 	debugStream->print(count);
// 	debugStream->print("  lock: ");
// 	debugStream->println(dbLock);	
// 	return false;
// }

// void Database::updateAcessDB(bool lock){
// 	fileDB.close();
// 	delay(500);
// 	dbLock = lock;
// 	debugStream->print("DB access updated to:  ");
// 	debugStream->println(dbLock);	
// }