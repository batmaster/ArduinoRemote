
// นำเข้าไลบรารี่สำหรับพวกวัดอุณหภูมิ
#include <OneWire.h>
#include <DallasTemperature.h>

// ที่เป็นแนวๆ ขื่อตัวแปร ตามด้วยตัวเลขแบบนี้ เดาเลยน่าจะเกี่ยวกับกำหนด pin
#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

boolean heaterAuto = true;
boolean filterAuto = true;

#define RELAY_1 28
#define RELAY_2 26
#define RELAY_3 24
#define RELAY_4 22

// นำเข้าไลบรารี่สำหรับ SD ตอนนี้ ยังไม่ใช้
#include <SPI.h>
#include <SD.h>

const int chipSelect = 48;

// ตัวแกรเก็บว่าจะให้แสดงผลทุกอย่างที่เข้าออกบอร์ดหรือไม่ #มันแถมมากับโค้ด wifi
#define DEBUG true
// ตัวแปรเก็บชื่อบอร์ด
const String bid = "A0001";


// นำเข้าไลบรารี่ LCD
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
// ตั้งค่า pin สำหรับ LCD
#define I2C_ADDR 0x27 // <
#define BACKLIGHT_PIN 3

LiquidCrystal_I2C lcd(I2C_ADDR,2,1,0,4,5,6,7);


// นำเข้าไลบรารี่ Keypad
#include <Keypad.h>
//กำหนดตัวอักษรบนปุ่ม
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {32, 34, 36, 38};
byte colPins[COLS] = {40, 42, 44, 46};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );








// ตัวแปรพื้นฐาน
// mode: 0 = auto, 1 = manual
int HEATER_MODE;
int FILTER_MODE;
double HEATER_MIN;
double FILTER_MAX;
int WIFI_MODE;
String WIFI_SSID;
String WIFI_PASSWORD;
String BOARD_NAME;



#include <EEPROM.h>
// EEPROM
//  0            HAS_SET 0 set, 255 not set (255)
//  1            HEATER_MODE 0 auto, 1 manual (0)
//  2            FILTER_MODE 0 auto, 1 manual (0)
//  3            HEATER_MIN:2 (25)
//  4            (0)
//  5            FILTER_MAX:2 (27)
//  6            (0)
//  7            WIFI_MODE 1 station, 2 ap, 3 both (2)
//  8            WIFI_SSID:k
//  8+k          255
//  8+k+1        WIFI_PASSWORD:l
//  8+k+1+l      255
//  8+k+1+l+1    BOARD_NAME:m
//  8+k+1+l+1+m  255

// ดึงค่าตัวแปรพื้นฐานจากหน่วยความจำ
int k = 0;
int l = 0;
int m = 0;

void getBased() {
  //  if (EEPROM.read(0) == 255)
  resetBased();
  
  HEATER_MODE = EEPROM.read(1);
  FILTER_MODE = EEPROM.read(2);
  HEATER_MIN = EEPROM.read(3) + EEPROM.read(4) / 100;
  FILTER_MAX = EEPROM.read(5) + EEPROM.read(6) / 100;
  WIFI_MODE = EEPROM.read(7);
  
  while (true) {
    int i = EEPROM.read(8 + k);
    if (i == 255)
    break;
    WIFI_SSID += (char) i;
    k++;
  }
  
  while (true) {
    int i = EEPROM.read(8 + k + 1 + l);
    if (i == 255)
    break;
    WIFI_PASSWORD += (char) i;
    l++;
  }
  
  while (true) {
    int i = EEPROM.read(8 + k + 1 + l + 1 + m);
    if (i == 255)
    break;
    BOARD_NAME += (char) i;
    m++;
  }
  
  Serial.println(HEATER_MODE);
  Serial.println(FILTER_MODE);
  Serial.println(HEATER_MIN);
  Serial.println(FILTER_MAX);
  Serial.println(WIFI_MODE);
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PASSWORD);
  Serial.println(BOARD_NAME);
}

