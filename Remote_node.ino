//V 1.0
//dust 14 sec
// co 60 sec
//taking more time, need to refine the code
// takes 1.5 - 2 min

#include "MQ135.h"
#include <Wire.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;

const int Mq_135_sen = A0;
const int Mq_07_out = A1;
const int Mq_07_vcc = A4;
long start = 0;
long target = 0;
long current = 0;
String data1 = "0000";
String data2 = "0000";
String inputString = ""; // a string to hold incoming data
boolean stringComplete = false; // whether the string is complete
String signal = "$GPGLL";
#define USE_AVG

// Arduino pin numbers.
const int sharpLEDPin = 7;   // Arduino digital pin 7 connect to sensor LED.
const int sharpVoPin = A7;   // Arduino analog pin 5 connect to sensor Vo.


//GSM module settings
String apn = "airtelgprs.com";                       //APN
String apn_u = "";                     //APN-Username
String apn_p = "";                     //APN-Password
//String url = "http://beacomp.000webhostapp.com/index1.php";  //URL for HTTP-POST-REQUEST
String url="http://5a0cb159.ngrok.io/recive_data" ;
//String data1;   //String for the first Paramter (e.g. Sensor1)
//String data2;   //String for the second Paramter (e.g. Sensor2)



String LAT1 = "" ;
String LON1 = "" ;
float dustr = 0.000 ;


double longitude;
double latitude;

String textForSMS;

long current_s = 0 ;
long start_s = 0 ;
long end1 = 0 ;
long wait_1min = 0 ;

// For averaging last N raw voltage readings.
#ifdef USE_AVG
#define N 100
static unsigned long VoRawTotal = 0;
static int VoRawCount = 0;
#endif // USE_AVG

// Set the typical output voltage in Volts when there is zero dust.
static float Voc = 0.6;

// Use the typical sensitivity in units of V per 100ug/m3.
const float K = 0.5;

/////////////////////////////////////////////////////////////////////////////

// Helper functions to print a data value to the serial monitor.
void printValue(String text, unsigned int value, bool isLast = false) {
  Serial.print(text);
  Serial.print("=");
  Serial.print(value);
  if (!isLast) {
    Serial.print(", ");
  }
}
void printFValue(String text, float value, String units, bool isLast = false) {
  Serial.print(text);
  Serial.print("=");
  Serial.print(value);
  dustr = value ;
  Serial.print(units);
  if (!isLast) {
    Serial.print(", ");
  }
}

/////////////////////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Setup Started");

  Serial3.begin(9600);  // GPS
  Serial1.begin(9600);  // GSM

  pinMode(Mq_135_sen, INPUT);        //Gas sensor will be an input to the arduino
  pinMode(Mq_07_out , INPUT) ;
  pinMode(Mq_07_vcc , OUTPUT);
  analogWrite(Mq_07_vcc, 1023);

  start = millis();
  target = start + 60000;
  current = start;

  current_s = millis() ;
  start_s = current_s ;
  end1 = current_s + 10000 ;

  wait_1min = current + 65000 ;  // to get all reading first time


  pinMode(sharpLEDPin, OUTPUT);
  Serial.println("");
  delay(5000) ;
  Serial.println("SETUP COMPLETE");
  Serial.println("=================");
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);


} // setup end

double No2 = 0.0;
double CO = 0.0;
String dust_s = "";

void loop() {
  No2 = get_No();
  CO = get_CO();
  dust();
  data1 = String( No2 , 6);
  data2 = String( CO , 6 );
  dust_s = String(dustr , 6) ;
  Serial.println("d1  " + data1);
  Serial.println("d2          " + data2);
  Serial.println("lat                    " + LAT1 + "  " + LON1);

  Serial.println(dustr, 6);
//  /dust();
  //  delay(400);
  //  Serial.print(dustr);
  Serial.println("Dust                                     " + dust_s) ;
  // lo_la_NO2_Co_dust
//   data1 + data2 + // dust_s +LAT1 + LON1


  get_location_and_send();
}



