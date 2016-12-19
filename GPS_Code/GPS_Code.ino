/*The open source pothole detection kit has been developed at Make-i-stan in collaboration with ICFJ Knight Fellowships*/

#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "Wire.h"
SoftwareSerial mySerial(10, 11); // RX, TX
TinyGPS gps;


void setup()  
{
  Wire.begin(8);     // Open serial communications and wait for port to open:
  Serial.begin(9600);     // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  delay(500);
  Wire.onRequest(requestEvent); // regi
}
  //3147636, 7434279
unsigned long time_=12000000, date_=111111, fix_age=99;
  long lat_ = 1000000, lon_ = 1000000;
void requestEvent() {
  String d = String(lat_) + ", " + String(lon_) + ", " + String(time_) + ", " + String(fix_age);// + ", " + String(time_);//+ ", " + String(fix_age);
  char buffer[30];
  d.toCharArray(buffer, 31);
  Wire.write(buffer); // respond with message of 6 bytes
  Serial.println(buffer);
  // as expected by master
}
int x = 0;
void loop() // run over and over
{
  if (mySerial.available()) 
  {
      char c = mySerial.read();
      if (gps.encode(c)) 
      {
          Serial.println("Acquired Data");
          Serial.println("-------------");
    
          gps.get_position(&lat_, &lon_);
          gps.get_datetime(&date_, &time_, &fix_age);

          Serial.print("Lat/Long(10^-5 deg): "); Serial.print(lat_); Serial.print(", "); Serial.print(lon_);Serial.print(", "); Serial.print(time_);Serial.print(", ");Serial.print(fix_age);Serial.print(", ");Serial.print(date_); 
          //delay(500);
          
          Serial.println();
      }
    }
}
