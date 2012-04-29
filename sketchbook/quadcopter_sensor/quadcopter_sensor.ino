// A simple data logger for the Arduino analog pins
#define LOG_INTERVAL  200 // mills between entries
#define ECHO_TO_SERIAL   0 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

// the digital pins that connect to the LEDs
#define redLEDpin 1
#define greenLEDpin 2

#include <Wire.h>
#include "Adafruit_BMP085.h"
#include "RTClib.h"
#include "SD.h"

Adafruit_BMP085 bmp;
RTC_DS1307 RTC; // define the Real Time Clock object

// these constants describe the pins. They won't change:
const int xpin = A0;                  // x-axis of the accelerometer
const int ypin = A1;                  // y-axis
const int zpin = A2;                  // z-axis (only on 3-axis models)

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

char filename[] = "LOGGER00.CSV";

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);
  
  while(1);
}

void setup()
{
  Serial.begin(9600);
  Serial.println();

#if ECHO_TO_SERIAL
  // initialize the SD card
  Serial.print("Initializing SD card...");
#endif
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  
#if ECHO_TO_SERIAL  
  Serial.println("card initialized.");
#endif

  // create a new file
//char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    error("couldnt create file");
  }

#if ECHO_TO_SERIAL  
  Serial.print("Logging to: ");
  Serial.println(filename);  
#endif
  
  bmp.begin();
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  
  logfile.println("millis \t Ax \t Ay \t Az \t T \t P");    
#if ECHO_TO_SERIAL
  Serial.println("millis \t Ax \t Ay \t Az \t T \t P");
#endif
  
  logfile.close();
  
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);  
}

void loop(void)
{
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));

  logfile = SD.open(filename, FILE_WRITE);
    
  digitalWrite(greenLEDpin, HIGH); 

  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print("\t");    
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print("\t");  
#endif

  // print the sensor values:
  logfile.print(analogRead(xpin));
  // print a tab between values:
  logfile.print("\t");
  logfile.print(analogRead(ypin));
  // print a tab between values:
  logfile.print("\t");
  logfile.print(analogRead(zpin));
  
#if ECHO_TO_SERIAL
  // print the sensor values:
  Serial.print(analogRead(xpin));
  // print a tab between values:
  Serial.print("\t");
  Serial.print(analogRead(ypin));
  // print a tab between values:
  Serial.print("\t");
  Serial.print(analogRead(zpin));
  Serial.println();
#endif

  logfile.print("\t");
  logfile.print(bmp.readTemperature());
  logfile.print("\t");
  logfile.print(bmp.readPressure());
  logfile.println();
#if ECHO_TO_SERIAL  
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C"); 
  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");
  Serial.println();
#endif

  logfile.close();

  digitalWrite(greenLEDpin, LOW);
}
