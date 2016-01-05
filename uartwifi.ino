#define SSID       "MAIN" // ™◊ËÕ ssid
#define PASSWORD   "" // √À— ºË“π


#include "uartWIFI.h"
#include <SoftwareSerial.h>
WIFI wifi;

extern int chlID;  //client id(0-4)

int led = 12;

void setup() {
  Serial.begin(9600);
  wifi.begin();
  bool b = wifi.Initialize(STA, SSID, PASSWORD);
  delay(8000);  //ÀπË«ß‡«≈“„ÀÈ‡§√◊ËÕß‡™◊ËÕ¡°—∫ wifi
  wifi.confMux(1);
  delay(200);
  if(wifi.confServer(1,8080)){ // ‡™◊ËÕ¡µËÕ wifi  ”‡√Á® „ÀÈ· ¥ß‰ø ∂“π–
    Serial.print("Server Ready at port: ");
    Serial.println(8080);
  }
  Serial.println(wifi.showIP());
}
void loop() {

  char buf[100];
  int iLen = wifi.ReceiveMessage(buf);
  if(iLen > 0)
  {
    if (strcmp(buf, "HELLO") == 0) // ∂È“¡’§”«Ë“ HELLO ®–‡ª‘¥/ª‘¥ LED
    {
      if(digitalRead(led)==0){
        digitalWrite(led,1);
        wifi.Send(chlID,"LED ON ");
      }
      else{
        digitalWrite(led,0);
        wifi.Send(chlID,"LED OFF");
      }
       //wifi.Send(chlID,"HELLO BACK"); //  Ëß¢ÈÕ¡Ÿ≈∑’ËµÈÕß°“√„ÀÈ°—∫ client
    }
  
  }
}