void resetBased() {
  EEPROM.write(0, 0);
  EEPROM.write(1, 0);
  EEPROM.write(2, 0);
  EEPROM.write(3, 25);
  EEPROM.write(4, 0);
  EEPROM.write(5, 27);
  EEPROM.write(6, 0);
  EEPROM.write(7, 2);
  
  EEPROM.write(8, 65);
  EEPROM.write(9, 65);
  EEPROM.write(10, 65);
  EEPROM.write(11, 255);
  EEPROM.write(12, 65);
  EEPROM.write(13, 255);
  
  for (int aa = 0; aa < 26; aa++) {
    EEPROM.write(14 + aa, 65 + aa);
  }
//  EEPROM.write(15, 255);
  
  //  EEPROM.write(0, 255);
  //  EEPROM.write(1, 255);
  //  EEPROM.write(2, 255);
  //  EEPROM.write(3, 255);
  //  EEPROM.write(4, 255);
  //  EEPROM.write(5, 255);
  //  EEPROM.write(6, 255);
  //  EEPROM.write(7, 255);
  //  
  //  EEPROM.write(8, 255);
  //  EEPROM.write(9, 255);
  //  EEPROM.write(10, 255);
  //  EEPROM.write(11, 255);
  //  EEPROM.write(12, 255);
  //  EEPROM.write(13, 255);
  //  EEPROM.write(14, 255);
  //  EEPROM.write(15, 255);
  
}

void setBased0To7(int address, int value) {
  EEPROM.write(address, value);
}

void setBasedWifiSSID(String ssid) {
  for (int i = 8+k+1+l+1 + ssid.length(); i > 8+k+1+l+1; i--) {
    EEPROM.write(i, 8+k+1+l+1);
  }
}



int frameSec = 100;
int frame = 0;
int r = 1; // tmp
int lcdMode = 0; // 0 = home, 1 = menu


int cursorMenu = 0;
int cursorMenuSub = 0;






int sizeMenu = 6;
String menu[] = {
  "1 Board Info.",
  "2 Feed",
  "3 Temperature",
  "4 Wifi",
  "5 Factory Reset",
  "6 Reboot"
};

int sizeMenuSub[] = {1, 1, 2, 7, 1, 1};
String menuSub[][7] = {
  {"1.1 Board Name"},
  {"2.1 Feed Now !"},
  {"3.1 Set Heater Min", "3.2 Set Filter Max"},
  {"4.1 Internet Info.", "4.2 Intranet Info", "4.3 Set Mode", "4.4 Set SSID", "4.5 Password", "4.6 Set Static IP", "4.7 Set Port"},
  {"5.1 Confirm !"},
  {"6.1 Confirm !"}
};




// ฟังก์ชั่นเริ่มต้นที่ arduino กำหนด
// คือเรียกใช้แค่ครั้งเดียวตอนเปิดบอร์ด ตรงกันข้ามกับ loop()
void setup() {
  
  // กำหนดความถี่ช่วงข้อมูลสำหรับรับเข้า ส่งออกข้อมูลผ่านหน้าจอ serial monitor
  Serial.begin(9600);
  // กำหนดความถี่ช่วงข้อมูลสำหรับรับเข้า ส่งออกข้อมูลโมดูลไวไฟ
  Serial1.begin(115200);
  Serial.println("Booting...");
  
  // เรีกใช้ฟังก์ชั่นที่ชื่อขึ้นต้นด้วย set คือ กำหนดค่าพื้นฐานให้แต่ละโมดูล
  
  getBased();
  
  setLCD();
  
  //    setHTTP("batmaster_wifi", "0817371393");
  //    
  //    setTemp();
  
  setRelays();
}

