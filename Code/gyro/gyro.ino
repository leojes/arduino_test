#include <Wire.h>


//const int a = A0;
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

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
  Wire.beginTransmission(mpu);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);
}

void loop() {
//  this is the code of angle with gyroscope
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
