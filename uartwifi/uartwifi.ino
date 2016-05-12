// DON'T FORGET TO SET HARDWARE SERIAL BUFFER SIZE TO 128

// สำหรับโค้ด reboot
#include <avr/wdt.h>
#include <stdio.h>


// นำเข้าไลบรารี่สำหรับพวกวัดอุณหภูมิ
#include <OneWire.h>
#include <DallasTemperature.h>


// ที่เป็นแนวๆ ขื่อตัวแปร ตามด้วยตัวเลขแบบนี้ เดาเลยน่าจะเกี่ยวกับกำหนด pin
#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define RELAY_1 28
#define RELAY_2 26
#define RELAY_3 24
#define RELAY_4 22


// ตัวแกรเก็บว่าจะให้แสดงผลทุกอย่างที่เข้าออกบอร์ดหรือไม่ #มันแถมมากับโค้ด wifi
#define DEBUG true


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
//กำหนดตัวอักษรบนปุ่มแ้ำแย
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
String STATIC_IP;
String PORT;

// ตัวแปรสำหรับแสดง 41 42
String INTERNET_IP = "-";
String INTRANET_IP = "-";

String INTERNET_MAC = "-";
String INTRANET_MAC = "-";

// ตัวแปรสำหรับอุณหภูมิ
double TEMP = 0;

#include <EEPROM.h>
// EEPROM
//  0               HAS_SET 0 set, 255 not set (255)
//  1               HEATER_MODE 0 auto, 1 manual (0)
//  2               FILTER_MODE 0 auto, 1 manual (0)
//  3               HEATER_MIN:2 (25)
//  4               (0)
//  5               FILTER_MAX:2 (27)
//  6               (0)
//  7               WIFI_MODE 1 station, 2 access point, 3 both (2)
//  8               WIFI_SSID:k
//  8+k             255
//  8+k+1           WIFI_PASSWORD:l
//  8+k+1+l         255
//  8+k+1+l+1       BOARD_NAME:m
//  8+k+1+l+1+m     255
//  8+k+1+l+1+m+1   STATIC_IP:4
//  8+k+1+l+1+m+1+4 PORT:2
//  8+k+1+l+1+m+1+4+2


// ดึงค่าตัวแปรพื้นฐานจากหน่วยความจำ
void getBased() {
    if (EEPROM.read(0) == 255)
    resetBased();

    HEATER_MODE = EEPROM.read(1);
    FILTER_MODE = EEPROM.read(2);
    HEATER_MIN = EEPROM.read(3) + (EEPROM.read(4) / 100.0);
    FILTER_MAX = EEPROM.read(5) + (EEPROM.read(6) / 100.0);
    WIFI_MODE = EEPROM.read(7);

    int k = 0;
    int l = 0;
    int m = 0;

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

    int ip1 = EEPROM.read(8 + k + 1 + l + 1 + m + 1);
    int ip2 = EEPROM.read(8 + k + 1 + l + 1 + m + 2);
    int ip3 = EEPROM.read(8 + k + 1 + l + 1 + m + 3);
    int ip4 = EEPROM.read(8 + k + 1 + l + 1 + m + 4);
    STATIC_IP = (ip1 < 10 ? "00" + String(ip1) : (ip1 < 100 ? "0" + String(ip1) : String(ip1))) + "."
    + (ip2 < 10 ? "00" + String(ip2) : (ip2 < 100 ? "0" + String(ip2) : String(ip2))) + "."
    + (ip3 < 10 ? "00" + String(ip3) : (ip3 < 100 ? "0" + String(ip3) : String(ip3))) + "."
    + (ip4 < 10 ? "00" + String(ip4) : (ip4 < 100 ? "0" + String(ip4) : String(ip4)));

    int z = EEPROM.read(8 + k + 1 + l + 1 + m + 5);
    long p = 0;
    for (int i = 0; i < z; i++) {
        p += 256;
    }
    p += EEPROM.read(8 + k + 1 + l + 1 + m + 6);
    PORT = (p < 10 ? "0000" + String(p) : (p < 100 ? "000" + String(p) : (p < 1000 ? "00" + String(p) : (p < 10000 ? "0" + String(p) : String(p)))));

    Serial.println("== read ==");
    Serial.println(HEATER_MODE);
    Serial.println(FILTER_MODE);
    Serial.println(HEATER_MIN);
    Serial.println(FILTER_MAX);
    Serial.println(WIFI_MODE);
    Serial.println(WIFI_SSID);
    Serial.println(WIFI_PASSWORD);
    Serial.println(BOARD_NAME);
    Serial.println(STATIC_IP);
    Serial.println(PORT);
    Serial.println("== end ==");
    // setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);
}

void clearEEPROM() {
    for (int z = 0; z < 512; z++) {
        EEPROM.write(z, 255);
    }
}

