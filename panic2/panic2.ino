#include <mpu6050_esp32.h> //Used for ESP32 stuff
#include <math.h>
#include <string.h> //used for some string handling and processing.
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <WiFi.h> //Connect to WiFi Network
#include <stdlib.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <cstring>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
MPU6050 imu; //imu object called, appropriately, imu
WiFiClientSecure client; //global WiFiClient Secure object

//Button Setup
  const int BUTTON1 = 45; //Button1 at pin 45
  const int BUTTON2 = 39; //Button2 at pin 39
  const int BUTTON3 = 38; //Button3 at pin 38
  const int BUTTON4 = 34; //Button4 at pin 34
  uint8_t button1; // for reading buttons
  uint8_t button2;
  uint8_t button3;
  uint8_t button4;
  uint8_t old_button1; // for storing old vals
  uint8_t old_button2;
  uint8_t old_button3;
  uint8_t old_button4;

//State machine variables
  int overallstate = 0;
  int mapstate = 0;
  int catchstate = 0;
  bool battle_or_catch = false; // battle = true, catch = false

//Miscl variables
  bool displaytext = true;
  bool askifcatch = false;
  bool catchbuttons = false;
  float userlocationx =   -71.101543; //Simmons     -71.092526;//Bridge     -71.097723;//Dlab
  float userlocationy =   42.357113; //Simmons      42.357008;//Bridge      42.361863;//Dlab
  float testproflocationx = -71.102543; //092482
  float testproflocationy = 42.357113; //360263
  float deltax = 0;
  float deltay = 0;
  float rdistance = 0;

//Location to TFT variables
  float tft_x_min_geolocation = -71.108093;
  float tft_y_min_geolocation = 42.351304;
  float tft_x_max_geolocation = -71.080977;
  float tft_y_max_geolocation = 42.367074;
  float y_change_geolocation = 0.01577;
  float x_change_geolocation = 0.026316;
  int tft_x_min_LCD = 0;
  int tft_y_min_LCD = 0;
  int tft_x_max_LCD = 160;
  int ftf_y_max_LCD = 128;
  int userdisplaytimer = 2000;
  uint8_t flipped_map[2560];

//WIFI/GETREQUEST-Related **Variables**
  const uint16_t RESPONSE_TIMEOUT = 6000;
  const uint16_t IN_BUFFER_SIZE = 4000; //size of buffer to hold HTTP request
  const uint16_t OUT_BUFFER_SIZE = 4000; //size of buffer to hold HTTP response
  const uint16_t JSON_BODY_SIZE = 3000;
  char request[IN_BUFFER_SIZE];
  char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
  char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
  char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
  char json_body[JSON_BODY_SIZE];
  const int BUTTON_TIMEOUT = 1000; //button timeout in milliseconds

//WIFI-Related **CONSTANTS**
  const char PREFIX[] = "{\"wifiAccessPoints\": ["; //beginning of json body
  const char SUFFIX[] = "]}"; //suffix to POST request
  const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //don't change this and don't share this
  const uint8_t BUTTON = 45;
  const int MAX_APS = 5;
  const char NETWORK[] = "EECS_Labs";
  const char PASSWORD[] = "";
  uint8_t channel = 1; //network channel on 2.4 GHz
  byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.
  int offset = 0;

  const char AUDIO_PREFIX[] = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\",\"speechContexts\":[{\"phrases\":[\"riemann\", \"jacobian\"]}]}, \"audio\": {\"content\":\"";
  const char AUDIO_SUFFIX[] = "\"}}"; //suffix to POST request_buffer
  const int AUDIO_IN = 1; //pin where microphone is connected

//WIFI-Related **Global Variables**
  uint8_t button_state; //used for containing button state and detecting edges
  int old_button_state; //used for detecting button edges
//   uint32_t time_since_sample;      // used for microsecond timing
  StaticJsonDocument<500> doc;

//WIFI Miscl
  int wifi_object_builder(char* object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t* mac_address) {
    char buffer[100];
    int num_chars = sprintf(buffer, "{\"macAddress\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"signalStrength\":%d,\"age\":0,\"channel\": %d}", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5], signal_strength, channel);
    if (num_chars < os_len) {
      sprintf(object_string, "%s", buffer);
      return num_chars;
    }
    return 0;
  }
  char*  SERVER = "googleapis.com";  // Server URL
  uint32_t timer;

