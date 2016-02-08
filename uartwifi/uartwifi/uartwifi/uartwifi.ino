
// นำเข้าไลบรารี่สำหรับพวกวัดอุณหภูมิ
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


boolean heaterAuto = true;
boolean filterAuto = true;

#define RELAY_1 28
#define RELAY_2 26
#define RELAY_3 24
#define RELAY_4 22

#include <SPI.h>
#include <SD.h>

const int chipSelect = 48;

#define DEBUG true

const String bid = "A0001";

void setup() {
    Serial.begin(9600);
    Serial1.begin(115200);
    Serial.println("Booting...");
    
    setHTTP("AP", "12345678");
    
    setTemp();
    
    setRelays();
}

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
    
    //    checkTemp(0);
    //    checkTemp(1);
    //  checkTemp(2);
    
    checkHTTP();
    
//    ping();
}

int runner = 0;
char server[] = "http://188.166.180.204:8888/arduinoping.php";

void ping() {
    if (runner++ == 30000) {
        Serial.println("Connecting to Server...");
        //     // Make a HTTP request:
        //     Serial1.print("GET http://188.166.180.204:8888/arduinoping.php?bid=A0000&ip=118.175.112.32&port=8080");
        //     Serial1.println(" HTTP/1.1");
        //     Serial1.println("Host: batmastertest.com");
        //     Serial1.println();
        
        sendHTTP("188.166.180.204", 8888, "/arduinoping.php?bid=A0001&port=8080");
    }
}

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

void setTemp() {
    sensors.begin();
}

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

void setHTTP(String ssid, String pass) {
    Serial.println("1=========================");
    sendCommand("AT+RST\r\n",2000,DEBUG); // reset module
    delay(5000);
    Serial.println("2=========================");
    sendCommand("AT+CWMODE=3\r\n",1000,DEBUG); // configure as access point
    delay(5000);
    Serial.println("3=========================");
    sendCommand("AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"\r\n",3000,DEBUG);
    delay(5000);
    Serial.println("4=========================");
    delay(5000);
    Serial.println("5=========================");
    sendCommand("AT+CIFSR\r\n",1000,DEBUG); // get ip address
    delay(5000);
    Serial.println("6=========================");
    sendCommand("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
    delay(5000);
    Serial.println("7=========================");
    sendCommand("AT+CIPSERVER=1,8080\r\n",1000,DEBUG); // turn on server on port 80
    delay(5000);
    Serial.println("8=========================");
    
    
    Serial.println("Server Ready");
}

void sendHTTP(String ipdomain, int port, String param) {
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
}


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
                double temp = sensors.getTempCByIndex(0);
                res += String(temp);
                
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
                double temp = sensors.getTempCByIndex(0);
                res += String(temp);
                
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

