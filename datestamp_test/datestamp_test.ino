/*
  Time Check

 Gets the time from Linux via Bridge then parses out hours,
 minutes and seconds using a YunShield/Yún.

 created  27 May 2013
 modified 21 June 2013
 By Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/TimeCheck

 */


#include <Process.h>

Process date;                 // process used to get the date
int hours, minutes, seconds;  // for the results
int lastSecond = -1;          // need an impossible value for comparison

void setup() {
  Bridge.begin();        // initialize Bridge
  Console.begin();    // initialize serial

  while (!Console);              // wait for Console Monitor to open
  Console.println("Time Check");  // Title of sketch

  // run an initial date process. Should return:
  // hh:mm:ss :
  if (!date.running()) {
    date.begin("date");
    date.addParameter("+%x_%H:%M:%S");
    date.run();
    Console.println("First datestamp running...");
  }
}

void loop() {

    // restart the date process:
    if (!date.running()) { // must block Process to wait results from across the Bridge
      date.begin("date");
      date.addParameter("+%x_%H:%M:%S");
      date.run();
    }

    String timeString = date.readString();
    Console.print("Time: ");
    Console.println(timeString);
    delay(5000); 
}