// Acceleration Variables
float x, x_avg, x_old, x_older;
float y, y_avg, y_old, y_older;
float z, z_avg, z_old, z_older;
const float ZOOM = 9.81; //for display (converts readings into m/s^2)...used for visualizing only

// State Machine Variables
enum game_mode {Map, Catch};
game_mode game_state = Map;
enum idle_mode {Idle, Yes, No};
idle_mode idle_state = Idle;
uint32_t ask_timer;
enum catch_mode {Catch_Idle, Quitting, Waiting, Motion, Throw_End, Catch_Fail, Catch_Success};
uint32_t state_timer = 0;
catch_mode catch_state = Catch_Idle; //state variable

// Catch Mode Variables
uint32_t cycle_timer;
uint32_t peak_timer = -1;
uint8_t motion = 0;
uint16_t swing_counter = 0;
uint32_t swing_timer;
uint32_t swing_times[30];
bool swing_in_progress = false;

// Game/Display Variables
const char user[] = "Pikachu";
uint8_t change_display = 0;
uint8_t new_profemon = 0;
char display_name[50] = "";
char profemon_name[50] = "";
char *profemon_data_labels[5] = {"type", "hp", "attack", "moves"};
char profemon_data[5][20] = {""};
char profedex_data[1000];
char* begin_profedex = 0;

// Location Variables
double latitude = 0.0;
double longitude = 0.0;
uint32_t loc_timer = 0;

// audio variables
const int DELAY = 1000;
const int SAMPLE_FREQ = 8000;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 5;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES = SAMPLE_FREQ * SAMPLE_DURATION;  // number of of samples
const int ENC_LEN = (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4;  // Encoded length of clip
bool conn;
uint32_t time_since_sample;      // used for microsecond timing
char speech_data[ENC_LEN + 200] = {0}; //global used for collecting speech data
char heard[200];

// overall battle variables
uint8_t b1;
uint8_t b2;
uint8_t b3;
uint8_t old_b1_input;
uint8_t old_b2_input;
uint8_t old_b3_input;

uint8_t num_turns;

uint8_t gesture;
uint8_t battle_state;

char profemon_id[50];
uint8_t game_id;
char standard_move_name[50];
char special_move_name[50];

uint8_t old_player_hp;
uint8_t player_hp;
uint8_t other_hp;
uint8_t max_attack;
char last_move[50];
uint8_t damage_done;

uint32_t time_since_turn_check; 
const uint32_t turn_check_period = 500;
uint32_t time_since_turn_started;
const uint32_t max_turn_length = 300000;

const uint8_t start = 0;
const uint8_t turn = 1;
const uint8_t attack = 2;
const uint8_t forfeit = 3;

// gesture recognition variables
int gesture_duration = 5000;

int battlemode;
const int x_shake = 1;
const int y_shake = 2;
const int z_shake = 3;

int start_time;

int gesture_total;

//PROFEDEX Variables
int state;
const int IDLE = 0;
const int BUFFER = 1;
const int VIEWING = 2;
const int BUFFER2 =3;
int idx;
// int offset;
int firsttime;

void setup() {
    
  //LCD + SERIAL SETUP
    tft.init();
    tft.setRotation(1);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    Serial.begin(115200);
    delay(50); //pause to make sure comms get set up
    Wire.begin();
    delay(50); //pause to make sure comms get set up
    Serial.println("Welcome to Prof-emons Go! - Serial View");

  //IMU Setup
    if (imu.setupIMU(1)) {
      Serial.println("IMU Connected!");
    } else {
      Serial.println("IMU Not Connected :/");
      Serial.println("Restarting");
      ESP.restart(); // restart the ESP (proper way)
      }
  
  
  //Button setup
    pinMode(BUTTON1, INPUT_PULLUP); //set input pin as an input button1
    pinMode(BUTTON2, INPUT_PULLUP); //set input pin as an input button2
    pinMode(BUTTON3, INPUT_PULLUP); //set input pin as an input button3
    pinMode(BUTTON4, INPUT_PULLUP); //set input pin as an input button4
    delay(4000);
  
  //Profedex Related Setup
  idx = 0;
  firsttime = 1;


  //SCAN WIFI AND CONNECT
    tft.printf("Scanning Wifi Networks...\n");
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    tft.printf("Analyzing Wifi Options...\n");
    if (n == 0) {
      Serial.println("no networks found");
    } else {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i) {
        Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
        uint8_t* cc = WiFi.BSSID(i);
        for (int k = 0; k < 6; k++) {
          Serial.print(*cc, HEX);
          if (k != 5) Serial.print(":");
          cc++;
        }
        Serial.println("");
      }
    }
    delay(100); //wait a bit (100 ms)

    //if using regular connection use line below:
    WiFi.begin(NETWORK, PASSWORD);
    //if using channel/mac specification for crowded bands use the following:
    // WiFi.begin(network, password, channel, bssid);

    uint8_t count = 0; //count used for Wifi check times
    Serial.print("Attempting to connect to ");
    Serial.println(NETWORK);
    while (WiFi.status() != WL_CONNECTED && count < 12) {
      delay(500);
      Serial.print(".");
      count++;
    }
    delay(2000);
    if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
      Serial.println("CONNECTED!");
      tft.printf("Connected to WIFI!\n");
      Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                    WiFi.localIP()[1], WiFi.localIP()[0],
                    WiFi.macAddress().c_str() , WiFi.SSID().c_str());
      delay(500);
    } else { //if we failed to connect just Try again.
      Serial.println("Failed to Connect :/  Going to restart");
      Serial.println(WiFi.status());
      ESP.restart(); // restart the ESP (proper way)
    }
    timer = millis();
    loc_timer = millis();
    tft.fillScreen(TFT_BLACK);
}

