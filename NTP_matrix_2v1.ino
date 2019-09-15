/*
 * original from //https://github.com/G6EJD/ESP8266-MAX7219-LED-4x8x8-Matrix-Clock
 * 
 * 
 * small changes by Nicu FLORICA (niq_ro), add reconnect to wi-fi server
 * http://www.arduinotehniq.com
 * https://nicuflorica.blogspot.com
 * http://arduinotehniq.blogspot.com
*/
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>  // https://github.com/markruys/arduino-Max72xxPanel
#include <time.h>

int pinCS = 12; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
const byte buffer_size = 45;
char time_value[buffer_size];

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> GPIO11/MOSI/D7  (Same Pin for WEMOS)
// CS             -> GPIO12/MISO/(D12)  (Same Pin for WEMOS)
// CLK            -> GPIO13/SCK/D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 70; // In milliseconds

int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels
const char *ssid      = "bbk2";
const char *pass  = "internet2";
byte w = 0; 
/*
char zi[7] = { 'Luni', 'Marti', 'Miercuri', 'Joi', 'Vineri', 'Sambata', 'Duminica' };
char luna[12] = { 'Ianuarie', 'Februarie', 'Martie', 'Aprilie' , 'Mai', 'Iunie', 'Iulie', 'August', 'Septembrie', 'Octombrie', 'Noiembrie', 'Decembrie' };
*/

void connect_to_WiFi() {  // We start by connecting to a WiFi network
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
/*
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
*/   
for (byte  i = 0; i <= 50; i++) 
   {
   if (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }  
    else
    i = 50;
}
    if (WiFi.status() == WL_CONNECTED) 
    {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(String(WiFi.localIP()));
    }
}

void setup() {
  Serial.begin(115200);
//  WiFi.begin(ssid,password);
connect_to_WiFi();  // We start by connecting to a WiFi network
  
  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  // See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
  setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4",1);
  matrix.setIntensity(1); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    // The first display is position upside down
  matrix.setRotation(2, 1);    // The first display is position upside down
  matrix.setRotation(3, 1);    // The first display is position upside down
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) 
    {
     connect_to_WiFi();   // We start by connecting to a WiFi network  
    }
  matrix.fillScreen(LOW);
  String time = get_time();
  time.trim();
  Serial.println(time);
  time.substring(0,5).toCharArray(time_value, 10); 
  Serial.println("HH:MM");
  Serial.println(time_value);
  //( Sun  21-07-19 ) ( PM 12:52:12 )
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH
   if (millis()/1000 % 2 == 0)  // animated second
     matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
   else
    matrix.drawChar(14,0,' ', HIGH,LOW,1); // HH: 
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write(); // Send bitmap to display
  delay(500);
  w = w + 1;
   if (w > 120)
   {
   String date = get_data();
   display_message(date); // Display time in format 'Wed, Mar 01 16:03:20 2017
   w = 0;
   }
 
} //  end main loop

void display_message(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}

String get_time(){
  time_t now;
  time(&now);
  char time_output[buffer_size];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  // Desired format: 10:03:20 
  strftime(time_output, buffer_size, "%T", localtime(&now));   
  return String(time_output); // returns 12:31:45
}

String get_data(){
  time_t now;
  time(&now);
  char time_output1[buffer_size];
  // See http://www.cplusplus.com/reference/ctime/strftime/ for strftime functions
  //  strftime(time_output1, buffer_size, "( %A %d-%m-%Y )", localtime(&now)); // english, short dscription
   strftime(time_output1, buffer_size, "( %A %d-%B-%Y )", localtime(&now)); // english, long description
  //  strftime(time_output1, buffer_size, "( %u %d-%m-%Y )", localtime(&now)); // just number
  // return String(time_output1); // returns ( Sat 20-Apr-19)  
   return String(time_output1); // returns ( Saturday 20-April-2019) 
  // return String(time_output1); // returns ( 6 20-04-2019)
}