void setEEPROM(int has_set, int heater_mode, int filter_mode, double heater_min, double filter_max, int wifi_mode, String wifi_ssid, String wifi_password, String board_name, String static_ip, String port) {
    EEPROM.write(0, has_set);
    EEPROM.write(1, heater_mode);
    EEPROM.write(2, filter_mode);

    int a = (int) heater_min;
    int b = ((int) heater_min * 100) % 100;
    EEPROM.write(3, a);
    EEPROM.write(4, b);

    a = (int) filter_max;
    b = ((int) filter_max * 100) % 100;
    EEPROM.write(5, a);
    EEPROM.write(6, b);

    EEPROM.write(7, wifi_mode);

    int k = 0;
    int l = 0;
    int m = 0;

    for (k = 0; k < wifi_ssid.length(); k++) {
        EEPROM.write(8 + k, (int) wifi_ssid[k]);
    }
    EEPROM.write(8 + k, 255);

    for (l = 0; l < wifi_password.length(); l++) {
        EEPROM.write(8 + k + 1 + l, (int) wifi_password[l]);
    }
    EEPROM.write(8 + k + 1 + l, 255);

    for (m = 0; m < board_name.length(); m++) {
        EEPROM.write(8 + k + 1 + l + 1 + m, (int) board_name[m]);
    }
    EEPROM.write(8 + k + 1 + l + 1 + m, 255);

    EEPROM.write(8 + k + 1 + l + 1 + m + 1, splitString(static_ip, '.', 0).toInt());
    EEPROM.write(8 + k + 1 + l + 1 + m + 2, splitString(static_ip, '.', 1).toInt());
    EEPROM.write(8 + k + 1 + l + 1 + m + 3, splitString(static_ip, '.', 2).toInt());
    EEPROM.write(8 + k + 1 + l + 1 + m + 4, splitString(static_ip, '.', 3).toInt());

    if (port.toInt() > 65535) {
        port = "08081";
    }

    int p1 = port.toInt() / 256;
    int p2 = port.toInt() % 256;

    EEPROM.write(8 + k + 1 + l + 1 + m + 5, p1);
    EEPROM.write(8 + k + 1 + l + 1 + m + 6, p2);

    int z = 8 + k + 1 + l + 1 + m + 6 + 1;
    for (; z < 512; z++) {
        EEPROM.write(z, 255);
    }
}

void resetBased() {
    lcd.setCursor(0, 0);
    lcd.print("resetting...");

    setEEPROM(0, 0, 0, 25.00, 30.00, 3, "SSID", "PASS", "default", "0.0.0.0", "8081");

    delay(500);
    lcd.setCursor(0, 0);
    lcd.print("done");

    getBased();
}


// ฟังก์ชั่นเซตค่าพื้นฐานโมดูล LCD
void setLCD() {
    // LCD เป็นแบบ 2 แถว 16 ตัวอักษร
    lcd.begin(16, 2);
    // สำหรับ 4 ขา
    lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    lcd.setBacklight(HIGH);
    lcd.home(); // ไปที่ตัวอักษรที่ 0 แถวที่ 1
}

// ส่วนโปรแกรมจอ
int frameSec = 50;
int frame = 0;
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

int sizeMenuSub[] = {1, 1, 4, 7, 1, 1};
String menuSub[][7] = {
    {"1.1 Board Name"},
    {"2.1 Feed Now !"},
    {"3.1 Heater Min", "3.2 Filter Max", "3.3 Heater Mode", "3.4 Filter Mode"},
    {"4.1 Internet Info", "4.2 Intranet Info", "4.3 Set Mode", "4.4 Set SSID", "4.5 Password", "4.6 Set Static IP", "4.7 Set Port"},
    {"5.1 Confirm !"},
    {"6.1 Confirm !"}
};


String tmp = "";
int indexEdit = 0;
int en = 0;

int lastButton = -1;
int indexInButton = -1;

int blink = 0;
char chtmp = '_';