void loop() {
  // Serial.println(overallstate);
  //READ DATA
    uint8_t button1 = digitalRead(BUTTON1);
    uint8_t button2 = digitalRead(BUTTON2); 
    uint8_t button3 = digitalRead(BUTTON3);
    uint8_t button4 = digitalRead(BUTTON4);
    
    if (button2 == 0) {
        if (millis() - userdisplaytimer > 10000){
        Serial.println("Location Updating");
        userdisplaytimer = millis();
        //GET LOCATION VIA GEOLOCATION
          int offset = sprintf(json_body, "%s", PREFIX);
          int n = WiFi.scanNetworks(); //run a new scan. could also modify to use original scan from setup so quicker (though older info)
          Serial.println("scan done");
          if (n == 0) {
            Serial.println("no networks found");
          } else {
            //tft.fillScreen(TFT_BLACK);
            tft.setCursor(0,120);
            tft.println("Updating location..."); 
            int max_aps = max(min(MAX_APS, n), 1);
            for (int i = 0; i < max_aps; ++i) { //for each valid access point
              uint8_t* mac = WiFi.BSSID(i); //get the MAC Address
              offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE-offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); //generate the query
              if(i!=max_aps-1){
                offset +=sprintf(json_body+offset,",");//add comma between entries except trailing.
              }
            }
            sprintf(json_body + offset, "%s", SUFFIX);
            Serial.println(json_body);
            int len = strlen(json_body);
            // Make a HTTP request:
            Serial.println("SENDING REQUEST");
            Serial.println("main loop");
            response[0] = '\0';
            request[0] = '\0'; //set 0th byte to null
            offset = 0; //reset offset variable for sprintf-ing
            offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
            offset += sprintf(request + offset, "Host: googleapis.com\r\n");
            offset += sprintf(request + offset, "Content-Type: application/json\r\n");
            offset += sprintf(request + offset, "cache-control: no-cache\r\n");
            offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
            offset += sprintf(request + offset, "%s", json_body);
            do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
            Serial.println("-----------");
            Serial.println(response);
            Serial.println("-----------");
        }
    }    }
    else if (button1 == 0) {
        delay(300);
        conn = false;
        for (int i = 0; i < 10; i++) {
            int val = (int)client.connect("speech.google.com", 443);//, 4000);
            Serial.print(i); Serial.print(": "); Serial.println(val);
            if (val != 0) {
            conn = true;
            break;
            }
            Serial.print(".");
            delay(300);
        }
        if (!conn) {
            client.stop();
            Serial.println("Connection failed!");
            return;
        }
        else {
            Serial.println("Connected to server!");
            Serial.println(client.connected());
            int len = strlen(speech_data);
            // Make a HTTP request_buffer:
            client.print("POST /v1/speech:recognize?key="); client.print(API_KEY); client.print(" HTTP/1.1\r\n");
            client.print("Host: speech.googleapis.com\r\n");
            client.print("Content-Type: application/json\r\n");
            client.print("cache-control: no-cache\r\n");
            client.print("Content-Length: "); client.print(len);
            client.print("\r\n\r\n");
            int ind = 0;
            int jump_size = 1000;
            char temp_holder[jump_size + 10] = {0};
            Serial.println("sending data");
            while (ind < len) {
            delay(80);//experiment with this number!
            //if (ind + jump_size < len) client.print(speech_data.substring(ind, ind + jump_size));
            strncat(temp_holder, speech_data + ind, jump_size);
            client.print(temp_holder);
            ind += jump_size;
            memset(temp_holder, 0, sizeof(temp_holder));
            }
            client.print("\r\n");
            //Serial.print("\r\n\r\n");
            //Serial.println("Through send...");
            unsigned long count = millis();
            while (client.connected()) {
            //Serial.println("IN!");
            String line = client.readStringUntil('\n');
            Serial.print(line);
            if (line == "\r") { //got header of response
                //Serial.println("headers received");
                break;
            }
            if (millis() - count > RESPONSE_TIMEOUT) break;
            }
            //Serial.println("");
            //Serial.println("Response...");
            count = millis();
            while (!client.available()) {
            delay(100);
            //Serial.print(".");
            if (millis() - count > RESPONSE_TIMEOUT) break;
            }
            Serial.println();
            //Serial.println("-----------");
            memset(response_buffer, 0, sizeof(response_buffer));
            while (client.available()) {
            char_append(response_buffer, client.read(), OUT_BUFFER_SIZE);
            }
            //Serial.println(response); //comment this out if needed for debugging
            char* trans_id = strstr(response_buffer, "transcript");
            char transcript[100] = {0};
            if (trans_id != NULL) {
            char* foll_coll = strstr(trans_id, ":");
            char* starto = foll_coll + 3; //starting index
            char* endo = strstr(starto + 1, "\""); //ending index
            int transcript_len = endo - starto;
            
            strncat(transcript, starto, transcript_len);
            Serial.printf("We heard you say: %s\n",transcript);

            memset(heard,0,200);
            strcpy(heard,transcript);
            }
            //Serial.println("-----------");
            client.stop();
            Serial.println("done");
        }
    } 
}

