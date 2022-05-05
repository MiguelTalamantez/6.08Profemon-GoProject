#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <string.h>  //used for some string handling and processing.
#include <mpu6050_esp32.h>
#include <WiFiClient.h>

#include<math.h>

const uint8_t BUTTON1 = 45;
const uint8_t BUTTON2 = 39;
const uint8_t BUTTON3 = 18;

const char NETWORK[] = "MIT";
const char PASSWORD[] = "";
const char user[] = "Pikachu";

WiFiClient client; //global WiFiClient Secure object
WiFiClient client2; //global WiFiClient Secure object

TFT_eSPI tft = TFT_eSPI(); 

int state;
const int IDLE = 0;
const int BUFFER = 1;
const int VIEWING = 2;
const int BUFFER2 =3;

int idx;
int offset;
int firsttime;

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 4000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 4000; //size of buffer to hold HTTP response
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request

uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.

void setup() {
  // put your setup code here, to run once:
  tft.init();  //init screen
  tft.setRotation(1); //adjust rotation
  tft.setTextSize(1); //default font size
  tft.fillScreen(TFT_BLACK); //fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); //set color of font to green foreground, black background
  Serial.begin(115200); //begin serial comms
  delay(100); //wait a bit (100 ms)
  Wire.begin();
  
  WiFi.begin(NETWORK, PASSWORD);
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(NETWORK);
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n",WiFi.localIP()[3],WiFi.localIP()[2],
                                            WiFi.localIP()[1],WiFi.localIP()[0], 
                                          WiFi.macAddress().c_str() ,WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  
  pinMode(BUTTON1, INPUT_PULLUP); 
  pinMode(BUTTON2, INPUT_PULLUP); 
  pinMode(BUTTON3, INPUT_PULLUP);
  
  idx = 0;
  firsttime = 1;
}

void loop() {
  uint8_t button1 = digitalRead(BUTTON1);
  uint8_t button2 = digitalRead(BUTTON2);
  uint8_t button3 = digitalRead(BUTTON3);

  switch_state(button1);

  if(state == VIEWING){
    profedex_navigator(button2, button3);
  }
}

void switch_state(int button){
  switch(state){
    case IDLE:
      if(button == 0){
        state = BUFFER;
      }
      break;
    case BUFFER:
      if(button == 1){
        state = VIEWING;
      }
      break; 
    case VIEWING:
      if(button == 0){
        state = BUFFER2;
      }
      break;
    case BUFFER2:
      if(button == 1){
        state = IDLE;
      }
      break;
  }
}

void profedex_navigator(int button_forward, int button_backward){
  if(button_forward == 0 || button_backward == 0 || firsttime == 1){
    firsttime = 0;

    if(button_forward == 0){
      idx += 1;
    }
    if(button_backward == 0){
      idx -= 1;
    }
    if(idx < 0){
      idx = 0;
    }
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);

    strcpy(request, "");
    request[0] = '\0'; //set 0th byte to null
    offset = 0; //reset offset variable for sprintf-ing
    offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team3/profedex_viewing.py?index=%f&user=%s  HTTP/1.1\r\n", float(idx), user);
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
    offset += sprintf(request + offset, "\r\n");
    Serial.println(request);
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    Serial.println("-----------");
    Serial.println(response);
    Serial.println("-----------");

    tft.println(response);
    delay(1000);
  }
}

void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  if (client2.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client2.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client2.connected()) { //while we remain connected read out data coming back
      client2.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client2.available()) { //read out remaining text (body of response)
      char_append(response,client2.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client2.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client2.stop();
  }
}  

uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