// อีกฟังก์ชั่นที่ arduino กำหนดให้ต้องมี
void loop() {
  //  digitalWrite(RELAY_4, LOW);
  //  digitalWrite(RELAY_1, HIGH);
  //  Serial.println(1);
  //  delay(500);
  //  digitalWrite(RELAY_1, LOW);
  //  digitalWrite(RELAY_2, HIGH);
  //  Serial.println(2);
  //  delay(500);
  //  digitalWrite(RELAY_2, LOW);
  //  digitalWrite(RELAY_3, HIGH);
  //  Serial.println(3);
  //  delay(500);
  //  digitalWrite(RELAY_3, LOW);
  //  digitalWrite(RELAY_4, HIGH);
  //  Serial.println(4);
  //  delay(500);
  
  //   checkTemp(0);
  //    checkTemp(1);
  // checkTemp(2);
  
  //    checkHTTP();
  
  //    ping();
  
  showLCD();
  checkKeypad();
  
  
}

String tmp;
int indexEdit = 0;
int en = 0;


int lastButton = -1;
int indexInButton = -1;



int runner = 0;
char server[] = "http://188.166.180.204:8888/arduinoping.php";

// เอาไว้ให้บอร์ดติดต่อกับ server ทุกๆช่วงเวลา
/*void ping() {
if (runner++ == 30000) {
Serial.println("Connecting to Server...");
//     // Make a HTTP request:
//     Serial1.print("GET http://188.166.180.204:8888/arduinoping.php?bid=A0000&ip=118.175.112.32&port=8080");
//     Serial1.println(" HTTP/1.1");
//     Serial1.println("Host: batmastertest.com");
//     Serial1.println();

sendHTTP("188.166.180.204", 8888, "/arduinoping.php?bid=A0001&port=8080");
}
}*/


int blink = 0;
char chtmp = 'A';

void showLCD() {
  if (lcdMode == 0) {
    if (frame % frameSec == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("28.32 ");
      lcd.print(r++); 
    }
    
    if (frame < 1*frameSec) {
      lcd.setCursor(0, 1); // ไปที่ตัวอักษรที่ 1 แถวที่ 2 (col, row)
      lcd.print("Heater: ");
      lcd.print(HEATER_MIN); 
      lcd.print("  A"); 
    }
    else if (frame < 2*frameSec) {
      lcd.setCursor(0, 1);
      lcd.print("Filter: ");
      lcd.print(FILTER_MAX); 
      lcd.print("  A");
    }
    else {
      frame = 0;
    }
    frame++;
  }
  else if (lcdMode == 1) {
    lcd.setCursor(0, 0);
    lcd.print(menu[cursorMenu]);
    lcd.setCursor(0, 1);
    lcd.print("* OK      # BACK");
  }
  else if (lcdMode == 2) {
    lcd.setCursor(0, 0);
    lcd.print(menuSub[cursorMenu][cursorMenuSub]);
    lcd.setCursor(0, 1);
    lcd.print("* OK      # BACK");
  }
  else if (lcdMode == 11) {
    if (frame < 0.5*frameSec) {
      blink = 0;
    }
    else if (frame < 1.0*frameSec) {
      blink = 1;
    }
    else {
      frame = 0;
    }
    
    String x = tmp.substring(indexEdit, en);
    
    if (blink == 0) {
      if (tmp[indexEdit] != '~')
        chtmp = tmp[indexEdit];
      tmp[indexEdit] = '~';
      x[0] = '~';
      blink = 0;
    }
    else {
      tmp[indexEdit] = chtmp;
      x[0] = chtmp;
    }
    lcd.setCursor(0, 0);
    lcd.print(x);
    lcd.setCursor(0, 1);
    lcd.print("< A  B >   del D");

    frame++;
  }
}


String b1 = "._+-=!@#$%^&*()";
String b2 = "abcABC";
String b3 = "defDEF";
String b4 = "ghIGHI";
String b5 = "jklJKL";
String b6 = "mnoMNO";
String b7 = "pqrsPQRS";
String b8 = "tuvTUV";
String b9 = "wxyzWXYZ";
String b0 = "1234567890 ";

