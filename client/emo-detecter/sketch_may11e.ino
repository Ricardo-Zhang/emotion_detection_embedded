// ArduCAM Mini demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with ArduCAM ESP8266 2MP/5MP camera.
// This demo was made for ArduCAM ESP8266 2MP/5MP Camera.
// It can take photo and send to the Web.
// It can take photo continuously as video streaming and send to the Web.
// The demo sketch will do the following tasks:
// 1. Set the camera to JPEG output mode.
// 2. if server.on("/capture", HTTP_GET, serverCapture),it can take photo and send to the Web.
// 3.if server.on("/stream", HTTP_GET, serverStream),it can take photo continuously as video
//streaming and send to the Web.

// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM ESP8266 2MP/5MP camera
// and use Arduino IDE 1.6.8 compiler or above

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include <SD.h>

#include "memorysaver.h"

//This demo can only work on OV2640_MINI_2MP or ARDUCAM_SHIELD_V2 platform.
// set GPIO16 as the slave select :
const int CS = 15;
//Version 1,set GPIO2 as the slave select :
const int SD_CS = 2;

//host
const char* host = "206.189.200.72";
const int port = 5000;

//you can change the value of wifiType to select Station or AP mode.
//Default is AP mode.
int wifiType = 0; // 0:Station  1:AP

//AP mode configuration
//Default is arducam_esp8266.If you want,you can change the AP_aaid  to your favorite name
const char *AP_ssid = "arducam_esp8266";
//Default is no password.If you want to set password,put your password here
const char *AP_password = "";

//Station mode you should put your ssid and password
const char *ssid = "The House"; // Put your SSID here
const char *password = ""; // Put your PASSWORD here

static const size_t bufferSize = 4096;
static uint8_t buffer[bufferSize] = {0xFF};
uint8_t temp = 0, temp_last = 0;
int i = 0;
bool is_header = false;

// file name counter
static int k = 0;
char str[8];

ESP8266WebServer server(80);

ArduCAM myCAM(OV2640, CS);


#define MTU_Size    2*1460  // this size seems to work best
// file sending from SD card
byte clientBuf[MTU_Size];
int clientCount = 0;
int progress = 0;
int zenklas   = 0;
int zenklas2   = 0;

// sensor
#define trigPin 12
#define echoPin 13
int sensor = 0;

int frequency=1000; //Specified in Hz
int buzzPin=0; 
int timeOn=1000; //specified in milliseconds
int timeOff=1000; //specified in millisecods

WiFiClient client;

void start_capture() {
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}

void camCapture(ArduCAM myCAM) {
//  WiFiClient client = server.client();
  const int httpPort = 5000;
  if (!client.connect(host, httpPort)){
    Serial.println("connection failed");
    return;
  }
  uint32_t len  = myCAM.read_fifo_length();
  if (len >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
  }
  if (len == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  String start_request = "";
  String end_request = "";
  start_request = start_request + 
    "\n--AaB03x\n" +
    "Content-Disposition: form-data; name=\"image\"; filename=\"CAM.JPG\"\n" + 
    "Content-Transfer-Encoding: binary\n\n";
  end_request = end_request + "\n--AaB03x--\n";
  uint16_t full_length;
  full_length = start_request.length() + len + end_request.length();
  client.println("POST /photo HTTP/1.1"); 
  client.println("Host: 206.189.200.72:5000"); 
  client.println("Authorization: Basic eno1MjQ6aGlrdWg4Zmc5NnQ=");
  client.println("Content-Type: multipart/form-data; boundary=AaB03x"); 
  client.print("Content-Length: "); 
  client.println(full_length); 
  client.print(start_request); 
//   static const size_t bufferSize = 1024; 
   // original value 4096 caused split pictures   
//   static uint8_t buffer[bufferSize] = {0xFF};   
   i = 0;
  while ( len-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buffer[i++] = temp;  //save the last  0XD9
      //Write the remain bytes in the buffer
      if (!client.connected()) break;
      client.write(&buffer[0], i);
      is_header = false;
      i = 0;
      myCAM.CS_HIGH();
      break;
    }
    if (is_header == true)
    {
      //Write image data to buffer if not full
      if (i < bufferSize)
        buffer[i++] = temp;
      else
      {
        //Write bufferSize bytes image data to file
        if (!client.connected()) break;
        client.write(&buffer[0], bufferSize);
        i = 0;
        buffer[i++] = temp;
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buffer[i++] = temp_last;
      buffer[i++] = temp;
    }
  }      
    client.println(end_request);     
    myCAM.CS_HIGH();  
}

void serverCapture() {
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  start_capture();
  Serial.println(F("CAM Capturing"));
  int total_time = 0;
  total_time = millis();
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  total_time = millis() - total_time;
  Serial.print(F("capture total_time used (in miliseconds):"));
  Serial.println(total_time, DEC);
  total_time = 0;
  Serial.println(F("CAM Capture Done."));
  total_time = millis();
  camCapture(myCAM);
  total_time = millis() - total_time;
  Serial.print(F("send total_time used (in miliseconds):"));
  Serial.println(total_time, DEC);
  Serial.println(F("CAM send Done."));
}


void connect_wifi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected. IP address:");
  Serial.println(WiFi.localIP());
}
void myCAMSaveToSDFile(){
  byte buf[256];
  static int i = 0;
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  bool is_header = false;
  File outFile;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("Star Capture"));
 while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK)){
  delay(5);
 }
 Serial.println(F("Capture Done."));  

 length = myCAM.read_fifo_length();
 Serial.print(F("The fifo length is :"));
 Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
  }
    if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }
 //Construct a file name
 k = k + 1;
 itoa(k, str, 10);
 strcat(str, ".jpg");
 //Open the new file
 outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
 if(! outFile){
  Serial.println(F("File open faild"));
  return;
 }
 i = 0;
 myCAM.CS_LOW();
 myCAM.set_fifo_burst();

