#include <SoftwareSerial.h>
SoftwareSerial mySerial(3,2); // connected to 2 and 3rd port of the controller

String SendCmd="AT+CIPSEND=";           // sending number of byte command
String Start="AT+CIPSTART=\"TCP\"";     // TCPIP start command 
String url="";
String ip="\"data.sparkfun.com\"";        // sparkfun server ip or url
String mobile="+ZZxxxxxxxxxx";//change ZZ with country code and xxxxxxxxxxx with phone number to sms
String msg="";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);

  Serial.print("Initializing GSM");
  initGSM();      // init GSM module

  Serial.print("Initializing GPRS");
  initGPRS();     // init GPRS in GSM Module
  
  mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();

  Serial.print("System Ready");
  
  sendTxt(mobile);
}

void loop() {
  // put your main code here, to run repeatedly:
while(1)
{
  if ("some condition")
  {
    Serial.print("Sending Data");
    Serial.print("To Server");
//    myGsm.println("AT+HTTPINIT"); //init the HTTP request
//    myGsm.println("AT+HTTPPARA=\"URL\",\"http://api.thingspeak.com/apps/thinghttp/send_request?api_key=VV7WQ9DS19E2BNNI\"");// setting the httppara, 
//    //the second parameter is the website from where you want to access data 
//    printSerialData();
//    myGsm.println("AT+HTTPACTION=0");//submit the GET request 
//    delay(8000);//the delay is important if the return datas are very large
//    printSerialData();
//    myGsm.println("AT+HTTPREAD=0,20");// read the data from the website you access
//    myGsm.println("AT+HTTPTERM");// terminate HTTP service
   url="GET /input/";
//    url+=publicKey;
//    url+="?private_key=";
//    url+=pvtKey;
//    url+="&log=";
    url+=msg;
    url+=" HTTP/1.0\r\n\r\n";
    String svr=Start+","+ip+","+"80";
    delay(1000);
    connectGSM(svr,"CONNECT");
    delay(1000);
    int len = url.length();
    String str="";
    str=SendCmd+len;
    sendToServer(str);
    mySerial.print(url);
    delay(1000);
    mySerial.write(0x1A);
    delay(1000);
  }
}
}

void sendTxt(String mobile)
{
  connectGSM("AT+CMGF=1","OK"); // Configuring TEXT mode
  String cmgs="AT+CMGS=";
  String cmd=cmgs+"\""+mobile+"\"";
  connectGSM(cmd,"OK");
  connectGSM("Random text content","OK"); //text content
}
void initGSM()
{
  connectGSM("AT","OK");
  connectGSM("ATE1","OK");
  connectGSM("AT+CPIN?","READY");
}
void initGPRS()
{
  connectGSM("AT+CIPSHUT","OK");
  connectGSM("AT+CGATT=1","OK");
//  connectGSM("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"","OK");//setting the SAPBR,connection type is GPRS
//  connectGSM("AT+SAPBR=3,1,\"APN\",\"\"","OK");//setting the APN,2nd parameter empty works for all networks 
  connectGSM("AT+CSTT=\"airtelgprs.com\",\"\",\"\"","OK");
  connectGSM("AT+CIICR","OK");
  delay(1000);
  mySerial.println("AT+CIFSR");
  delay(1000);
}
void connectGSM (String cmd, char *res)
{
  while(1)
  {
    mySerial.println(cmd);
    mySerial.println(cmd);
    delay(500);
    while(mySerial.available()>0)
    {
      if(mySerial.find(res))
      {
        delay(1000);
        return;
      }
    }
    delay(1000);
   }
 }
void sendToServer(String str)
{
  mySerial.println(str);
  delay(1000);
}
void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available())
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