void checkKeypad() {
  char c = keypad.getKey();
  if (c != NO_KEY) {
    lcd.clear();
    if (lcdMode == 0) {
      if (c == '*') {
        lcdMode = 1;
      }
    }
    else if (lcdMode == 1) {
      if (c == '*') {
        lcdMode = 2;
      }
      else if (c == '#') {
        lcdMode = 0;
        cursorMenu = 0;
      }
      else if (c == 'A') {
        if (cursorMenu == 0) {
          cursorMenu = sizeMenu;
        }
        
        cursorMenu -= 1;
      }
      else if (c == 'B') {
        if (cursorMenu == sizeMenu - 1) {
          cursorMenu = -1;
        }
        cursorMenu += 1;
      }
    }
    else if (lcdMode == 2) {
      if (c == '#') {
        lcdMode = 1;
        cursorMenuSub = 0;
      }
      else if (c == 'A') {
        if (cursorMenuSub == 0) {
          cursorMenuSub = sizeMenuSub[cursorMenu];
        }
        
        cursorMenuSub -= 1;
      }
      else if (c == 'B') {
        if (cursorMenuSub == sizeMenuSub[cursorMenu] - 1) {
          cursorMenuSub = -1;
        }
        cursorMenuSub += 1;
      }
      else if (c == '*') {
        if (cursorMenu == 0 && cursorMenu == 0) {
          tmp = BOARD_NAME;

          if (tmp.length() > 16) {
            en = 16;
          }
          else {
            en = tmp.length();
          }
          
          lcdMode = 11;
        }
      }
    }
    else if (lcdMode == 11) {
      if (c == '*') {
        if (blink == 0)
           tmp[indexEdit] = chtmp;
        BOARD_NAME = tmp;
        tmp = "";
        // setBOARD_NAME();
        Serial.println(BOARD_NAME);
        lastButton = -1;
        indexInButton = -1;
        indexEdit = 0;
        en = 0;
        blink = 0;
        frame = 0;
        lcdMode = 2;
      }
      else if (c == '#') {
        tmp = "";
        lastButton = -1;
        indexInButton = -1;
        indexEdit = 0;
        en = 0;
        blink = 0;
        frame = 0;
        lcdMode = 2;
      }
      else if (c == '1') {
        if (lastButton != 1) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b1.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b1.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b1[indexInButton];
        lastButton = 1;
      }
      else if (c == '2') {
        if (lastButton != 2) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b2.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b2.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b2[indexInButton];
        lastButton = 2;
      }
      else if (c == '3') {
        if (lastButton != 3) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b3.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b3.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b3[indexInButton];
        lastButton = 3;
      }
      else if (c == '4') {
        if (lastButton != 4) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b4.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b4.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b4[indexInButton];
        lastButton = 4;
      }
      else if (c == '5') {
        if (lastButton != 5) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b5.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b5.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b5[indexInButton];
        lastButton = 5;
      }
      else if (c == '6') {
        if (lastButton != 6) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b6.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b6.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b6[indexInButton];
        lastButton = 6;
      }
      else if (c == '7') {
        if (lastButton != 7) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b7.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b7.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b7[indexInButton];
        lastButton = 7;
      }
      else if (c == '8') {
        if (lastButton != 8) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b8.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b8.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b8[indexInButton];
        lastButton = 8;
      }
      else if (c == '9') {
        if (lastButton != 9) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b9.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b9.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b9[indexInButton];
        lastButton = 9;
      }
      else if (c == '0') {
        if (lastButton != 0) {
          indexInButton = 0;
        }
        else {
          if (indexInButton == b0.length() - 1) {
            indexInButton = 0;
          }
          else {
            if (indexInButton + 1 < b0.length()) {
              indexInButton++;
            }
            else {
              indexInButton = 0;
            }
          }
        }
        tmp[indexEdit] = b2[indexInButton];
        lastButton = 0;
      }
      else if (c == 'A') {
         tmp[indexEdit] = chtmp;
         
         frame = 0;
           
          if (indexEdit + 1 < tmp.length()) {
            indexEdit++;

            if (indexEdit + 16 > tmp.length()) {
              en = tmp.length();
            }
            else {
               en = indexEdit + 16;
            }

            indexInButton = -1;
          }
        
      }
      else if (c == 'B') {
         tmp[indexEdit] = chtmp;
         
         frame = 0;
         
        if (indexEdit > 0) {
          indexEdit--;

          if (tmp.length() - indexEdit > 16) {
              en = indexEdit + 16;
            }
            else {
               en = tmp.length();
            }

            indexInButton = -1;
        }

      }
      else if (c == 'B') {
         tmp[indexEdit] = chtmp;
         
         frame = 0;
         
        if (indexEdit > 0) {
          indexEdit--;

          if (tmp.length() - indexEdit > 16) {
              en = indexEdit + 16;
            }
            else {
               en = tmp.length();
            }

            indexInButton = -1;
        }

      } else if (c == 'D') {
         Serial.print(tmp);
         Serial.print(" ");
         Serial.print(indexEdit);
         Serial.print(" ");
         Serial.print(tmp[indexEdit]);
         
         tmp.remove(indexEdit, 1);
         chtmp = tmp[indexEdit];
         if (tmp.length() < indexEdit) {
           indexEdit--;
         }

         lastButton = -1;
         indexInButton = -1;

         Serial.print(" ");
         Serial.print(tmp);
         Serial.print(" ");
         Serial.println(indexEdit);

         
      }
    }
    
  }
}


