//----------------------------------------//
//  gps.ino
//
//  created 06/09/2018
//  update to esp32 - 05/06/2019
//  by Luiz H. Cassettari
//----------------------------------------//


#include <TinyGPS++.h>
#include <HardwareSerial.h>

#if defined(GPS_RX_PIN) && defined (GPS_TX_PIN)
// use pins defined in pins.h
#define RXPin GPS_RX_PIN
#define TXPin GPS_TX_PIN
#else
// default pins are 34 and 35
#define RXPin 34
#define TXPin 35
#endif

#define GPSBaud 9600

// uncomment next line to test with a fake location
// #define FAKE_GPS 1

#define FAKE_GPS_LAT -27.565886
#define FAKE_GPS_LON 152.937851
#define FAKE_GPS_ALT 11.700000

TinyGPSPlus gps;
HardwareSerial ss(1);

void gps_setup(){
  ss.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
  Serial.print(F("TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println();
}

bool gps_loop(){
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }
  if (runEvery_gps(5000)) 
  {
    Serial.print(F("GPS timestamp: "));
    Serial.println(gps_time() + " " + gps_date());
    return true;
  }
  return false;
}

boolean gps_read(){
#ifdef FAKE_GPS
  return true;
#else
  return (gps.location.isValid());
#endif
}

float gps_latitude(){
#ifdef FAKE_GPS
  return FAKE_GPS_LAT;
#else
  return gps.location.lat();
#endif
}
float gps_longitude(){
#ifdef FAKE_GPS
  return FAKE_GPS_LON;
#else
  return gps.location.lng();
#endif
}

float gps_meters() {
#ifdef FAKE_GPS
  return FAKE_GPS_ALT;
#else  
  return gps.altitude.meters();
#endif
}

float gps_HDOP(){
  if (gps.hdop.isValid())
    return gps.hdop.hdop();
  else
    return 40.0;
}

String gps_location()
{
  if (gps.location.isValid())
  {
    String str = "";
    str += gps_latitude();
    str += " ";
    str += gps_longitude();
    return str;
  }
  else
  {
    return "#### ####";
  }  
}

String gps_date()
{
  if (gps.date.isValid())
  {
    int d = gps.date.day();
    int m = gps.date.month();
    int y = gps.date.year();
    String str = "";
    if (d < 10) str += "0";
    str += d;
    str += "/";
    if (m < 10) str += "0";
    str += m;
    str += "/";
    str += y;
    return str;
  }
  else
  {
    return "xx/xx/xxxx";
  }  
}

String gps_time()
{
  if (gps.time.isValid())
  {
    int s = gps.time.second();
    int m = gps.time.minute();
    int h = gps.time.hour();
    String str = "";
    if (h < 10) str += "0";
    str += h;
    str += ":";
    if (m < 10) str += "0";
    str += m;
    str += ":";
    if (s < 10) str += "0";
    str += s;
    return str;
  }
  else
  {
    return "xx:xx:xx";
  }  
}

boolean runEvery_gps(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}


void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }


  Serial.println();
}
