#include <DreamHome-Master.h>
DreamHomeMaster* dreamhome;
void setup() {
  Serial.begin(115200);
  dreamhome = new DreamHomeMaster((Stream *)&Serial, true);
  dreamhome->database->setDebug(true);
  dreamhome->setDebug(true);
  const String creds[] = {"b32535b0f7ec27c2923bfbaa3a3af1f2998698d5", "Dream Home: GnuvbF4EY2tpGzLLjIk", "Z7e1zEJR"};
  int creds_len = (int)sizeof(creds)/sizeof(creds[0]);
  dreamhome->writeCredentials(creds, creds_len);
}
void loop() {}
