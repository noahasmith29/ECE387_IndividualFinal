//The api code was modified to work with the Arduino ethernet shield as it was
//orginally designed for the MKR1000. 
//Npt code was modified to give the desired time values not UTC time
//Combination and implementation of combining the api code and Npt code created by me
//LED and button code was created by me

#include <SPI.h> 
#include <Ethernet.h>
#include <Servo.h>
#include <EthernetUdp.h>
// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02
};
// fill in an available IP address on your network here,
// for manual configuration:
IPAddress ip(192, 168, 0, 7);

// fill in your Domain Name Server address here:
IPAddress myDns(209, 18, 47, 62);

unsigned int localPort = 8888;       // local port to listen for UDP packets

char timeServer[] = "time.nist.gov"; // time.nist.gov NTP server to get the current time

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// A client instance that lets the shield communicate to a server
EthernetClient client;
 
int theHour; //int to hold the hours
int theMinutes; //int to hold the minutes
int GMTOffset = -4; //int to hold your current time zone's offset from GMT time
//open weather map api key 
String apiKeyWeather= "xxxxxxxxxxxxxxxxxxxxxxxxxxx"; 
int alarmTimeHour = 11; //written in 24 hour time
int alarmTimeMinutes = 54; //from 0-59 
String theTemp = ""; //string to hold the temperature
String windSpeed = ""; //string to hold the wind speed
String weatherInfo = ""; //string to hold all weather info pulled from server
//the city you want the weather for 
String location= "cincinnati,US"; //another example: "yakutsk,RU"
//connection to the api server openweathermap.org
char server[] = "api.openweathermap.org";   
  
const int blueLEDPin = 4;    // RGBLED connected to digital pin 11
const int greenLEDPin = 5;    // RGBLED connected to digital pin 9
const int redLEDPin = 6;     // RGBLED connected to digital pin 10
const int yellowLED = 8; // yellowLED connected to digital pin 8
const int alarmPin = 3;  // alarmPinLED connected to digital pin 3
const int alarmButton = 7; // button for alarm pin connected to digital pin 7
boolean alarmPressed = false; //boolean to determine if button has been pressed

const int weatherButton = 2;   // button for weather connected to digital pin 2
int buttonState = LOW;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
int weatherReadState = HIGH;         // the current state of the output pin


void setup() { 
 // set all pinModes
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
  pinMode(alarmPin, OUTPUT);
  pinMode(alarmButton, INPUT);
  pinMode(weatherButton, INPUT);
  pinMode(yellowLED, OUTPUT);

   // start serial port:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  // give the ethernet module time to boot up:
  delay(1000);
  // start the Ethernet connection using a fixed IP address and DNS server:
  Ethernet.begin(mac, ip, myDns);
  // print the Ethernet board/shield's IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  // connect the Udp to the local port
  Udp.begin(localPort);
}


void loop() {
 //on every iteration of the loop, get the current time 
 getTime();
 //print the current time in hours and minutes
 Serial.print("Hour: " ); 
 Serial.println(theHour);
 Serial.print("Minutes: ");
 Serial.println(theMinutes);
 //turn on the alarm if the alarm time matches the current time and the alarm has not been pressed
 if(theHour == alarmTimeHour && theMinutes == alarmTimeMinutes && !alarmPressed){
  digitalWrite(3, HIGH);
 }
 //if the alarmbutton has been pressed, turn off the alarm
 if(digitalRead(alarmButton) == LOW){
  digitalWrite(3, LOW);
  alarmPressed = true;
 }
 // read the state of the switch into a local variable:
  int reading = digitalRead(weatherButton);

  //if the button state has changed, change the state of weatherReadState
  if (reading != buttonState) {
    buttonState = reading;
    if (buttonState == HIGH) {
      weatherReadState = !weatherReadState;
    }
    lastButtonState = reading;
  }
  //used for debugging, prints out the current state of the weather button
  Serial.print("weatherButtonState: ");
  Serial.println(weatherReadState);
  //if the switch is on, read the weather and display it appropriately
  if(weatherReadState == HIGH){
    getWeather();
    displayWeather(); 
  }
  //if the switch is off stop displaying the weather 
  if(weatherReadState == LOW){
         setColor(0,0,0);
         digitalWrite(yellowLED, LOW);
  }
}    

//method that gets weather from api server
void getWeather() { 
 Serial.println("\nStarting connection to server...");  
 if (client.connect(server, 80)) { 
   // if you get a connection, report back via serial: 
   Serial.println("connected to server"); 
   // Make a HTTP request: 
   client.print("GET /data/2.5/weather?"); 
   client.print("q="+location);
   client.print("&appid="+apiKeyWeather);  
   client.println("&units=imperial"); 
   client.println("Host: api.openweathermap.org"); 
   client.println("Connection: close"); 
   client.println(); 
 } else { 
   Serial.println("unable to connect"); 
 }
 //wait a second for information to come in
  delay(1000); 
 weatherInfo = ""; //reset the weather info
 while (client.connected()) {
   //set the string weatherInfo to the incoming string from ther server 
   weatherInfo = client.readStringUntil('\n'); 
   //Serial.println(weatherInfo); //use this to debug incoming string
   //find the location of the temperature in the string
   int locTempStart = weatherInfo.indexOf("temp") + 6; 
   int locTempEnd = weatherInfo.indexOf("pressure") - 2;
   //get the temperature from the string
   theTemp = weatherInfo.substring(locTempStart, locTempEnd);
   //find the location of the wind speed in the string
   int locWindStart = weatherInfo.indexOf("speed") + 7;
   int locWindEnd = weatherInfo.indexOf("deg") - 2;
   //get the windspeed from the string
   windSpeed = weatherInfo.substring(locWindStart, locWindEnd);
   //print out the temperature and windspeed
   Serial.println("The temperature: "+theTemp+ " Farenheit");
   Serial.println("The wind speed: "+windSpeed+ " mph");
  }
   //tell the ethernet shield to end the call to the server so it can call again  
   client.stop();
 }

 //method to get the current time from internet
 void getTime(){
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // now convert NTP time into everyday time:
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    //Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    theHour = ((epoch  % 86400L) / 3600)+GMTOffset; //gets the current hour in 24 hour time, converting to your time zone
    //if the hour becomes negative, wrap it back around
    if(theHour<0){
      theHour = theHour+24;
    }
    //get the current minutes 
    theMinutes = (epoch  % 3600) / 60;
    //wait a second and then maintain the ethernet shield's connection to the server
    delay(1000);
    Ethernet.maintain();
  }
}

//method that utilizes the weather info to display it visually
void displayWeather(){
  double castTemp = theTemp.toDouble(); //convert the string of the temperature to a double
  double castWind = windSpeed.toDouble();//convert the string of the wind speed to a double

  //turns on the LEDs and sets the color depending on the temperature read
  if(castTemp<20.00){
    setColor(0,0,255);
    digitalWrite(yellowLED, LOW);
  }else if(castTemp<50.00){
    setColor(170,0,255);
    digitalWrite(yellowLED, LOW);
  }else if(castTemp<80.00){
    digitalWrite(yellowLED, HIGH);
  }else{
    setColor(255,0,0);
    digitalWrite(yellowLED, LOW);
  }

// can display something with the wind speed info
//  if(castWind > 24.99){
//    
//  }
  
}
// send an NTP request to the time server at the given address
void sendNTPpacket(char* address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

//function to set RGB led colors
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redLEDPin, redValue);
  analogWrite(greenLEDPin, greenValue);
  analogWrite(blueLEDPin, blueValue);
}