const char* CA_CERT = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
                      "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
                      "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
                      "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
                      "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
                      "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
                      "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
                      "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
                      "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
                      "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
                      "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
                      "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
                      "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
                      "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
                      "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
                      "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
                      "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
                      "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
                      "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
                      "-----END CERTIFICATE-----\n";



uint8_t char_append(char* buff, char c, uint16_t buff_size) {
  int len = strlen(buff);
  if (len > buff_size) return false;
  buff[len] = c;
  buff[len + 1] = '\0';
  return true;
}

void do_https_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial) {
  // client.setHandshakeTimeout(30);
  client.setCACert(CA_CERT); //set cert for https
  if (client.connect(host,443,4000)) { //try to connect to host on port 443
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    response[0] = '\0';
    //memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n', response, response_size);
      if (serial) Serial.println(response);
      if (strcmp(response, "\r") == 0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis() - count > response_timeout) break;
    }
    memset(response, 0, response_size);
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response, client.read(), OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");
  } else {
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}

void record_audio() {
  int sample_num = 0;    // counter for samples
  int enc_index = strlen(AUDIO_PREFIX) - 1;  // index counter for encoded samples
  float time_between_samples = 1000000 / SAMPLE_FREQ;
  int value = 0;
  char raw_samples[3];   // 8-bit raw sample data array
  Serial.println("THIS SHOULD NOT RUN EITHER");
  memset(speech_data, 0, sizeof(speech_data)); // BREAKS IT
  sprintf(speech_data, "%s", AUDIO_PREFIX); // BREAKS IT
  char holder[5] = {0};
  Serial.println("starting");
  uint32_t text_index = enc_index;
  uint32_t start = millis();
  time_since_sample = micros();
  while (sample_num < NUM_SAMPLES) { //read in NUM_SAMPLES worth of audio data
    value = analogRead(AUDIO_IN);  //make measurement
    raw_samples[sample_num % 3] = mulaw_encode(value - 1800); //remove 1.5ishV offset (from 12 bit reading)
    sample_num++;
    if (sample_num % 3 == 0) {
      base64_encode(holder, raw_samples, 3);
      strncat(speech_data + text_index, holder, 4);
      text_index += 4;
    }
    // wait till next time to read
    while (micros() - time_since_sample <= time_between_samples); //wait...
    time_since_sample = micros();
  }  
  Serial.println(millis() - start);
  sprintf(speech_data + strlen(speech_data), "%s", AUDIO_SUFFIX);
  Serial.println("out");
}

