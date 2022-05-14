#include <DreamHome-Operator.h>
DreamHomeOperator* dreamhome;
void setup() {
  Serial.begin(115200);
  dreamhome = new DreamHomeOperator((Stream *)&Serial, true);
  dreamhome->setDebug(true);
//  dreamhome->database->setDebug(true);
  Serial.println("zzzzzzzzz1");
  const String creds[] = {"Dream Home: GnuvbF4EY2tpGzLLjIk", "Z7e1zEJR"};
  Serial.println("zzzzzzzzzz2");
  int creds_len = (int)sizeof(creds)/sizeof(creds[0]);
  Serial.println("zzzzzzzzzz3");
//  Serial.println("zzzzzzzzz4 "+String(__func__));
  dreamhome->writeCred(creds, creds_len);
//  EEPROM.write(70, 1);
//  EEPROM.write(80, map(948, 0, 1024, 0, 255));
//  EEPROM.commit();
//  Serial.println("Done");
}
void loop() {}
