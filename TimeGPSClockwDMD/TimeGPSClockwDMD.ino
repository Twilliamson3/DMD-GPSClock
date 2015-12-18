/*
 * DMD Code for displaying clock based on a Software RTC and GPS Sync Source
 * Board : MEGA (Needed for RAM and Ports)
 * OBJECTIVE: GPS Sync'd Clock
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
SoftDMD dmd(2, 1); // DMD controls the entire display
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
  dmd.drawString(8, 0, "GPS Lost");

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
      }

    }
  }


  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if the time has changed
      prevDisplay = now();
      updateDMDprintableTime();
      serialPrintTime();
      gpsSatsSignal(second(), gps.satellites()); // Display GPS Signal Bars
      secTicker(second()); //Display Second Ticker
      digitalClockDisplay();
    }
  }
}

void digitalClockDisplay() {
  // Display Date to the serial port
  Serial.print(weekday()); //?Sunday=1?
  Serial.print("   ");
  Serial.print(month());
  Serial.print("/");
  Serial.print(day());
  Serial.print("/");
  Serial.print(year());
  Serial.println();
}


void updateDMDprintableTime() {
  //String cTIME = Hours + Col + Minutes + Col + Sec + AP; // The String that holds the time.
  dmd.drawString(6, 0, (timeTOtwodigits(hr24to12(hour())) + Col + timeTOtwodigits(minute()) + Col + timeTOtwodigits(second()) + AP)); // Display the Time on the LED Panel
}

void serialPrintTime() {

  Serial.println(timeTOtwodigits(hr24to12(hour())) + Col + timeTOtwodigits(minute()) + Col + timeTOtwodigits(second()) + AP); // The String that holds the time.

}


//----Function----
//Pass in 24 hour (hour) and the global appisPM flag and AP String is
//adjusted and the hour converted into 12 hour format is returned.

int hr24to12(int hour24) {
  if (hour24 > 12) {
    hour24 = hour24 - 12;
    appisPM = true;
    AP = "p";
    return hour24;
  }
  if (hour24 == 12) {
    appisPM = true;
    AP = "p";
    return 12;
  }
  if (hour24 == 0) {
    appisPM = false;
    AP = "a";
    return 12;
  }
  if (hour24 < 12) {
    appisPM = false;
    AP = "a";
    return hour24;
  }
}

//----Function----
//timeTOtwodigits
//Pass in a int value and it is converted to a propper String with 2 digits
String timeTOtwodigits (int timeValue) {
  if (timeValue <= 9) {
    return "0" + String(timeValue) ;
  } else {
    return String(timeValue);
  }
}

void secTicker (int sec) {
  //This Draws a line across the screen that builds with each second, it also had blocks at each end for better effect. This is designed for two panels in horizontal arrangment
  //Draw a the boxes
  dmd.drawFilledBox(0, 6, 1, 8);
  dmd.drawFilledBox(62, 6, 63, 8);
  // Psudo: Erase the line if sec = 0 ELSE the line length should equal sec +1
  if (sec == 0) {
    dmd.drawLine(2, 7, 62, 7, GRAPHICS_OFF);
  } else {
    dmd.drawLine(0, 7, (sec + 1), 7);
  }
  //end of draw a line
}

void gpsSatsSignal(int seconds, int green) {
  if (seconds % 15 == 0 || green == 0) {
    //Print Sats
    Serial.print("Satellites: "); Serial.println(gps.satellites());
    dmd.drawFilledBox(61, 15, 63, 13, GRAPHICS_OFF);
    if (green <= 3) {
      //Display an X
      dmd.setPixel(61, 15, GRAPHICS_ON);
      dmd.setPixel(62, 14, GRAPHICS_ON);
      dmd.setPixel(63, 13, GRAPHICS_ON);
      dmd.setPixel(61, 13, GRAPHICS_ON);
      dmd.setPixel(63, 15, GRAPHICS_ON);
    }
    if (green > 3) {
      //Display 1 Short bar
      dmd.setPixel(61, 15, GRAPHICS_ON);
    }
    if (green > 4) {
      //Display 1 Short bar && 1 Medium bar
      dmd.setPixel(62, 15, GRAPHICS_ON);
      dmd.setPixel(62, 14, GRAPHICS_ON);
    }
    if (green > 5) {
      //Display 1 Short bar && 1 Medium bar && 1 Tall Bar
      dmd.setPixel(63, 15, GRAPHICS_ON);
      dmd.setPixel(63, 14, GRAPHICS_ON);
      dmd.setPixel(63, 13, GRAPHICS_ON);

    }
  }
}