const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
/* 'Private' declarations */
inline void a3_to_a4(unsigned char * a4, unsigned char * a3);
inline void a4_to_a3(unsigned char * a3, unsigned char * a4);
inline unsigned char b64_lookup(char c);


int base64_encode(char *output, char *input, int inputLen) {
  int i = 0, j = 0;
  int encLen = 0;
  unsigned char a3[3];
  unsigned char a4[4];

  while(inputLen--) {
    a3[i++] = *(input++);
    if(i == 3) {
      a3_to_a4(a4, a3);

      for(i = 0; i < 4; i++) {
        output[encLen++] = pgm_read_byte(&b64_alphabet[a4[i]]);
      }

      i = 0;
    }
  }

  if(i) {
    for(j = i; j < 3; j++) {
      a3[j] = '\0';
    }

    a3_to_a4(a4, a3);

    for(j = 0; j < i + 1; j++) {
      output[encLen++] = pgm_read_byte(&b64_alphabet[a4[j]]);
    }

    while((i++ < 3)) {
      output[encLen++] = '=';
    }
  }
//  output[encLen] = '\0';
  return encLen;
}

int base64_decode(char * output, char * input, int inputLen) {
  int i = 0, j = 0;
  int decLen = 0;
  unsigned char a3[3];
  unsigned char a4[4];

  while (inputLen--) {
    if(*input == '=') {
      break;
    }

    a4[i++] = *(input++);
    if (i == 4) {
      for (i = 0; i <4; i++) {
        a4[i] = b64_lookup(a4[i]);
      }

      a4_to_a3(a3,a4);

      for (i = 0; i < 3; i++) {
        output[decLen++] = a3[i];
      }
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++) {
      a4[j] = '\0';
    }

    for (j = 0; j <4; j++) {
      a4[j] = b64_lookup(a4[j]);
    }

    a4_to_a3(a3,a4);

    for (j = 0; j < i - 1; j++) {
      output[decLen++] = a3[j];
    }
  }
  output[decLen] = '\0';
  return decLen;
}

int base64_enc_len(int plainLen) {
  int n = plainLen;
  return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

int base64_dec_len(char * input, int inputLen) {
  int i = 0;
  int numEq = 0;
  for(i = inputLen - 1; input[i] == '='; i--) {
    numEq++;
  }

  return ((6 * inputLen) / 8) - numEq;
}

inline void a3_to_a4(unsigned char * a4, unsigned char * a3) {
  a4[0] = (a3[0] & 0xfc) >> 2;
  a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
  a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
  a4[3] = (a3[2] & 0x3f);
}

inline void a4_to_a3(unsigned char * a3, unsigned char * a4) {
  a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
  a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
  a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
}

inline unsigned char b64_lookup(char c) {
  if(c >='A' && c <='Z') return c - 'A';
  if(c >='a' && c <='z') return c - 71;
  if(c >='0' && c <='9') return c + 4;
  if(c == '+') return 62;
  if(c == '/') return 63;
  return -1;
}


int8_t mulaw_encode(int16_t sample) {
  const uint16_t MULAW_MAX = 0x1FFF;
  const uint16_t MULAW_BIAS = 33;
  uint16_t mask = 0x1000;
  uint8_t sign = 0;
  uint8_t position = 12;
  uint8_t lsb = 0;
  if (sample < 0)
  {
    sample = -sample;
    sign = 0x80;
  }
  sample += MULAW_BIAS;
  if (sample > MULAW_MAX)
  {
    sample = MULAW_MAX;
  }
  for (; ((sample & mask) != mask && position >= 5); mask >>= 1, position--)
      ;
  lsb = (sample >> (position - 4)) & 0x0f;
  return (~(sign | ((position - 5) << 4) | lsb));
}

