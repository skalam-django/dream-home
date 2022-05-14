#include <DreamHome-Appliance.h>
DreamHomeAppliance* dreamhome;
void setup() {
  Serial.begin(115200);
  dreamhome = new DreamHomeAppliance((Stream *)&Serial, true);
  const String creds[] = {"Dream Home: GnuvbF4EY2tpGzLLjIk", "Z7e1zEJR"};
  int creds_len = (int)sizeof(creds)/sizeof(creds[0]);
  dreamhome->writeCredentials(creds, creds_len);
}
void loop() {}
