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

//********DMDisplay *************//
#include <SPI.h>        //SPI.h must be included as DMD is written by SPI (the IDE complains otherwise)
#include <DMD2.h>        //https://github.com/freetronics/DMD
#include "SystemFont5x7.h"
#include "Arial_black_16.h"

//Fire up the DMD library as dmd
#define DISPLAYS_ACROSS 1
#define DISPLAYS_DOWN 1
SoftDMD dmd(1,1,3,2,4,7,6,8);  // DMD controls the entire display (largura,altura)
//SoftDMD(byte panelsWide, byte panelsHigh, byte pin_noe, byte pin_a, byte pin_b, byte pin_sck, byte pin_clk, byte pin_r_data);
DMD_TextBox box(dmd, 0, 2);  // "box" provides a text box to automatically write to/scroll the display
//******END CONFIGS*****//

void setup()
{
  Serial.begin(115200);
  dmd.setBrightness(255);
  dmd.selectFont(Arial_Black_16);
  dmd.begin();
  /* TIP: If you want a longer string here than fits on your display, just define the display DISPLAYS_WIDE value to be wider than the
    number of displays you actually have.
   */
  dmd.drawString(0, 0, F("Hello World!"));

  while (!Serial) ; // Needed for Leonardo only
  SerialGPS.begin(9600);
  Serial.println("Waiting for GPS time ... ");
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
   dmd.clearScreen();
   dmd.selectFont(Arial_Black_16);
   box.print('Hello');

  //Output to the Serialport 
  if (timeStatus()!= timeNotSet) {
    if (now() != prevDisplay) { //update the display only if the time has changed
      prevDisplay = now();
      digitalClockDisplay(); 
    }
  }
}

void digitalClockDisplay(){
  // digital clock display of the time
  //Serial.print(hour());
  Serial.print(hr12to24(hour()));
  printDigits(minute());
  printDigits(second());
  if(appisPM == true){Serial.println(" PM");}else{Serial.println(" AM");}
  
    
//  Serial.print(" ");
//  Serial.print(day());
//  Serial.print(" ");
//  Serial.print(month());
//  Serial.print(" ");
//  Serial.print(year()); 
//  Serial.println(); 
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


