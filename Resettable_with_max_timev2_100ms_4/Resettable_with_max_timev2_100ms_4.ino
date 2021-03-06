#include <LiquidCrystal.h>
#include <I2Cdev.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>
using namespace std;
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include <SPI.h>
#include <SD.h>

MPU6050 mpu;
File myFile;
int getgps=2;
int getgyro=2;
int iter=0;
unsigned long telapsed=0;
float pitch=0, roll=0;
int bumpval=0;
LiquidCrystal lcd(12, 11, 6, 5, 4, 3);

#define OUTPUT_READABLE_YAWPITCHROLL

#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno & most boards
#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
SoftwareSerial mySerial(10, 11); // RX, TX
TinyGPS gps;

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };

//int smallBups = 0, LargeBups = 0;
class valuesPackage
{
  public:
    float p = 0, r =0;
};

int index = 0;
valuesPackage valuesArr[2]; //os

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================


volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}
  


// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {

Wire.begin();
Serial.begin(9600);
mySerial.begin(9600);
lcd.begin(16, 2);


//Initialize SD card code

 while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  if (!SD.begin(53)) {
    Serial.println("SD card initialization failed!");
    lcd.print("SD card initialization failed!");
    delay(2000);
  lcd.clear();
   return;
  }
  Serial.println("SD card initialization done."); 
//Initialize SD card code ends here

    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
       // Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    
    while (!Serial); // wait for Leonardo enumeration, others continue immediately

    // initialize device
    Serial.println(F("Initializing I2C devices..."));

    mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // wait for ready



    
    Serial.println(F("\nSend any character to begin DMP programming and demo: "));
    while (Serial.available() && Serial.read()); // empty buffer
    lcd.print("waiting in 40");
    //while (!Serial.available());                 // wait for data
    while (Serial.available() && Serial.read()); // empty buffer again




    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        //Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        
        
        //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        //mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

    // Arduino Mega Reset pin
    //pinMode( 7, OUTPUT);
  //digitalWrite( 7, LOW);
  
     


}



// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {

 myFile = SD.open("gyro_gps.txt", FILE_WRITE);
 //myFile.println();
 //myFile.println("took approximately 8 seconds to reset");
 Wire.requestFrom(8,15);    // request 8 bytes from slave device #8
 delay(50);
 if(myFile)
 {
 if(getgps==1)
 {
  while (Wire.available()) { // slave may send less than requested
    char c = Wire.read(); // receive a byte as character
    myFile.print(c);
    Serial.print(c); // print the character
    lcd.print(c);
  }
  myFile.print(", ");
  myFile.close();
 getgyro=1;
 Serial.println();
 telapsed=millis();
 Serial.print("Iteration number: ");
 iter=iter+1;
 Serial.println(iter);
 Serial.print("Time Elapsed");
 Serial.println(telapsed);
}
else;
 }
 else
 {
 lcd.print("Check SD card");
 Serial.print("Stuck in line 241");
 delay(300);
 lcd.clear();
 pinMode(7,OUTPUT);


 }
 getgps=0;

 
    // if programming failed, don't try to do anything
    if (!dmpReady) return;

    // wait for MPU interrupt or extra packet(s) available

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 3072) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
     //    Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {

       
        // wait for correct available data length, should be a VERY short wait
        
        
        
        
        //while (fifoCount < packetSize) //was turned off by awais
          
          
          
          
          fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        
        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;

        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

valuesPackage obj;

obj.p = ypr[1] * 180/M_PI;
obj.r = ypr[2] * 180/M_PI;
            valuesArr[index%2] = obj;
index++;
             
  if(index > 50)
  {
    int lastIndex =  (index+1)%2;
    pitch = valuesArr[index%2].p - valuesArr[lastIndex].p;
    roll = valuesArr[index%2].r - valuesArr[lastIndex].r;
    if(pitch<0)
    pitch= (-1*pitch);
    else;
    if(roll<0)
    roll= (-1*roll);
    else;
    pitch=pitch*4;
    roll=roll*4;
    bumpval= round(pitch+roll);
    //pitch = abs(pitch);
    //roll = abs(roll);
  
  
     Serial.print("-----------------------");
        Serial.print(pitch);Serial.print(" ");
      Serial.println(roll);Serial.println();
      //Writing Data to SD card now
      // if the file opened okay, write to it
      if(getgyro==1){
      if (myFile) {
    Serial.print("Writing gyro_gps.txt...");
    //lcd.print("Writing gyro.txt...");
    delay(30);
    lcd.setCursor(0,1);
    //myFile.print(pitch);
    //lcd.print(pitch);
    //myFile.print(", ");
    //lcd.print(", ");
    //myFile.print(roll);
    //lcd.print(roll);
    //myFile.print(", ");
    //lcd.print(", ");
    lcd.print(bumpval);
    myFile.print(bumpval);
    myFile.print(", ");
    lcd.print(", ");
    myFile.println();
    // close the file:
    myFile.close();
    Serial.println("done.");   
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening gyro_gps.txt");
    lcd.setCursor(0,1);
    lcd.println("error opening gyro.txt");
    delay(500);
 
  }
      }
      getgps=1;
  }
  else
  lcd.print("Gyro stabilizing..");
delay(70);
lcd.clear();
        #endif
        
    }
  
}