void showLCD() {
    if (lcdMode == 0) {
        if (frame % frameSec == 0) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(TEMP);
            
            checkTemp(0);
        }
        
        if (frame < 1*frameSec) {
            lcd.setCursor(0, 1); // ไปที่ตัวอักษรที่ 1 แถวที่ 2 (col, row)
            lcd.print("Heater: ");
            lcd.print(HEATER_MIN);
            lcd.print("  ");
            lcd.print(HEATER_MODE == 0 ? "A" : "M");
        }
        else if (frame < 2*frameSec) {
            lcd.setCursor(0, 1);
            lcd.print("Filter: ");
            lcd.print(FILTER_MAX);
            lcd.print("  ");
            lcd.print(FILTER_MODE == 0 ? "A" : "M");
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
        if (frame < 0.25*frameSec) {
            blink = 0;
        }
        else if (frame < 0.5*frameSec) {
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
    else if (lcdMode == 21) {
      lcd.setCursor(0, 0);
      lcd.print("feeding");
    }
    else if (lcdMode == 31 || lcdMode == 32) {
        if (frame < 0.25*frameSec) {
            blink = 0;
        }
        else if (frame < 0.5*frameSec) {
            blink = 1;
        }
        else {
            frame = 0;
        }

        if (blink == 0) {
            if (tmp[indexEdit] != '~')
            chtmp = tmp[indexEdit];
            tmp[indexEdit] = '~';
        }
        else {
            tmp[indexEdit] = chtmp;
        }
        lcd.setCursor(0, 0);
        lcd.print(tmp);
        lcd.setCursor(0, 1);
        lcd.print("< A  B >        ");

        frame++;
    }
    else if (lcdMode == 33 || lcdMode == 34) {
        if (frame < 0.25*frameSec) {
            blink = 0;
        }
        else if (frame < 0.5*frameSec) {
            blink = 1;
        }
        else {
            frame = 0;
        }

        if (blink == 0) {
            if (tmp[indexEdit] != '~')
            chtmp = tmp[indexEdit];
            tmp[indexEdit] = '~';
        }
        else {
            tmp[indexEdit] = chtmp;
        }
        lcd.setCursor(0, 0);
        lcd.print(tmp);
        lcd.setCursor(0, 1);
        lcd.print("0 Auto  1 Manual");

        frame++;
    }
    else if (lcdMode == 41) {
        lcd.setCursor(0, 0);
        String ipp = INTERNET_IP + ":" + PORT;
        lcd.print(ipp);
        lcd.setCursor(0, 1);
        lcd.print(INTERNET_MAC);
    }
    else if (lcdMode == 42) {
        lcd.setCursor(0, 0);
        String ipp = INTRANET_IP + ":" + PORT;
        lcd.print(ipp);
        lcd.setCursor(0, 1);
        lcd.print(INTRANET_MAC);
    }
    else if (lcdMode == 43) {
        if (frame < 0.25*frameSec) {
            blink = 0;
        }
        else if (frame < 0.5*frameSec) {
            blink = 1;
        }
        else {
            frame = 0;
        }

        if (blink == 0) {
            if (tmp[indexEdit] != '~')
            chtmp = tmp[indexEdit];
            tmp[indexEdit] = '~';
        }
        else {
            tmp[indexEdit] = chtmp;
        }
        lcd.setCursor(0, 0);
        lcd.print(tmp);
        lcd.setCursor(0, 1);
        lcd.print("1Sta  2AP  3Both");

        frame++;
    }
    else if (lcdMode == 44 || lcdMode == 45) {
        if (frame < 0.25*frameSec) {
            blink = 0;
        }
        else if (frame < 0.5*frameSec) {
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
    else if (lcdMode == 46) {
        if (frame < 0.25*frameSec) {
            blink = 0;
        }
        else if (frame < 0.5*frameSec) {
            blink = 1;
        }
        else {
            frame = 0;
        }

        if (blink == 0) {
            if (tmp[indexEdit] != '~')
            chtmp = tmp[indexEdit];
            tmp[indexEdit] = '~';
        }
        else {
            tmp[indexEdit] = chtmp;
        }
        lcd.setCursor(0, 0);
        lcd.print(tmp);
        lcd.setCursor(0, 1);
        lcd.print("< A  B >   del D");

        frame++;
    }
    else if (lcdMode == 47) {
        if (frame < 0.25*frameSec) {
            blink = 0;
        }
        else if (frame < 0.5*frameSec) {
            blink = 1;
        }
        else {
            frame = 0;
        }

        if (blink == 0) {
            if (tmp[indexEdit] != '~')
            chtmp = tmp[indexEdit];
            tmp[indexEdit] = '~';
        }
        else {
            tmp[indexEdit] = chtmp;
        }
        lcd.setCursor(0, 0);
        lcd.print(tmp);
        lcd.setCursor(0, 1);
        lcd.print("< A  B >   del D");

        frame++;
    }
    else if (lcdMode == 51) {

        resetBased();
        delay(500);
        lcd.setCursor(0, 0);
        lcd.print("rebooting...");
        wdt_enable(WDTO_1S);

        while (true) {

        }
    }
    else if (lcdMode == 61) {
        lcd.setCursor(0, 0);
        lcd.print("rebooting...");
        wdt_enable(WDTO_2S);

        while (true) {

        }
    }
}



// นำเข้าไลบรารี่ Servo
#include  <Servo.h>
Servo myservo;

void setServo() {
    myservo.attach(13);
}

// สั่งหมุน servo
void feed() {
  Serial.println(myservo.read());
  
  myservo.write(0);
  Serial.println(myservo.read());
  delay(1000);
  
  myservo.write(180);
  Serial.println(myservo.read());
  delay(3000);

  myservo.write(0);
  Serial.println(myservo.read());
  delay(500);
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

    TEMP = sensors.getTempCByIndex(index);
    double workLoad = (FILTER_MAX - HEATER_MIN) / 2 / 2;
    // ควบคุมฮีต
    if (HEATER_MODE == 0) {
      
        if (TEMP < HEATER_MIN || TEMP < HEATER_MIN + workLoad) {
            digitalWrite(RELAY_1, LOW);
        }
        else {
            digitalWrite(RELAY_1, HIGH);
        }
    }

    // กรอง
    if (FILTER_MODE == 0) {
        if (TEMP > FILTER_MAX || TEMP > FILTER_MAX - workLoad) {
            digitalWrite(RELAY_2, LOW);
        }
        else {
            digitalWrite(RELAY_2, HIGH);
        }
    }
}


// ตั้งค่าโมดูลไวไฟ ยุ่งยางมาก
// #ไม่ต้องถามมาก เอามาจากเน็ต
void setHTTP(String ssid, String pass, int WIFI_MODE_TEMP = 0) {
    lcd.setCursor(0, 0);
    lcd.print("connecting...");
    lcd.setCursor(0, 1);
    lcd.print("mode: ");
    lcd.print(WIFI_MODE);

    Serial.println("1=========================");
    sendCommand("AT+RST\r\n",2000,DEBUG); // reset module
    delay(2000);
    Serial.println("2=========================");
    String CWMODE = "AT+CWMODE=";
    CWMODE += WIFI_MODE;
    CWMODE += "\r\n";
    sendCommand(CWMODE,1000,DEBUG); // configure as access point
    delay(1000);
    Serial.println("3=========================");
    sendCommand("AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"\r\n",3000,DEBUG);
    delay(5000);
    Serial.println("4=========================");
    delay(1000);
    Serial.println("5=========================");
    String x = sendCommand("AT+CIFSR\r\n",1000,DEBUG); // get ip address
    
    if (WIFI_MODE == 1) {
        INTERNET_IP = splitString(x, '"', 1);
        INTERNET_IP = splitString(INTERNET_IP, '"', 0);

        INTERNET_MAC = splitString(x, '"', 3);
        INTERNET_MAC = splitString(INTERNET_MAC, '"', 0);
    }
    else if (WIFI_MODE == 2) {
        INTRANET_IP = splitString(x, '"', 1);
        INTRANET_IP = splitString(INTRANET_IP, '"', 0);

        INTRANET_MAC = splitString(x, '"', 3);
        INTRANET_MAC = splitString(INTRANET_MAC, '"', 0);
    }
    else if (WIFI_MODE == 3) {
        INTRANET_IP = splitString(x, '"', 1);
        INTRANET_IP = splitString(INTRANET_IP, '"', 0);

        INTRANET_MAC = splitString(x, '"', 3);
        INTRANET_MAC = splitString(INTRANET_MAC, '"', 0);

        INTERNET_IP = splitString(x, '"', 5);
        INTERNET_IP = splitString(INTERNET_IP, '"', 0);

        INTERNET_MAC = splitString(x, '"', 7);
        INTERNET_MAC = splitString(INTERNET_MAC, '"', 0);
    }

    if (INTERNET_IP.equals("")) {
        INTERNET_IP = "-";
    }
    if (INTRANET_IP.equals("")) {
        INTRANET_IP = "-";
    }

    if (INTERNET_IP.equals("-") && INTRANET_IP.equals("-")) {
        WIFI_MODE_TEMP = WIFI_MODE;
        WIFI_MODE = 2;
        setHTTP(ssid, pass, WIFI_MODE_TEMP);
        return;
    }

    delay(1000);
    Serial.println("6=========================");
    sendCommand("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
    delay(1000);
    Serial.println("7=========================");
    String CIPSERVER = "AT+CIPSERVER=1,";
    CIPSERVER += PORT.toInt();
    CIPSERVER += "\r\n";
    sendCommand(CIPSERVER,1000,DEBUG); // turn on server on port 80
    delay(1000);
    Serial.println("8=========================");

    if (WIFI_MODE_TEMP != 0) {
        Serial.print("WIFI_MODE ");
        Serial.print(WIFI_MODE);
        WIFI_MODE = WIFI_MODE_TEMP;
        Serial.print(" back to ");
        Serial.println(WIFI_MODE);
        WIFI_MODE_TEMP = 0;
    }
    Serial.println("Server Ready");
}


// ส่งข้อมูลปิงไป server
void sendHTTP(String ipdomain, int port, String param) {
    String startcommand = "AT+CIPSTART=4,\"TCP\",\"" + ipdomain + "\"," + port;
    Serial1.println(startcommand);
    Serial.println(startcommand);

    if (Serial1.find("Error")) {
        Serial.println("error on start");
        return;
    }

    String content = "GET " + param + " HTTP/1.1\r\nHost: " + ipdomain + "\r\n\r\n ";
    Serial.println(content);
    sendCIPData(4, content);
}

// เรียกค่าจากโมดูลไวไฟ แล้วเช็คว่ามันส่งอะไรมา
void checkHTTP() {
    if(Serial1.available()) // check if the esp is sending a message
    { 
        if(Serial1.find("+IPD,")) {
            delay(500);
            int connectionId = Serial1.read()-48;
            
            Serial1.find("relay=");
            String tmp = "";
            while (Serial1.available()) {
                int c = Serial1.read();
                if (c == 32)
                  break;
                tmp += (char) c;
            }

            Serial.println("get");
            Serial.println(tmp);

            String res = "";
            boolean reboot = false;
            if (tmp[0] == 'Z') {
              // Z001*
              // เปลี่ยนสถานะ relay 1, 2, 3, 4
               if (tmp[1] != '*')
                   digitalWrite(RELAY_1, tmp[1]);
               if (tmp[2] != '*')
                   digitalWrite(RELAY_2, tmp[2]);
               if (tmp[3] != '*')
                   digitalWrite(RELAY_3, tmp[3]);
               if (tmp[4] != '*')
                   digitalWrite(RELAY_4, tmp[4]);
               
               res += digitalRead(RELAY_1) == LOW ? "0" : "1";
               res += digitalRead(RELAY_2) == LOW ? "0" : "1";
               res += digitalRead(RELAY_3) == LOW ? "0" : "1";
               res += digitalRead(RELAY_4) == LOW ? "0" : "1";
            }
            else if (tmp[0] == 'A') {
              // A
              // ส่งค่าสถานะ โหมด heater, filter และ relay 1, 2
                    res += String(HEATER_MODE);
                    res += String(FILTER_MODE);
                    res += digitalRead(RELAY_1) == LOW ? "0" : "1";
                    res += digitalRead(RELAY_2) == LOW ? "0" : "1";
    
                    res += BOARD_NAME;
                    res += "-";
                    res += sensors.getTempCByIndex(0);
            }
            else if (tmp[0] == 'B') {
                    // B0011
                    // ตั้งค่า โหมด heater, filter และ relay 1, 2 และ
                    // ส่งค่าสถานะ โหมด heater, filter และ relay 1, 2
    
                    if (tmp[1] == '0') {
                        HEATER_MODE = 0;
                    }
                    else {
                        HEATER_MODE = 1;
                        digitalWrite(RELAY_1, tmp[3] == '0' ? LOW : HIGH);
                    }
                    
                    if (tmp[2] == '0') {
                        FILTER_MODE = 0;
                    }
                    else {
                        FILTER_MODE = 1;
                        digitalWrite(RELAY_2, tmp[4] == '0' ? LOW : HIGH);
                    }
                    
                    setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);
    
                    res += String(HEATER_MODE);
                    res += String(FILTER_MODE);
                    res += digitalRead(RELAY_1) == LOW ? "0" : "1";
                    res += digitalRead(RELAY_2) == LOW ? "0" : "1";
    
                    res += BOARD_NAME;
                    res += "-";
                    res += sensors.getTempCByIndex(0);
              
            }
            else if (tmp[0] == 'C') {
                  feed();
                  res += "OK";
            }
            else if (tmp[0] == 'D') {
                  res += HEATER_MODE;
                  res += "~";
                  res += FILTER_MODE;
                  res += "~";
                  res += HEATER_MIN;
                  res += "~";
                  res += FILTER_MAX;
                  res += "~";
                  res += WIFI_MODE;
                  res += "~";
                  res += WIFI_SSID;
                  res += "~";
                  res += WIFI_PASSWORD;
                  res += "~";
                  res += BOARD_NAME;
                  res += "~";
                  res += STATIC_IP;
                  res += "~";
                  res += PORT;
            }
            else if (tmp[0] == 'E') {
              HEATER_MODE = ((int) tmp[1]) - 48;
              FILTER_MODE = ((int) tmp[3]) - 48;
              HEATER_MIN = splitString(tmp, '~', 2).toFloat();
              FILTER_MAX = splitString(tmp, '~', 3).toFloat();
              WIFI_MODE = splitString(tmp, '~', 4).toInt();
              WIFI_SSID = splitString(tmp, '~', 5);
              WIFI_PASSWORD = splitString(tmp, '~', 6);
              BOARD_NAME = splitString(tmp, '~', 7);
              STATIC_IP = splitString(tmp, '~', 8);
              PORT = splitString(tmp, '~', 9);
              
              reboot = splitString(tmp, '~', 10) == "0" ? false : true;
              
              setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);
              
                  res += HEATER_MODE;
                  res += "~";
                  res += FILTER_MODE;
                  res += "~";
                  res += HEATER_MIN;
                  res += "~";
                  res += FILTER_MAX;
                  res += "~";
                  res += WIFI_MODE;
                  res += "~";
                  res += WIFI_SSID;
                  res += "~";
                  res += WIFI_PASSWORD;
                  res += "~";
                  res += BOARD_NAME;
                  res += "~";
                  res += STATIC_IP;
                  res += "~";
                  res += PORT;

                  if (reboot) {
                      sendHTTPResponse(connectionId, res);

                      lcd.setCursor(0, 0);
                      lcd.print("rebooting...");
                      wdt_enable(WDTO_1S);
                      
                      while (true) {
              
                      }
                  }
            }
            else {
              return;
            }

            sendHTTPResponse(connectionId, res);
        }
    }
}

// ฟังก์ชั่นเริ่มต้นที่ arduino กำหนด
// คือเรียกใช้แค่ครั้งเดียวตอนเปิดบอร์ด ตรงกันข้ามกับ loop()
void setup() {
  
//  clearEEPROM();
  //  resetBased();
      // กำหนดความถี่ช่วงข้อมูลสำหรับรับเข้า ส่งออกข้อมูลผ่านหน้าจอ serial monitor
    Serial.begin(9600); //9600
      // กำหนดความถี่ช่วงข้อมูลสำหรับรับเข้า ส่งออกข้อมูลโมดูลไวไฟ
    Serial1.begin(115200); //115200
    Serial.println("Booting...");
    
      // เรีกใช้ฟังก์ชั่นที่ชื่อขึ้นต้นด้วย set คือ กำหนดค่าพื้นฐานให้แต่ละโมดูล
    getBased();
    setLCD();
    setHTTP(WIFI_SSID, WIFI_PASSWORD);
    //setHTTP("MUCHJURAT", "plo013677");
    
    setServo();
    setTemp();
    setRelays();
}


#define PING_LONG 1000
int loop_runner = 0;
int r = 1;
// อีกฟังก์ชั่นที่ arduino กำหนดให้ต้องมี
void loop() {
    if (loop_runner % PING_LONG == 0) {
      sendHTTP("188.166.180.204", 8888, "/arduinoping.php?bid=" + BOARD_NAME + "&port=" + PORT);
      loop_runner = 1;
    }
    
    //    checkTemp(1);
    // checkTemp(2);

    checkHTTP();

    showLCD();
    checkKeypad();

    loop_runner++;
}

// โปรแกรม Keypad
String b1 = "._+-=!@#$%^&*()";
String b2 = "abcABC";
String b3 = "defDEF";
String b4 = "ghiGHI";
String b5 = "jklJKL";
String b6 = "mnoMNO";
String b7 = "pqrsPQRS";
String b8 = "tuvTUV";
String b9 = "wxyzWXYZ";
String b0 = "1234567890 ";

// ตรวจสอบปุ่มกด
void checkKeypad() {
    char c = keypad.getKey();
    if (c != NO_KEY) {
        lcd.clear();
        if (lcdMode == 0) {
            if (c == '*') {
                lcdMode = 1;
            }
            else if (c == 'C') {
                HEATER_MODE = 1;
                int state = digitalRead(RELAY_1);
                digitalWrite(RELAY_1, state == LOW ? HIGH : LOW);
                
            }
            else if (c == 'D') {
                FILTER_MODE = 1;
                int state = digitalRead(RELAY_2);
                digitalWrite(RELAY_2, state == LOW ? HIGH : LOW);
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
                // route
                if (cursorMenu == 0 && cursorMenuSub == 0) {
                    tmp = BOARD_NAME;

                    if (tmp.length() > 16) {
                        en = 16;
                    }
                    else {
                        en = tmp.length();
                    }

                    lcdMode = 11;
                }
                else if (cursorMenu == 1 && cursorMenuSub == 0) {
                    lcdMode = 21;
                }
                else if (cursorMenu == 2 && cursorMenuSub == 0) {
                    int a = (int) HEATER_MIN;
                    int b = ((int) (HEATER_MIN * 100) % 100);
                    tmp = (a < 10 ? "0" + String(a) : String(a)) + "." + (b < 10 ? "0" + String(b) : String(b));

                    lcdMode = 31;
                }
                else if (cursorMenu == 2 && cursorMenuSub == 1) {
                    int a = (int) FILTER_MAX;
                    int b = ((int) (FILTER_MAX * 100) % 100);
                    tmp = (a < 10 ? "0" + String(a) : String(a)) + "." + (b < 10 ? "0" + String(b) : String(b));

                    lcdMode = 32;
                }
                else if (cursorMenu == 2 && cursorMenuSub == 2) {
                    tmp = HEATER_MODE;

                    lcdMode = 33;
                }
                else if (cursorMenu == 2 && cursorMenuSub == 3) {
                    tmp = FILTER_MODE;

                    lcdMode = 34;
                }
                else if (cursorMenu == 3 && cursorMenuSub == 0) {
                    lcdMode = 41;
                }
                else if (cursorMenu == 3 && cursorMenuSub == 1) {
                    lcdMode = 42;
                }
                else if (cursorMenu == 3 && cursorMenuSub == 2) {
                    tmp = WIFI_MODE;

                    lcdMode = 43;
                }
                else if (cursorMenu == 3 && cursorMenuSub == 3) {
                    tmp = WIFI_SSID;

                    if (tmp.length() > 16) {
                        en = 16;
                    }
                    else {
                        en = tmp.length();
                    }

                    lcdMode = 44;
                }
                else if (cursorMenu == 3 && cursorMenuSub == 4) {
                    tmp = WIFI_PASSWORD;

                    if (tmp.length() > 16) {
                        en = 16;
                    }
                    else {
                        en = tmp.length();
                    }

                    lcdMode = 45;
                }
                else if (cursorMenu == 3 && cursorMenuSub == 5) {
                    tmp = STATIC_IP;

                    lcdMode = 46;
                }
                else if (cursorMenu == 3 && cursorMenuSub == 6) {
                    tmp = PORT;

                    lcdMode = 47;
                }
                else if (cursorMenu == 4 && cursorMenuSub == 0) {
                    lcdMode = 51;
                }
                else if (cursorMenu == 5 && cursorMenuSub == 0) {
                    lcdMode = 61;
                }

                chtmp = tmp[0];
            }
        }
        else if (lcdMode == 11) {
            if (tmp[tmp.length() - 1] != ' ') {
                tmp = tmp + " ";
            }
            if (c == '*') {
                if (blink == 0)
                tmp[indexEdit] = chtmp;

                tmp.trim();
                BOARD_NAME = tmp;
                setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);

                tmp = "";
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
            else if (c == 'D') {
                tmp.remove(indexEdit, 1);
                chtmp = tmp[indexEdit];
                if (tmp.length() - 1 < indexEdit) {
                    indexEdit--;
                    en--;
                }

                lastButton = -1;
                indexInButton = -1;
            }
        }
        else if (lcdMode == 21) {
          feed();
          lcdMode = 2;
        }
        else if (lcdMode == 31 || lcdMode == 32) {
            if (c == '*') {
                if (tmp[0] == '~')
                tmp[0] = chtmp;
                if (tmp[1] == '~')
                tmp[1] = chtmp;
                if (tmp[3] == '~')
                tmp[3] = chtmp;
                if (tmp[4] == '~')
                tmp[4] = chtmp;

                int ia = tmp.toInt();
                int ib = ((int) (tmp.toFloat() * 100)) % 100;

                if (lcdMode == 31) {
                    EEPROM.write(3, ia);
                    EEPROM.write(4, ib);
                    HEATER_MIN = EEPROM.read(3) + (EEPROM.read(4) / 100.0);
                }
                else if (lcdMode == 32) {
                    EEPROM.write(5, ia);
                    EEPROM.write(6, ib);
                    FILTER_MAX = EEPROM.read(5) + (EEPROM.read(6) / 100.0);
                }

                tmp = "";
                indexEdit = 0;
                en = 0;
                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '#') {

                tmp = "";
                indexEdit = 0;
                en = 0;
                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '0') {
                tmp[indexEdit] = c;
                if (indexEdit < 4) {
                    if (indexEdit == 1) {
                        indexEdit += 2;
                    }
                    else {
                        indexEdit++;
                    }
                }
                chtmp = tmp[indexEdit];
            }
            else if (c == 'A') {
                tmp[indexEdit] = chtmp;
                if (indexEdit < 4) {
                    if (indexEdit == 1) {
                        indexEdit += 2;
                    }
                    else {
                        indexEdit++;
                    }
                }
                chtmp = tmp[indexEdit];
            }
            else if (c == 'B') {
                tmp[indexEdit] = chtmp;
                if (indexEdit != 0) {
                    if (indexEdit == 3) {
                        indexEdit -= 2;
                    }
                    else {
                        indexEdit--;
                    }
                }
                chtmp = tmp[indexEdit];
            }

        }
        else if (lcdMode == 33 || lcdMode == 34) {
            if (c == '*') {
                if (tmp[0] == '~')
                tmp[0] = chtmp;

                int ia = tmp.toInt();

                if (lcdMode == 33) {
                    EEPROM.write(1, ia);
                    HEATER_MODE = EEPROM.read(1);
                }
                else if (lcdMode == 34) {
                    EEPROM.write(2, ia);
                    FILTER_MODE = EEPROM.read(2);
                }

                tmp = "";

                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '#') {
                tmp = "";
                indexEdit = 0;

                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '0' || c == '1') {
                chtmp = c;
                tmp[0] = c;
            }
        }
        else if (lcdMode == 41 || lcdMode == 42) {
            if (c == '#') {
                lcdMode = 2;
            }
        }
        else if (lcdMode == 43) {
            if (c == '*') {
                if (tmp[0] == '~')
                tmp[0] = chtmp;

                int ia = tmp.toInt();

                EEPROM.write(7, ia);
                WIFI_MODE = EEPROM.read(7);

                tmp = "";

                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '#') {
                tmp = "";

                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '1' || c == '2' || c == '3') {
                chtmp = c;
                tmp[0] = c;
            }
        }
        else if (lcdMode == 44) {
            if (tmp[tmp.length() - 1] != ' ') {
                tmp = tmp + " ";
            }
            if (c == '*') {
                if (blink == 0)
                tmp[indexEdit] = chtmp;

                tmp.trim();
                WIFI_SSID = tmp;
                setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);

                tmp = "";
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
            else if (c == 'D') {
                tmp.remove(indexEdit, 1);
                chtmp = tmp[indexEdit];
                if (tmp.length() - 1 < indexEdit) {
                    indexEdit--;
                    en--;
                }

                lastButton = -1;
                indexInButton = -1;
            }
        }
        else if (lcdMode == 45) {
            if (tmp[tmp.length() - 1] != ' ') {
                tmp = tmp + " ";
            }
            if (c == '*') {
                if (blink == 0)
                tmp[indexEdit] = chtmp;

                tmp.trim();
                WIFI_PASSWORD = tmp;
                setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);

                tmp = "";
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
            else if (c == 'D') {
                tmp.remove(indexEdit, 1);
                chtmp = tmp[indexEdit];
                if (tmp.length() - 1 < indexEdit) {
                    indexEdit--;
                    en--;
                }

                lastButton = -1;
                indexInButton = -1;
            }
        }
        else if (lcdMode == 46) {
            if (c == '*') {
                if (tmp[0] == '~')
                tmp[0] = chtmp;
                if (tmp[1] == '~')
                tmp[1] = chtmp;
                if (tmp[2] == '~')
                tmp[2] = chtmp;
                if (tmp[4] == '~')
                tmp[4] = chtmp;
                if (tmp[5] == '~')
                tmp[5] = chtmp;
                if (tmp[6] == '~')
                tmp[6] = chtmp;
                if (tmp[8] == '~')
                tmp[8] = chtmp;
                if (tmp[9] == '~')
                tmp[9] = chtmp;
                if (tmp[10] == '~')
                tmp[10] = chtmp;
                if (tmp[12] == '~')
                tmp[12] = chtmp;
                if (tmp[13] == '~')
                tmp[13] = chtmp;
                if (tmp[14] == '~')
                tmp[14] = chtmp;

                int ip1 = tmp.substring(0, 3).toInt();
                if (ip1 > 255)
                ip1 = 255;
                int ip2 = tmp.substring(4, 7).toInt();
                if (ip2 > 255)
                ip2 = 255;
                int ip3 = tmp.substring(8, 11).toInt();
                if (ip3 > 255)
                ip3 = 255;
                int ip4 = tmp.substring(12, 15).toInt();
                if (ip4 > 255)
                ip1 = 255;

                STATIC_IP = (ip1 < 10 ? "00" + String(ip1) : (ip1 < 100 ? "0" + String(ip1) : String(ip1))) + "."
                + (ip2 < 10 ? "00" + String(ip2) : (ip2 < 100 ? "0" + String(ip2) : String(ip2))) + "."
                + (ip3 < 10 ? "00" + String(ip3) : (ip3 < 100 ? "0" + String(ip3) : String(ip3))) + "."
                + (ip4 < 10 ? "00" + String(ip4) : (ip4 < 100 ? "0" + String(ip4) : String(ip4)));
                setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);

                tmp = "";
                indexEdit = 0;
                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '#') {
                tmp = "";
                indexEdit = 0;
                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '0') {
                chtmp = c;
                tmp[indexEdit] = c;

                if (indexEdit < 14) {
                    if (indexEdit == 2 || indexEdit == 6 || indexEdit == 10) {
                        indexEdit += 2;
                    }
                    else {
                        indexEdit++;
                    }
                }
                chtmp = tmp[indexEdit];
            }
            else if (c == 'A') {
                tmp[indexEdit] = chtmp;
                if (indexEdit < 14) {
                    if (indexEdit == 2 || indexEdit == 6 || indexEdit == 10) {
                        indexEdit += 2;
                    }
                    else {
                        indexEdit++;
                    }
                }
                chtmp = tmp[indexEdit];
            }
            else if (c == 'B') {
                tmp[indexEdit] = chtmp;
                if (indexEdit != 0) {
                    if (indexEdit == 5 || indexEdit == 8 || indexEdit == 12) {
                        indexEdit -= 2;
                    }
                    else {
                        indexEdit--;
                    }
                }
                chtmp = tmp[indexEdit];
            }
            else if (c == 'D') {
                tmp[indexEdit] = '0';
                if (indexEdit != 0) {
                    if (indexEdit == 5 || indexEdit == 8 || indexEdit == 12) {
                        indexEdit -= 2;
                    }
                    else {
                        indexEdit--;
                    }
                    chtmp = tmp[indexEdit];
                }
            }
        }
        else if (lcdMode == 47) {
            if (c == '*') {
                if (tmp[0] == '~')
                tmp[0] = chtmp;
                if (tmp[1] == '~')
                tmp[1] = chtmp;
                if (tmp[2] == '~')
                tmp[2] = chtmp;
                if (tmp[3] == '~')
                tmp[3] = chtmp;
                if (tmp[4] == '~')
                tmp[4] = chtmp;

                if (tmp.toInt() > 65535) {
                    tmp = "08081";
                }

                int p1 = tmp.toInt() / 256;
                int p2 = tmp.toInt() % 256;
                long p = (p1 * 256) + p2;

                PORT = (p < 10 ? "0000" + String(p) : (p < 100 ? "000" + String(p) : (p < 1000 ? "00" + String(p) : (p < 10000 ? "0" + String(p) : String(p)))));
                setEEPROM(0, HEATER_MODE, FILTER_MODE, HEATER_MIN, FILTER_MAX, WIFI_MODE, WIFI_SSID, WIFI_PASSWORD, BOARD_NAME, STATIC_IP, PORT);

                tmp = "";
                indexEdit = 0;
                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '#') {
                tmp = "";
                indexEdit = 0;
                blink = 0;
                frame = 0;
                lcdMode = 2;
            }
            else if (c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '0') {
                tmp[indexEdit] = c;

                if (indexEdit < 4) {
                    indexEdit++;
                }

                chtmp = c;
            }
            else if (c == 'A') {
                tmp[indexEdit] = chtmp;
                if (indexEdit < 4) {
                    indexEdit++;
                }
                chtmp = tmp[indexEdit];
            }
            else if (c == 'B') {
                tmp[indexEdit] = chtmp;
                if (indexEdit != 0) {

                    indexEdit--;
                }
                chtmp = tmp[indexEdit];
            }
            else if (c == 'D') {
                tmp[indexEdit] = '0';
                if (indexEdit != 0) {
                    indexEdit--;
                    chtmp = tmp[indexEdit];
                }
            }
        }
        else if (lcdMode == 61) {

        }

    }
}




// ฟังก์ชั่นตัดข้อความ
String splitString(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length()-1;

    for(int i=0; i<=maxIndex && found<=index; i++){
        if(data.charAt(i)==separator || i==maxIndex){
            found++;
            strIndex[0] = strIndex[1]+1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }

    return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}



// =================== ข้างล่างนี้ ก็อบมา มันมากับโค้ดไวไฟ =================== //

/*
* Name: sendHTTPResponse
* Description: Function that sends HTTP 200, HTML UTF-8 response
*/
void sendHTTPResponse(int connectionId, String content)
{
Serial.println("RESS ===============================");
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
    Serial.println("END ===============================");
    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println();
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
    Serial.println("sendCommand ===============================");
    sendCommand(cipSend,1000,DEBUG);
    Serial.println("sendData ===============================");
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