while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
        buf[i++] = temp;  //save the last  0XD9     
       //Write the remain bytes in the buffer
        myCAM.CS_HIGH();
        outFile.write(buf, i);    
      //Close the file
        outFile.close();
        Serial.println(F("Image save OK."));
        is_header = false;
        i = 0;
    }  
    if (is_header == true)
    { 
       //Write image data to buffer if not full
        if (i < 256)
        buf[i++] = temp;
        else
        {
          //Write 256 bytes image data to file
          myCAM.CS_HIGH();
          outFile.write(buf, 256);
          i = 0;
          buf[i++] = temp;
          myCAM.CS_LOW();
          myCAM.set_fifo_burst();
        }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    } 
  } 
}

void setup() {
  uint8_t vid, pid;
  uint8_t temp;
#if defined(__SAM3X8E__)
  Wire1.begin();
#else
  Wire.begin();
#endif
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  // set the CS as an output:
  pinMode(CS, OUTPUT);
  // initialize SPI:
  SPI.begin();
  SPI.setFrequency(4000000); //4MHz
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(F("SPI1 interface Error!"));
    while (1);
  }
  //Initialize SD Card
  if(!SD.begin(SD_CS)){
    Serial.println(F("SD Card Error"));
//    while (1);
  }
  else
  Serial.println(F("SD Card detected!"));
   

  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
    Serial.println(F("Can't find OV2640 module!"));
  else
    Serial.println(F("OV2640 detected."));

  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();

  myCAM.OV2640_set_JPEG_size(OV2640_320x240);

delay(1000);
  myCAM.clear_fifo_flag();

  connect_wifi();

//  pinMode(rxPin, INPUT);
//  pinMode(txPin, OUTPUT);
  // set the data rate for the SoftwareSerial port
//  mySerial.begin(9600);
}
void sd2server(){
  File img_file;
  char b = 'F';
  
  img_file = SD.open(str,FILE_READ);
  int len = img_file.size();
//  Serial.print("get file");
  Serial.print(len);
  if (client.connect(host, port)){
    String start_request = "";
    String end_request = "";
    start_request = start_request + 
      "\n--AaB03x\n" +
      "Content-Disposition: form-data; name=\"image\"; filename=\"CAM.JPG\"\n" + 
      "Content-Transfer-Encoding: binary\n\n";
    end_request = end_request + "\n--AaB03x--\n";
    uint16_t full_length;
    full_length = start_request.length() + len + end_request.length();
    client.println("POST /photo HTTP/1.1"); 
    client.println("Host: 206.189.200.72:5000"); 
    client.println("Content-Type: multipart/form-data; boundary=AaB03x"); 
    client.print("Content-Length: "); 
    client.println(full_length); 
    client.print(start_request); 
    progress = 0;   
    while(img_file.available()){
      if (img_file.available()>= MTU_Size){
        clientCount = MTU_Size;
        img_file.read(&clientBuf[0],clientCount);
      }
      else{
        clientCount = img_file.available();
        img_file.read(&clientBuf[0],clientCount);
      }

      if(clientCount > 0){
        client.write((const uint8_t *)&clientBuf[0],clientCount);
        progress += clientCount;
        clientCount = 0;
        // some kind of progress output
        zenklas = (progress * 100)/img_file.size();
        if (zenklas != zenklas2){
          zenklas2 = zenklas;
          Serial.print(".");
        }
      }
    }
  client.println(end_request);
  }
  Serial.println("request sent!"); 
  delay(2000);
  while (client.available()){
    char a = client.read();
    Serial.print(a);
//    if (a=='\"'){
//      b = client.read();
//      break;
//    }
//    Serial.print(a);
  }
  // dealing with all read
  while (client.available()){
    client.read();
  }
  Serial.print("emotion outcome:");
  Serial.println(b);
  playmusic(b);
}

void playmusic(char mode){
  if (mode == '1'){
    delay(100);
    Serial.println("music 1.wav instruction sent");
    tone(buzzPin, frequency);
    delay(timeOn);
    noTone(buzzPin);
    delay(timeOff);
  }
  if (mode == '2'){
    delay(100);
    Serial.println("music 2.wav instruction sent");
    tone(buzzPin, frequency);
    delay(timeOn);
    noTone(buzzPin);
    delay(timeOff);
  }
  else{
    return;
    Serial.println("No Music");
  }
}

void loop() {
//  if (mySerial.available()<=0){
    delay(1000);
//  }
//  else{
//    char received = mySerial.read();
//    if (received =='1'){
      Serial.print("got trigger");
      myCAMSaveToSDFile();
      delay(2000);
      sd2server();
      delay(15000);
//    }
//  }
}

