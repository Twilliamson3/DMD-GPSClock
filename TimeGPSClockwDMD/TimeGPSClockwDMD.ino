/*
 * TimeGPS.pde
 * example code illustrating time synced from a GPS
 * PLUS
 * DMD Code for displaying clock
 * OBJECTIVE: GPS Sync'd Clock 
 * |--------|--------|
 * |1 2: 4 5 : 59MMDD
 * |--------|--------|
 */

//********Clock and GPS **********//
#include <Time.h>            // http://www.pjrc.com/teensy/td_libs_Time.html
#include <TinyGPS.h>         // http://arduiniana.org/libraries/TinyGPS/
//#include <SoftwareSerial.h>  // TinyGPS and SoftwareSerial libraries are the work of Mikal Hart

//SoftwareSerial SerialGPS = SoftwareSerial(10, 11);  // receive on pin 10
TinyGPS gps; 

// To use a hardware serial port, which is far more efficient than
// SoftwareSerial, uncomment this line and remove SoftwareSerial
#define SerialGPS Serial1

// Offset hours from gps time (UTC)
//const int offset = 1;   // Central European Time
const int offset = -5;  // Eastern Standard Time (USA)
//const int offset = -4;  // Eastern Daylight Time (USA)
//const int offset = -8;  // Pacific Standard Time (USA)
//const int offset = -7;  // Pacific Daylight Time (USA)

time_t prevDisplay = 0; // when the digital clock was displayed
boolean appisPM = false;
boolean clockinsync = false; // Has SoftRTC been sync'd lately"?
String Hours ;
String Minutes ;
String Sec ;
String Col = ":";
String AP;
String Mo;
String Date;
String Yr;

//********DMDisplay *************//
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD2.h>        //https://github.com/freetronics/DMD
#include "SystemFont5x7.h"
#include "Arial_black_16.h"
#include <fonts/Arial14.h>
#include <fonts/Droid_Sans_16.h>


//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
SoftDMD dmd(2,1);  // DMD controls the entire display
//******END CONFIGS*****//

void setup()
{
  Serial.begin(115200);
  dmd.setBrightness(30);
  //dmd.selectFont(Arial_Black_16);
  //dmd.selectFont(Arial14);
  //dmd.selectFont(Droid_Sans_16);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();

  while (!Serial) ; // Needed for Leonardo only
  SerialGPS.begin(9600);
  Serial.println("Waiting for GPS time ... ");
  dmd.drawString(8,0,"GPS Lost");
}

void loop()
{
  while (SerialGPS.available()) {
    if (gps.encode(SerialGPS.read())) { // process gps messages
      // when TinyGPS reports new data...
      unsigned long age;
      int Year;
      byte Month, Day, Hour, Minute, Second;
      gps.crack_datetime(&Year, &Month, &Day, &Hour, &Minute, &Second, NULL, &age);
      if (age < 500) {
        // set the Time to the latest GPS reading
        setTime(Hour, Minute, Second, Day, Month, Year);
        adjustTime(offset * SECS_PER_HOUR);
        Serial.println("Sync'd");
        clockinsync = true;
      }
      
    }
  }
  //Print to the DMD
   //dmd.drawString(0,0,"Hello");

  //Output to the Serialport 
  if (timeStatus()!= timeNotSet) {
    if (now() != prevDisplay) { //update the display only if the time has changed
      prevDisplay = now();
      digitalClockDisplay(); 
    }
  }
}

void digitalClockDisplay(){


  //Serial.print(cTIME);
  // digital clock display of the time
  //Serial.print(hour());
  Serial.print(hr12to24(hour()));
  printDigits(minute());
  printDigits(second());
  
  if(appisPM == true){Serial.println(" PM"); AP = "p";}else{Serial.println(" AM");AP = "a";}
  
  
int min =  minute();
if ( min < 10){Minutes = "0" + String(min);}else{Minutes = String(min);}    

int seconds =  second();
if ( seconds < 10){Sec = "0" + String(seconds);}else{Sec = String(seconds);} 

int hrs =  hour();
if ( hrs < 12) { if( hrs < 10 ) {Hours = "0" + String(hrs);} else {Hours = String(hrs);}} else {hrs = hrs - 12; if( hrs < 10 ) {Hours = "0" + String(hrs);} else {Hours = String(hrs);} ;} 

String cTIME= Hours + Col + Minutes + Col + Sec + AP;
Serial.println(cTIME);
//dmd.clearScreen();
//dmd.drawFilledBox(54,0,64,16);
dmd.drawString(6,0,cTIME);
//Draw a line for secs
dmd.drawFilledBox(0,6,1,8);
dmd.drawFilledBox(62,6,63,8);
if (seconds == 0){dmd.drawLine(2,7,62,7,GRAPHICS_OFF);}else{dmd.drawLine(0,7,(seconds+1),7);}
//end of draw a line

  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}


int hr12to24 (int hour24){
  //Serial.print("hour24: ");Serial.println(hour24);
  if(hour24 > 12){
    hour24 = hour24 - 12;
    appisPM = true;
    } else {appisPM = false;}
    return hour24; 
}
