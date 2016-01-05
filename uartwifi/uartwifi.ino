
#define DEBUG true

void setup() {
    Serial.begin(9600);
    Serial1.begin(115200);
    Serial.println("Booting...");
    Serial.println("1=========================");
    sendCommand("AT+RST\r\n",2000,DEBUG); // reset module
    Serial.println("2=========================");
    sendCommand("AT+CWMODE=3\r\n",1000,DEBUG); // configure as access point
    Serial.println("3=========================");
    sendCommand("AT+CWJAP=\"MAIN\",\"\"\r\n",3000,DEBUG);
    Serial.println("4=========================");
    delay(1000);
    Serial.println("5=========================");
    sendCommand("AT+CIFSR\r\n",1000,DEBUG); // get ip address
    Serial.println("6=========================");
    sendCommand("AT+CIPMUX=1\r\n",1000,DEBUG); // configure for multiple connections
    Serial.println("7=========================");
    sendCommand("AT+CIPSERVER=1,8080\r\n",1000,DEBUG); // turn on server on port 80
    Serial.println("8=========================");

    
    Serial.println("Server Ready");
}

void loop() {

    checkHTTP();
}

void checkHTTP() {
    if(Serial1.available()) // check if the esp is sending a message
    {
      sendHTTPResponse(0, "");
      Serial.println();
//        if(Serial1.find("+IPD,"))
//        {
//            delay(1000); // wait for the serial buffer to fill up (read all the serial data)
//            // get the connection id so that we can then disconnect
//            int connectionId = Serial1.read()-48; // subtract 48 because the read() function returns
//            // the ASCII decimal value and 0 (the first decimal number) starts at 48
//
////            Serial1.find("pin="); // advance cursor to "pin="
//
//            int d1 = (Serial1.read()-48); // get first number i.e. if the pin 13 then the 1st number is 1
//            int d2 = (Serial1.read()-48);
//            int d3 = (Serial1.read()-48);
//            int d4 = (Serial1.read()-48);
//            int d5 = (Serial1.read()-48);
//            int d6 = (Serial1.read()-48);
//
//            String content = "hello";
//            
//
//            Serial.print("d1 = ");
//            Serial.print(d1);
//            Serial.println();
//             sendHTTPResponse(connectionId, content);
//        }
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
        Serial.println("\r\n====== HTTP Response From Arduino ======");
        Serial.write(data,dataSize);
        Serial.println("\r\n========================================");
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