void get_location_and_send()
{
  displayInfo();

  current_s = millis() ;
  if (current_s > end1)
  {
    end1 = current_s + 10000 ;

    displayInfo();

    latitude = gps.location.lat(), 6 ;
    Serial.print(latitude) ;
    Serial.print("        ");
    longitude = gps.location.lng(), 6 ;
    Serial.println(longitude) ;

    String lo = String( longitude , 6) ;
    Serial.print("lo :" + lo);
    String la = String( latitude , 6) ;
    Serial.println("       la :" + la);

    // for latitude
    long datareal = int(latitude);
    int fahad = ( latitude - datareal) * 100000;

    // for longitude
    long datareal2 = int(longitude);
    int fahad2 = (longitude - datareal2 ) * 100000;

    //    textForSMS = "_";
    //    textForSMS.concat(datareal2);
    //    textForSMS = textForSMS + ".";
    //    textForSMS.concat(fahad2);
    //    textForSMS = textForSMS + "_";
    //    textForSMS.concat(datareal);
    //    textForSMS = textForSMS + ".";
    //    textForSMS.concat(fahad);
    //    textForSMS = textForSMS + "_";

    textForSMS = "_";
    textForSMS = textForSMS + lo;
    textForSMS = textForSMS + "_";
    textForSMS = textForSMS + la;
    textForSMS = textForSMS + "_";
    textForSMS = textForSMS + data1;
    textForSMS = textForSMS + "_";
    textForSMS = textForSMS + data2;
    textForSMS = textForSMS + "_";
    textForSMS = textForSMS + dust_s;
    textForSMS = textForSMS + "_";
///lo_la_data1_d2_dust
    if (wait_1min < millis())
    {
      wait_1min = millis() + 20000 ;
      send_data() ;
    }
    Serial.println(textForSMS);
    Serial.println("message sent.");
    delay(1000);

  } // if end

  while (Serial3.available() > 0)
    if (gps.encode(Serial3.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }

}




void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    Serial.print(" ");
    Serial.print(F("Speed:"));
    Serial.print(gps.speed.kmph());
  }
  else
  {
    Serial.print(F("INVALID"));
  }
/*
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
//////////////////
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
*/
  Serial.println();
}





void dust()
{
  // Turn on the dust sensor LED by setting digital pin LOW.
  digitalWrite(sharpLEDPin, LOW);

  // Wait 0.28ms before taking a reading of the output voltage as per spec.
  delayMicroseconds(280);

  // Record the output voltage. This operation takes around 100 microseconds.
  int VoRaw = analogRead(sharpVoPin);

  // Turn the dust sensor LED off by setting digital pin HIGH.
  digitalWrite(sharpLEDPin, HIGH);

  // Wait for remainder of the 10ms cycle = 10000 - 280 - 100 microseconds.
  delayMicroseconds(9620);

  // Print raw voltage value (number from 0 to 1023).
#ifdef PRINT_RAW_DATA
  //  printValue("VoRaw", VoRaw, t/rue);
  Serial.println("in");
#endif // PRINT_RAW_DATA

  // Use averaging if needed.
  float Vo = VoRaw;
#ifdef USE_AVG
  VoRawTotal += VoRaw;
  VoRawCount++;
  if ( VoRawCount >= N ) {
    Vo = 1.0 * VoRawTotal / N;
    VoRawCount = 0;
    VoRawTotal = 0;
  } else {
    return;
  }
#endif // USE_AVG

  // Compute the output voltage in Volts.
  Vo = Vo / 1024.0 * 5.0;
  //  printFValue("Vo", Vo * 1000.0, "mV");

  // Convert to Dust Density in units of ug/m3.
  float dV = Vo - Voc;
  if ( dV < 0 ) {
    dV = 0;
    Voc = Vo;
  }
  float dustDensity = dV / K * 100.0;
  dustr = dustDensity;

  //  printFValue("DustDensity", dustDensity, "ug/m3", true);
  Serial.println("dust_r *******                 ***********");
  delay(1000);

}

double get_No()
{
  MQ135 gasSensor = MQ135(Mq_135_sen);

  float No_2 = gasSensor.getPPM();
  // Serial.print("Mq135 :") ;
  //  Serial.println(analogRead(Mq_135_sen)) ;
  No_2 = No_2 * 1000 ;
  //Serial.println(No_2) ;
  return No_2;
}


double ppm_CO()
{
  analogWrite(Mq_07_vcc, 300);
  current = millis();
  int in = analogRead(Mq_07_out);
  //Serial.print("IN :");
  //Serial.println(in);
  double ppm = 3.027 * exp(1.0698 * (in * 3.3 / 1023));
  Serial.print(" CO : PPM:                                                   ****");
  Serial.println(ppm);
  analogWrite(Mq_07_vcc, 1023);
  delay(2000);
  return ppm;
}


double get_CO()
{
  //  Serial.print("Cur :");
  //  Serial.println(current)+" "+ target);

  if ( current < target)
  {
    current = millis();
  }
  else
  {
    start = millis();
    target = start + 60000;
    current = start;
    return (ppm_CO());
  }
  return CO;
}