// ฟังก์ชั่นเซตค่าพื้นฐานโมดูล LCD
void setLCD() {
  // LCD เป็นแบบ 2 แถว 16 ตัวอักษร
  lcd.begin(16, 2);
  
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home(); // ไปที่ตัวอักษรที่ 0 แถวที่ 1
}

// ฟังก์ชั่นเซตค่าพื้นฐานโมดูล relay
// กำหนด pin และ
// ทำให้รีเลย์ทั้ง4 ปิดอยู่ก่อน
void setRelays() {
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
}

// ฟังกั่นเซตค่าพื้นฐานโมดูลอุณหภูมิ
// ไม่ต้องกำหนด pin เพราะเซ็ตไว้ข้างบนแล้ว
// #ไม่ต้องถามว่าทำไมไม่เหมือนกับการเซตโมดูลอื่น เพราะขึ้นอยู่กับไลบรารี่ทั้งนั้น
void setTemp() {
  sensors.begin();
}

// เอาไว้เรียกค่าจากตัววัดอุณหภูมิให้แสดงผลทาง serial monitor
// และตรวจสอบเงื่อนไขการเปิดปิดฮีตเตอร์ เครื่องกรองในนี้
void checkTemp(int index) {
  sensors.requestTemperatures();
  
  Serial.print("Temperature for Device ");
  Serial.print(index);
  Serial.print(" is: ");
  double a = sensors.getTempCByIndex(index);
  Serial.println(a);
  
  // ควบคุมฮีต
  if (heaterAuto) {
    if (a > 25) {
      digitalWrite(RELAY_1, HIGH);
    }
    else {
      digitalWrite(RELAY_1, LOW);
    }
  }
  
  
  // กรอง
  if (filterAuto) {
    if (a < 27) {
      digitalWrite(RELAY_2, HIGH);
    }
    else {
      digitalWrite(RELAY_2, LOW);
    }
  }
  
}

