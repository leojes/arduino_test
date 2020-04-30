#include <TinyGPS++.h>
#include <SoftwareSerial.h>  /* Create object named bt of the class SoftwareSerial */

#include <Wire.h>
SoftwareSerial GPS_SoftSerial(5, 4);/* (Rx, Tx) /* Create an object named gps of the class TinyGPSPlus */
TinyGPSPlus gps;      

SoftwareSerial mySerial(3,2); // connected to 2 and 3rd port of the controller
volatile float minutes, seconds;
volatile int degree, secs, mins;

const int mpu=0x68;
const int gyro=0x43;
const int acc=0x3B;
float x,y,z;
float xacc, yacc, zacc, accAngleX, accAngleY;
float previousTime, currentTime, elapsedTime;
float xf,yf,zf;
float roll, pitch;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
int c = 0;

String SendCmd="AT+CIPSEND=";           // sending number of byte command
String Start="AT+CIPSTART=\"TCP\"";     // TCPIP start command 
String url="";
String ip="\"data.sparkfun.com\"";        // sparkfun server ip or url
String mobile="+ZZxxxxxxxxxx";//change ZZ with country code and xxxxxxxxxxx with phone number to sms
String msg="";
typedef struct gyro{
  float roll,pitch;
}gy;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  /* Define baud rate for serial communication */
  GPS_SoftSerial.begin(9600); /* Define baud rate for software serial communication */
  mySerial.begin(9600);
  Serial.print("Initializing GSM");
  initGSM();      // init GSM module
  Serial.print("Initializing GPRS");
  initGPRS();     // init GPRS in GSM Module
  Serial.print("Initializing wire");
  Wire.begin();
  Wire.beginTransmission(mpu);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
//  sendTxt(mobile); //if nessary
  Serial.print("System Ready");
}

void loop() {
  // put your main code here, to run repeatedly:
  gy pt;
  gps_data();
  pt=gyro_data();
  main_gsm();
}
void gps_data() {
   smartDelay(1000);  /* Generate precise delay of 1ms */
   unsigned long start;
   double lat_val, lng_val, alt_m_val;
   uint8_t hr_val, min_val, sec_val;
   bool loc_valid, alt_valid, time_valid;
   lat_val = gps.location.lat(); /* Get latitude data */
   loc_valid = gps.location.isValid(); /* Check if valid location data is available */    lng_val = gps.location.lng(); /* Get longtitude data */
   alt_m_val = gps.altitude.meters();  /* Get altitude data in meters */
   alt_valid = gps.altitude.isValid(); /* Check if valid altitude data is available */
   hr_val = gps.time.hour(); /* Get hour */
   min_val = gps.time.minute();  /* Get minutes */
   sec_val = gps.time.second();  /* Get seconds */
   time_valid = gps.time.isValid();  /* Check if valid time data is available */
   if (!loc_valid) {          
      Serial.print("Latitude : ");
      Serial.println("*****");
      Serial.print("Longitude : ");
      Serial.println("*****");
      }
   else {
      DegMinSec(lat_val);
      Serial.print("Latitude in Decimal Degrees : ");
      Serial.println(lat_val, 6);
      Serial.print("Latitude in Degrees Minutes Seconds : ");
      Serial.print(degree);
      Serial.print("\t");
      Serial.print(mins);
      Serial.print("\t");
      Serial.println(secs);
      DegMinSec(lng_val); /* Convert the decimal degree value into degrees minutes seconds form */
      Serial.print("Longitude in Decimal Degrees : ");
      Serial.println(lng_val, 6);
      Serial.print("Longitude in Degrees Minutes Seconds : ");
      Serial.print(degree);
      Serial.print("\t");
      Serial.print(mins);
      Serial.print("\t");
      Serial.println(secs);
      }
      if (!alt_valid) {
         Serial.print("Altitude : ");
         Serial.println("*****");
        }
      else {
         Serial.print("Altitude : ");
         Serial.println(alt_m_val, 6);    
        }
      if (!time_valid) {
         Serial.print("Time : ");
         Serial.println("*****");
        }
      else {
          char time_string[32];
          sprintf(time_string, "Time : %02d/%02d/%02d \n", hr_val, min_val, sec_val);
          Serial.print(time_string);    
        }
}
void DegMinSec( double tot_val)    /* Convert data in decimal degrees into degrees minutes seconds form */
{  
  degree = (int)tot_val;
  minutes = tot_val - degree;
  seconds = 60 * minutes;
  minutes = (int)seconds;
  mins = (int)minutes;
  seconds = seconds - minutes;
  seconds = 60 * seconds;
  secs = (int)seconds;
}
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (GPS_SoftSerial.available())  /* Encode data read from GPS while data is available on serial port */
      gps.encode(GPS_SoftSerial.read());
    /* Encode basically is used to parse the string received by the GPS and to store it in a buffer so that information can be extracted from it */
  } while (millis() - start < ms);
}

gy gyro_data() {
  gy point;
  previousTime=currentTime;
  currentTime=millis();
  elapsedTime=(currentTime-previousTime)/1000;
  Wire.beginTransmission(mpu);
  Wire.write(gyro);
  Wire.endTransmission(false);
  Wire.requestFrom(mpu, 6, true);
  
  x = (Wire.read() << 0 | Wire.read())/131.0;
  y = (Wire.read() << 0 | Wire.read())/131.0;
  z = (Wire.read() << 0 | Wire.read())/131.0;

  x = x + 0.56; // GyroErrorX ~(-0.56)
  y = y - 2; // GyroErrorY ~(2)
  z = z + 0.79; // GyroErrorZ (-0.79)

  xf=xf+x*elapsedTime;
  yf=yf+y*elapsedTime;
  zf=zf+z*elapsedTime;
  
  //this is the code for accelerometer
  Wire.beginTransmission(mpu);
  Wire.write(acc);
  Wire.endTransmission(false);
  Wire.requestFrom(mpu, 6, true);
  
  xacc = (Wire.read() << 0 | Wire.read())/16384.0;
  yacc = (Wire.read() << 0 | Wire.read())/16384.0;
  zacc = (Wire.read() << 0 | Wire.read())/16384.0;

  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) + 1.58; // AccErrorY ~(-1.58)

  //combination
  roll = 0.95*xf+0.05*accAngleX;
  pitch = 0.95*yf+0.05*accAngleY;
  point.roll=roll;
  point.pitch=pitch;
  return point;
}
void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times
  while (c < 200) {
    Wire.beginTransmission(mpu);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;
  c = 0;
  // Read gyro values 200 times
  while (c < 200) {
    Wire.beginTransmission(mpu);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(mpu, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
  GyroErrorZ = GyroErrorZ / 200;
  // Print the error values on the Serial Monitor
  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);
}
void sendTxt(String mobile)
{
  connectGSM("AT+CMGF=1","OK"); // Configuring TEXT mode
  mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();

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
void main_gsm() {
  while(1)
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


//#include <stdio.h>
//
//typedef struct test {
//    int a,b;
//}s;
//
//
//int main()
//{
//    printf("Hello World\n");
//    s q;
//    q.a=1;
//    q.b=2;
//    printf("%d",q.b);
//    return 0;
//}