void gps1()
{
  serialEvent();
  if (stringComplete) {
    String BB = inputString.substring(0, 6);
    if (BB == signal) {
      String LAT = inputString.substring(7, 17);
      int LATperiod = LAT.indexOf('.');
      int LATzero = LAT.indexOf('0');
      if (LATzero == 0) {
        LAT = LAT.substring(1);
      }

      String LON = inputString.substring(20, 31);
      int LONperiod = LON.indexOf('.');
      int LONTzero = LON.indexOf('0');
      if (LONTzero == 0) {
        LON = LON.substring(1);
      }
      Serial.print("Lat :");
      Serial.println(LAT);
      LAT1 = LAT;
      Serial.print("Lon :");
      Serial.println(LON);
      LON1 = LON;
    }

    // Serial3.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }

}


void serialEvent() {
  while (Serial3.available()) {
    // get the new byte:

    char inChar = (char) Serial3.read();
    // add it to the inputString:
    //        Serial.println(inChar);
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}



void send_data() {
  Serial.print("AT");
  Serial1.println("AT");
  runsl();//Print GSM Status an the Serial Output;
  delay(4000); //4000
  Serial1.println("AT+SAPBR=3,1,Contype,GPRS");
  runsl();
  delay(100);
  Serial1.println("AT+SAPBR=3,1,APN," + apn);
  runsl();
  delay(100);
  Serial1.println("AT+SAPBR=3,1,USER," + apn_u); //Comment out, if you need username
  runsl();
  delay(100);
  Serial1.println("AT+SAPBR=3,1,PWD," + apn_p); //Comment out, if you need password
  runsl();
  delay(100);
  Serial1.println("AT+SAPBR =1,1");
  runsl();
  delay(100);
  Serial1.println("AT+SAPBR=2,1");
  runsl();
  delay(2000);
  Serial1.println("AT+HTTPINIT");
  runsl();
  delay(100);
  Serial1.println("AT+HTTPPARA=CID,1");
  runsl();
  delay(100);
  Serial1.println("AT+HTTPPARA=URL," + url);
  runsl();
  delay(100);
  Serial1.println("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  runsl();
  delay(100);
  Serial1.println("AT+HTTPDATA=192,10000");
  runsl();
  delay(100);
  //  Serial1.println("params=" + data1 + "-" + data2);
  Serial1.println("params=" + textForSMS);
  runsl();
  delay(10000);
  Serial1.println("AT+HTTPACTION=1");
  runsl();
  delay(5000);
  Serial1.println("AT+HTTPREAD");
  runsl();
  delay(100);
  Serial1.println("AT+HTTPTERM");
  runsl();
}

//Print GSM Status
void runsl() {
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}


void gsm_sendhttp() {
  Serial.print("AT");
  Serial1.println("AT");
  print_sim();//Print GSM Status an the Serial Output;
  delay(4000);
  Serial1.println("AT+SAPBR=3,1,Contype,GPRS");
  print_sim();
  delay(100);
  Serial1.println("AT+SAPBR=3,1,APN," + apn);
  print_sim();
  delay(100);
  Serial1.println("AT+SAPBR=3,1,USER," + apn_u); //Comment out, if you need username
  print_sim();
  delay(100);
  Serial1.println("AT+SAPBR=3,1,PWD," + apn_p); //Comment out, if you need password
  print_sim();
  delay(100);
  Serial1.println("AT+SAPBR =1,1");
  print_sim();
  delay(100);
  Serial1.println("AT+SAPBR=2,1");
  print_sim();
  delay(2000);
  Serial1.println("AT+HTTPINIT");
  print_sim();
  delay(100);
  Serial1.println("AT+HTTPPARA=CID,1");
  print_sim();
  delay(100);
  Serial1.println("AT+HTTPPARA=URL," + url);
  print_sim();
  delay(100);
  Serial1.println("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  print_sim();
  delay(100);
  Serial1.println("AT+HTTPDATA=192,10000");
  print_sim();
  delay(100);
  Serial1.println("params=" + data1 + "-" + data2);
  print_sim();
  delay(10000);
  Serial1.println("AT+HTTPACTION=1");
  print_sim();
  delay(5000);
  Serial1.println("AT+HTTPREAD");
  print_sim();
  delay(100);
  Serial1.println("AT+HTTPTERM");
  print_sim();

  Serial.println("DATA SEND");
}

//Print GSM Status
void print_sim() {
  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }

}