// ตั้งค่าโมดูลไวไฟ ยุ่งยางมาก
// #ไม่ต้องถามมาก เอามาจากเน็ต
void setHTTP(String ssid, String pass) {
  Serial.println("1=========================");
  sendCommand("AT+RST\r\n",2000,DEBUG); // reset module
  delay(2000);
  Serial.println("2=========================");
  sendCommand("AT+CWMODE=2\r\n",1000,DEBUG); // configure as access point
  delay(5000);
  Serial.println("3=========================");
  sendCommand("AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"\r\n",3000,DEBUG);
  delay(4000);
  Serial.println("4=========================");
  delay(1000);
  Serial.println("5=========================");
  sendCommand("AT+CIFSR\r\n",1000,DEBUG); // get ip address
  delay(1000);
  Serial.println("6=========================");
  sendCommand("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
  delay(1000);
  Serial.println("7=========================");
  sendCommand("AT+CIPSERVER=1,8080\r\n",1000,DEBUG); // turn on server on port 80
  delay(1000);
  Serial.println("8=========================");
  
  
  Serial.println("Server Ready");
}

/*void sendHTTP(String ipdomain, int port, String param) {
String startcommand = "AT+CIPSTART=4,\"TCP\",\"" + ipdomain + "\", \"8888\"";
Serial1.println(startcommand);
Serial.println(startcommand);

if (Serial1.find("Error")) {
Serial.println("error on start");
return;
}

String sendcommand = "GET http://"+ ipdomain + ":" + port + param + " HTTP/1.1\r\n\r\n\r\n";//works for most cases

Serial.print(sendcommand);

sendCIPData(4, sendcommand);

Serial1.print("AT+CIPSEND=4,");
Serial1.println(sendcommand.length());

//debug the command
Serial.print("AT+CIPSEND=4,");
Serial.println(sendcommand.length());

//delay(5000);
if(Serial1.find(">"))
{
Serial.println(">");
}else
{
Serial1.println("AT+CIPCLOSE");
Serial.println("connect timeout");
delay(1000);
return;
}

//Serial.print(getcommand);
Serial1.print(sendcommand);
}*/


// เรียกค่าจากโมดูลไวไฟ แล้วเช็คว่ามันส่งอะไรมา
void checkHTTP() {
  if(Serial1.available()) // check if the esp is sending a message
  {
    if(Serial1.find("+IPD,"))
    {
      delay(1000); // wait for the serial buffer to fill up (read all the serial data)
      // get the connection id so that we can then disconnect
      int connectionId = Serial1.read()-48; // subtract 48 because the read() function returns
      // the ASCII decimal value and 0 (the first decimal number) starts at 48
      
      Serial1.find("relay=");
      int r1 = (Serial1.read()-48);
      Serial.println(r1);
      if (r1 == 0 || r1 == 1 || r1 == -6) {
        //  เอาไว้เทสการสั่งเปิดปิดรีเลย์แต่ละตัว
        // ในแอป ไม่ได้ใช้นะ ใช้อันข้างล่างแทน
        int r2 = (Serial1.read()-48);
        int r3 = (Serial1.read()-48);
        int r4 = (Serial1.read()-48);
        
        // -6 = *
        // A
        if (r1 != -6) {
          digitalWrite(RELAY_1, r1);
        }
        if (r2 != -6) {
          digitalWrite(RELAY_2, r2);
        }
        if (r3 != -6) {
          digitalWrite(RELAY_3, r3);
        }
        if (r4 != -6) {
          digitalWrite(RELAY_4, r4);
        }
        String res = digitalRead(RELAY_1) == LOW ? "0" : "1";
        res += digitalRead(RELAY_2) == LOW ? "0" : "1";
        res += digitalRead(RELAY_3) == LOW ? "0" : "1";
        res += digitalRead(RELAY_4) == LOW ? "0" : "1";
        sendHTTPResponse(connectionId, res);
      }
      else if (r1 == 17) {
        // 17 = A
        // ส่งค่าสถานะ โหมด heater, filter และ relay 1, 2
        String res = heaterAuto ? "1" : "0";
        res += filterAuto ? "1" : "0";
        res += digitalRead(RELAY_1) == LOW ? "0" : "1";
        res += digitalRead(RELAY_2) == LOW ? "0" : "1";
        
        res += bid;
        res += "-";
        res += sensors.getTempCByIndex(1);
        
        sendHTTPResponse(connectionId, res);
      }
      else if (r1 == 18) {
        // 18 = B
        // B0011
        // ตั้งค่า โหมด heater, filter และ relay 1, 2 และ
        // ส่งค่าสถานะ โหมด heater, filter และ relay 1, 2
        int r2 = (Serial1.read()-48);
        int r3 = (Serial1.read()-48);
        int r4 = (Serial1.read()-48);
        int r5 = (Serial1.read()-48);
        
        if (r2 == 1) {
          heaterAuto = true;
        }
        else {
          heaterAuto = false;
          digitalWrite(RELAY_1, r4);
        }
        
        if (r3 == 1) {
          filterAuto = true;
        }
        else {
          filterAuto = false;
          digitalWrite(RELAY_2, r5);
        }
        
        String res = heaterAuto ? "1" : "0";
        res += filterAuto ? "1" : "0";
        res += digitalRead(RELAY_1) == LOW ? "0" : "1";
        res += digitalRead(RELAY_2) == LOW ? "0" : "1";
        
        res += bid;
        res += "-";
        res += sensors.getTempCByIndex(1);
        
        sendHTTPResponse(connectionId, res);
      }
      else if (r1 == 19) {
        // 19 = C
        // C
        // ให้อาหาร
        int motorTime = 4000;
        sendHTTPResponse(connectionId, String(motorTime));
        digitalWrite(RELAY_3, LOW);
        delay(motorTime);
        digitalWrite(RELAY_3, HIGH);
      }
    }
  }
}






// =================== ข้างล่างนี้ ก็อบมา มันมากับโค้ดไวไฟ =================== //




/*
* Name: sendHTTPResponse
* Description: Function that sends HTTP 200, HTML UTF-8 response
*/
void sendHTTPResponse(int connectionId, String content)
{
  
  // build HTTP response
  String httpResponse;
  String httpHeader;
  // HTTP Header
  httpHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n";
  httpHeader += "Content-Length: ";
  httpHeader += content.length();
  httpHeader += "\r\n";
  httpHeader +="Connection: close\r\n\r\n";
  httpResponse = httpHeader + content + " "; // There is a bug in this code: the last character of "content" is not sent, I cheated by adding this extra space
  sendCIPData(connectionId,httpResponse);
}

/*
* Name: sendCIPDATA
* Description: sends a CIPSEND=<connectionId>,<data> command
*
*/
void sendCIPData(int connectionId, String data)
{
  String cipSend = "AT+CIPSEND=";
  cipSend += connectionId;
  cipSend += ",";
  cipSend +=data.length();
  cipSend +="\r\n";
  sendCommand(cipSend,1000,DEBUG);
  sendData(data,1000,DEBUG);
}

/*
* Name: sendCommand
* Description: Function used to send data to Serial1.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the Serial1 (if there is a reponse)
*/
String sendCommand(String command, const int timeout, boolean debug)
{
  String response = "";
  
  Serial1.print(command); // send the read character to the Serial1
  
  long int time = millis();
  
  while( (time+timeout) > millis())
  {
    while(Serial1.available())
    {
      
      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response+=c;
    }
  }
  
  if(debug)
  {
    Serial.print(response);
  }
  
  return response;
}

/*
* Name: sendData
* Description: Function used to send data to Serial1.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the Serial1 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  
  int dataSize = command.length();
  char data[dataSize];
  command.toCharArray(data,dataSize);
  
  Serial1.write(data,dataSize); // send the read character to the Serial1
  if(debug)
  {
    Serial.println("\r\nvvvvvv HTTP Response From Arduino vvvvvv");
    Serial.write(data,dataSize);
    Serial.println("\r\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
  }
  
  long int time = millis();
  
  while( (time+timeout) > millis())
  {
    while(Serial1.available())
    {
      
      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response+=c;
    }
  }
  
  if(debug)
  {
    Serial.print(response);
  }
  
  return response;
}


