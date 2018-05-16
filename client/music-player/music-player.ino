#include <ArduinoHttpClient.h>
#include <WiFi101.h>
#include <SD.h>
#include <SPI.h>
#include <AudioZero.h>

#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// Wifi Settings ///////
char ssid[] = "Yan's iPhone";
char pass[] = "950424xin";


char serverAddress[] = "206.189.200.72";  // server address
int port = 5000;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;
String response;
int statusCode = 0;

void setup() {
  Serial.begin(9600);
  analogWrite(DAC0,0);
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println(" failed!");
    while(true);
  }
  Serial.println(" done.");

  // 44100kHz stereo => 88200 sample rate
  AudioZero.begin(16000);
}

void loop() {
  Serial.println("making GET request");
  client.get("/data");

  // read the status code and body of the response
  statusCode = client.responseStatusCode();
  response = client.responseBody();
  String* pointer = &response;
  Serial.println(response.charAt(1));
  char res = response.charAt(1);
  if (res != '0'){
     play_music(res);
  }
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println("Wait five seconds");
  delay(5000);
  
}

void play_music(char i){
    File myFile;
    if (i == '1'){
      myFile  = SD.open("1.wav");
    }
     else{
       myFile  = SD.open("2.wav");
     }
    if (!myFile) {
    // if the file didn't open, print an error and stop
      Serial.println("error opening a.wav");
      return;
    }

    Serial.print("Playing");
    AudioZero.play(myFile);
    Serial.println("End of file.");
    return;
  }
