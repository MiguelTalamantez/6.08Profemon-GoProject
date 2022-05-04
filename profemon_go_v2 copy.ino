#include <mpu6050_esp32.h> //Used for ESP32 stuff
#include <string.h> //used for some string handling and processing.
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include <stdio.h>
#include <cstring>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
MPU6050 imu; //imu object called, appropriately, imu
WiFiClientSecure client2;

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

const int BUTTON1 = 45; //Button1 at pin 45
const int BUTTON2 = 39; //Button2 at pin 39
const int BUTTON3 = 38; //Button3 at pin 38
const int BUTTON4 = 34; //Button4 at pin 34

// Wifi/HTTP variables
const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 5000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 5000; //size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 3000;
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP request
char json_body[JSON_BODY_SIZE];
uint32_t offset;

const char network[] = "MIT GUEST";
const char password[] = "";

// audio variables
const int DELAY = 1000;
const int SAMPLE_FREQ = 8000;                          // Hz, telephone sample rate
const int SAMPLE_DURATION = 5;                        // duration of fixed sampling (seconds)
const int NUM_SAMPLES = SAMPLE_FREQ * SAMPLE_DURATION;  // number of of samples
const int ENC_LEN = (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4;  // Encoded length of clip
bool conn;

const char PREFIX[] = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\",\"speechContexts\":[{\"phrases\":[\"riemann\", \"jacobian\"]}]}, \"audio\": {\"content\":\"";
//const char PREFIX[] = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\",\"speechContexts\":[{\"phrases\":[\"awkward smile\",\"suit and tie\",\"rhino\",\"evil laughter\",\"taylor series\",\"riemann sums\",\"jacobian determinant\",\"archery\",\"recursion\",\"test cases\",\"pointers\",\"induction\"]}]}, \"audio\": {\"content\":\"";
const char SUFFIX[] = "\"}}"; //suffix to POST request
const int AUDIO_IN = 1; //pin where microphone is connected
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //don't change this

uint32_t time_since_sample;      // used for microsecond timing

char speech_data[ENC_LEN + 200] = {0}; //global used for collecting speech data
const char* NETWORK = "MIT GUEST";     // your network SSID (name of wifi network)
const char* PASSWORD = ""; // your network password
const char*  SERVER = "speech.google.com";  // Server URL

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
uint8_t state;

char user_id[50];
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

float x, x_avg, x_old, x_older;
float y, y_avg, y_old, y_older;
float z, z_avg, z_old, z_older;

const int ZOOM = 9.81;

void setup() {
  // this is what happens when the battle is about to start
  // TFT setup
  tft.init();
  tft.setRotation(3);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0,1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  Serial.begin(115200);
  Serial.println("Let us get the battle set up for you");

  // VARIABLES THAT THE USER SHOULD'VE ALREADY CHOSEN
  strcpy(user_id,"diana");
  strcpy(profemon_id,"Max Goldman");

  // show the loading "battle is just beginning screen"
  tft.printf("You have chosen %s as your Prof-emon!\n",profemon_id);
  tft.printf("Loading...");

  // IMU setup
  Wire.begin();
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }

  // button setup
  pinMode(BUTTON1, INPUT_PULLUP); //set input pin as an input button1
  pinMode(BUTTON2, INPUT_PULLUP); //set input pin as an input button2
  pinMode(BUTTON3, INPUT_PULLUP); //set input pin as an input button3
  pinMode(BUTTON4, INPUT_PULLUP); //set input pin as an input button4

  b1 = 1;
  b2 = 1;
  b3 = 1;

  old_b1_input = 1;
  old_b2_input = 1;
  old_b3_input = 1;

  delay(4000);

  // Wifi setup
  WiFi.begin(NETWORK, PASSWORD); //attempt to connect to wifi
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
    Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                  WiFi.localIP()[1], WiFi.localIP()[0],
                  WiFi.macAddress().c_str() , WiFi.SSID().c_str());
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
  client2.setCACert(CA_CERT); //set cert for https
  
  // variable initialization
  num_turns = 0;
  state = 0;

  // send a POST request labeled "start" with the user id and the prof-emon, send another request for prof-emon's attack value, send ANOTHER request for prof-emon's two moves
  // (all of these are accomplished within the server_request function)
  make_server_request(start);

  // show the "it's the other player's turn right now" screen
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0,1);
  tft.printf("Waiting for your game to start...\n");

  time_since_turn_check = millis();
}

void loop() {
  b1 = digitalRead(BUTTON1);
  b2 = digitalRead(BUTTON2);
  b3 = digitalRead(BUTTON3);

  // two states: either we're waiting for the time when it'll be our move, or it's our move
  if(state==0) {
    // waiting for it to be our move. every [time period], we send a GET request labeled "turn" to ask whose turn it is
    if(millis()-time_since_turn_check > turn_check_period) {
      make_server_request(turn);
    }

    // if it's our turn, the function changed state to 1. we need to update the hp values, the screen (depends on whether the second move can be used yet), and the state value
    if(state==1) {
      // if the other player suddenly has hp zero, they forfeited, so you win! yay!
      if(other_hp==0) {
        Serial.println("they lost :D");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        tft.printf("Your opponent has FORFEITED!\n\n");
        tft.printf("%s hp: %u\n",profemon_id,player_hp);
        tft.printf("Your opponent hp: 0\n");
        tft.printf("Congratulations, you won!\n");
        Serial.printf("state is now 3\n");
        state = 3;
      }

      // if YOUR hp is zero, you lost! shit!
      else if(player_hp==0) {
        Serial.printf("oopsie doopsie");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        tft.printf("Your opponent used %s!\nIt was super effective!\n\n",last_move);
        tft.printf("%s hp: %u\n",profemon_id,player_hp);
        tft.printf("Your opponent hp: %u\n",other_hp);

        tft.printf("Unfortunately, you lost! Better luck next time.\n");
        Serial.printf("state is now 3\n");
        state = 3;
      }

      // otherwise, it was just a normal move -- time to start our move
      else {
        Serial.printf("time to start the move\n");
        num_turns++;
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        // if their attack did less damage than a third of our attack value, it's considered "not very effective" (by our standards)
        if(old_player_hp-player_hp>0 && old_player_hp-player_hp<(int)((double)max_attack/3.0)) {
          tft.printf("Your opponent used %s with damage %u!\nIt was not very effective.\n\n",last_move,(old_player_hp-player_hp));
        }
        // otherwise, solid attack!
        else if(old_player_hp-player_hp>0){
          tft.printf("Your opponent used %s with damage %u!\n\n",last_move,(old_player_hp-player_hp));
        }
        else {
          tft.printf("Your turn!\n\n");
        }
        
        tft.printf("%s hp: %u\n",profemon_id,player_hp);
        tft.printf("Your opponent hp: %u\n\n",other_hp);

        tft.printf("Choose a move!\n");
        tft.printf("Standard attack: %s\n",standard_move_name);
        if(num_turns>2) {
          tft.printf("Special attack: %s\n",special_move_name);
        }
        tft.printf("Press the third button to forfeit the match.");

        time_since_turn_started = millis();
      }
    }
  }
  else if(state==1) {
    // okay, it's now our turn, so we're going to wait for a button to be pressed

    // if the first button is pressed, they want to do the gesture
    if(old_b1_input!=b1 && b1==1) {
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0,1);
      tft.printf("You have selected the move %s. To perform this move, follow the instructions.\n",standard_move_name);
      start_time = millis();
      // calculate the amount of damage done and send it over
      damage_done = battle_direction(gesture);
      memset(last_move,0,50);
      strcpy(last_move,standard_move_name);
      
      make_server_request(attack);
      //make_server_request(turn);

      // if the other player's new hp is zero, you win! yay!
      if(other_hp-damage_done<1) {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        tft.printf("You used %s with %u damage! It was super effective!\n\n",last_move,damage_done);
        tft.printf("%s hp: %u\n",profemon_id,player_hp);
        tft.printf("Your opponent hp: 0\n\n");

        tft.printf("Your opponent has fainted!\nCongratulations, you won!\n");
        Serial.printf("state is now 3\n");
        state = 3;
      }

      // otherwise, standard message, and back to checking when our move begins
      else {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        if(damage_done>(int)(3.0*(double)max_attack/4.0)) {
          tft.printf("You used %s with %u damage! It was super effective!\n\n",last_move,damage_done);
        }
        else if(damage_done<(int)((double)max_attack/3.0)){
          tft.printf("You used %s with %u damage! It was not very effective.\n\n",last_move,damage_done);
        }
        else {
          tft.printf("You used %s with %u damage!\n\n",last_move,damage_done);
        }
        tft.printf("%s hp: %u\n",profemon_id,player_hp);
        tft.printf("Your opponent hp: %u\n",other_hp-damage_done);

        tft.printf("Waiting");
        delay(100);
        tft.printf(".");
        delay(100);
        tft.printf(".");
        delay(100);
        tft.printf(".\n");

        old_player_hp = player_hp;
        Serial.printf("state is now 0\n");
        state = 0;
        time_since_turn_check = millis();
      }
    }

    // if the second button is pressed, they want to do the voice recognition
    else if(num_turns>2 && old_b2_input!=b2 && b2==1) {
      Serial.println("you have selected the SPECIAL move");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0,1);
      tft.printf("You have selected the move %s.\n",special_move_name);
      tft.printf("Get ready to shout in...\n");
      tft.printf("3\n");
      delay(500);
      tft.printf("2\n");
      delay(500);
      tft.printf("1\n");
      delay(500);
      tft.printf("Speak!\n");

      // voice recognition goes here. they have three chances, or they do zero damage
      int i=0;
      while(i<3) {
        hear_command();

        Serial.printf("supposedly, |%s| and |%s| are super different /s\n",heard,special_move_name);
        //Serial.printf("and the difference between them is %d or maybe %d\n",strncmp(heard,special_move_name,strlen(special_move_name)),strcmp(heard,special_move_name));
        if(strncmp(heard,special_move_name,strlen(special_move_name))==0) {
          break;
        }
        else {
          tft.fillScreen(TFT_BLACK);
          tft.setCursor(0,0,1);
          tft.printf("Try again!\n");
          tft.printf("Get ready to shout %s in...\n",special_move_name);
          tft.printf("3\n");
          delay(500);
          tft.printf("2\n");
          delay(500);
          tft.printf("1\n");
          delay(500);
          tft.printf("Speak!\n");
        }

        i++;
      }

      if(i<3) {
        damage_done = max_attack;
      }
      else {
        damage_done = 0;
      }

      // calculate the amount of damage done and send it over. if the other player's new hp is zero, you win! yay!

      memset(last_move,0,50);
      strcpy(last_move,special_move_name);

      make_server_request(attack);
      //make_server_request(turn);

      if(other_hp-damage_done<1) {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        tft.printf("You used %s with %u damage! It was super effective!\n\n",last_move,damage_done);
        tft.printf("%s hp: %u\n",profemon_id,player_hp);
        tft.printf("Your opponent hp: 0\n\n");

        tft.printf("Your opponent has fainted!\nCongratulations, you won!\n");
        Serial.printf("state is now 3\n");
        state = 3;
      }

      // otherwise, standard message, and back to checking when our move begins
      else {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0,0,1);
        if(damage_done>(int)(3.0*(double)max_attack/4.0)) {
          tft.printf("You used %s with %u damage! It was super effective!\n\n",last_move,damage_done);
        }
        else if(damage_done<(int)((double)max_attack/3.0)){
          tft.printf("You used %s with %u damage! It was not very effective.\n\n",last_move,damage_done);
        }
        else {
          tft.printf("You used %s with %u damage!\n\n",last_move,damage_done);
        }
        tft.printf("%s hp: %u\n",profemon_id,player_hp);
        tft.printf("Your opponent hp: %u\n",other_hp-damage_done);

        tft.printf("Waiting for your opponent");
        delay(100);
        tft.printf(".");
        delay(100);
        tft.printf(".");
        delay(100);
        tft.printf(".");

        old_player_hp = player_hp;
        Serial.printf("state is now 0\n");
        state = 0;
        time_since_turn_check = millis();
      }
    }
    
    // if the third button is pressed, or they haven't done anything in five minutes, they forfeit the match
    else if((old_b3_input!=b3 && b3==1) || (millis()-time_since_turn_started>max_turn_length)) {
      Serial.println("you have FORFEITED the match :(");
      // how does that work, you may ask? send a POST request with the game_id and your player_id labeled "forfeit"
      make_server_request(forfeit);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0,1);
      tft.printf("You have FORFEITED!\n\n");
      tft.printf("Better luck next time :(\n");
      Serial.printf("state is now 3\n");
      state = 3;
    }
  }
  // i just realized i skipped state 2. that one's an accident -- this should technically be state 2 but since i'm guessing it's gonna be changed in integration anyway i don't care
  else if(state==3) {
    // you lost/you won state. revert to status quo now!
  }

  old_b1_input = b1;
  old_b2_input = b2;
  old_b3_input = b3;
}

void make_server_request(int type) {
  request[0] = '\0'; //set 0th byte to null
  offset = 0; //reset offset variable for sprintf-ing
  
  if(type == start) {
    // send a POST request labeled "start" with the player_id and prof_id
    request[0] = '\0';
    offset = 0;
    offset += sprintf(request + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=start&player_id=%s&prof_id=%s  HTTP/1.1\r\n",user_id,profemon_id);
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    // info to store: game_id, gesture, our own hp (as both player_hp and old_player_hp)
    char breaks[2];
    breaks[0] = ',';
    breaks[1] = '\0';
    char* ptr = strtok(response, breaks);

    //Serial.printf("1: %s",ptr);
    game_id = atoi(ptr);
    ptr = strtok (NULL, breaks);

    //Serial.printf("2: %s",ptr);
    gesture = atoi(ptr);
    ptr = strtok (NULL, breaks);

    //Serial.printf("3: %s",ptr);
    player_hp = atoi(ptr);
    old_player_hp = player_hp;

    Serial.printf("GAME HAS BEGUN! id is %u\n",game_id);

    // send a GET request to profedex.py asking for attack
    request[0] = '\0';
    offset = 0;
    offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team3/profedex.py?professor=%s&item=attack  HTTP/1.1\r\n",profemon_id);
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    // info to store: our own attack value
    max_attack = atoi(response);

    Serial.printf("max attack score is: %u\n",max_attack);

    // send ANOTHER request to figure out what your prof-emon's two moves are
    request[0] = '\0';
    offset = 0;
    offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team3/profedex.py?professor=%s&item=moves  HTTP/1.1\r\n",profemon_id);
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    char* ending1 = strrchr(response, '\n');
    *(ending1 + 0) = NULL;

    //Serial.printf("%s\n",response);

    // info to store: the two moves
    ptr = strtok(response, breaks);
    
    memset(standard_move_name,0,50);
    strcpy(standard_move_name,ptr);
    ptr = strtok (NULL, breaks);

    memset(special_move_name,0,50);
    strcpy(special_move_name,ptr);

    Serial.printf("The real moves are %s and %s\n", standard_move_name,special_move_name);
  }
  else if(type == turn) {
    // send a GET request labeled "turn" with the game_id and player_id
    offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=turn&player_id=%s&game_id=%u  HTTP/1.1\r\n",user_id,game_id);
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    // if the response is just "-1", the game hasn't started yet, and we should just keep waiting for the other player to enter the game
    if(strncmp(response,"-1",2)==0) {
      return;
    }

    else if(state==1) {
      Serial.printf("at the time, the turn is %s\n",response);
    }

    // info to store: the id of whose turn it is, our own hp, the other player's hp, and the last move
    char breaks[2];
    breaks[0] = ',';
    breaks[1] = '\0';
    char* ptr = strtok(response, breaks);
    
    if(strcmp(ptr,user_id)==0) {
      Serial.printf("state is now 1\n");
      state = 1;
    }

    ptr = strtok (NULL, breaks);
    player_hp = atoi(ptr);

    ptr = strtok (NULL, breaks);
    other_hp = atoi(ptr);

    ptr = strtok (NULL, breaks);
    if(strncmp(response,"n/a",3)==0) {
      memset(last_move,0,50);
    }
    else {
      memset(last_move,0,50);
      strcpy(last_move,ptr);
    }

    // Serial.printf("this will never be reached :/");
  }
  else if(type == attack) {
    // send a POST request labeled "attack" with the game_id, player_id, move, and damage
    request[0] = '\0';
    offset = 0;
    offset += sprintf(request + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=attack&player_id=%s&game_id=%u&move=%s&damage=%u  HTTP/1.1\r\n",user_id,game_id,last_move,damage_done);
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
    Serial.printf("sending an attack request with url %s\n",request);
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    Serial.printf("request has been sent\n");
  }
  else if(type == forfeit) {
    // send a POST request labeled "forfeit" with the game_id and player_id
    request[0] = '\0';
    offset = 0;
    offset += sprintf(request + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=forfeit&player_id=%s&game_id=%u  HTTP/1.1\r\n",user_id,game_id);
    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
  }
}

int battle_direction(int what_to_do){
  battlemode = what_to_do;
  gesture_total = 0;
  switch(battlemode){
    case x_shake:
    tft.println("SHAKE LEFT AND RIGHT");
      while((millis() - start_time) < gesture_duration){
        imu.readAccelData(imu.accelCount);
        x = abs(ZOOM * imu.accelCount[0] * imu.aRes);

        x_avg = (x + x_old + x_older) / 3.0;
        x_older = x_old;
        x_old = x;
        
        // char output[100];
        // sprintf(output,"X:%4.2f", x_avg); 
        // Serial.println(output);

        if(x_avg > 6){
          gesture_total += 1;
        }
      }
      break;
    case y_shake:
      tft.println("SHAKE UP AND DOWN");
      while((millis() - start_time) < gesture_duration){
        imu.readAccelData(imu.accelCount);
        y = abs(ZOOM * imu.accelCount[1] * imu.aRes);

        y_avg = (y + y_old + y_older) / 3.0;
        y_older = y_old;
        y_old = y;

        // char output[100];
        // sprintf(output,"Y:%4.2f", y_avg); 
        // Serial.println(output);

        if(y_avg > 12){
          gesture_total += 1;
        }
      }
      break;
    case z_shake:
      tft.println("SHAKE FORWARD AND BACK");
      while((millis() - start_time) < gesture_duration){
        imu.readAccelData(imu.accelCount);
        z = abs(ZOOM * imu.accelCount[2] * imu.aRes);

        z_avg = (z + z_old + z_older) / 3.0;
        z_older = z_old;
        z_old = z;

        //char output[100];
        //sprintf(output,"Z:%4.2f", z_avg); 
        //Serial.println(output);

        if(z_avg > 6){
          gesture_total += 1;
        }
      }
      break;
  }

  return (gesture_total*max_attack/2500);
}

void do_http_request(char* host, char* request, char* response, uint16_t response_size, uint16_t response_timeout, uint8_t serial){
  WiFiClient client; //instantiate a client object
  if (client.connect(host, 80)) { //try to connect to host on port 80
    if (serial) Serial.print(request);//Can do one-line if statements in C without curly braces
    client.print(request);
    memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
    uint32_t count = millis();
    while (client.connected()) { //while we remain connected read out data coming back
      client.readBytesUntil('\n',response,response_size);
      if (serial) Serial.println(response);
      if (strcmp(response,"\r")==0) { //found a blank line!
        break;
      }
      memset(response, 0, response_size);
      if (millis()-count>response_timeout) break;
    }
    memset(response, 0, response_size);  
    count = millis();
    while (client.available()) { //read out remaining text (body of response)
      char_append(response,client.read(),OUT_BUFFER_SIZE);
    }
    if (serial) Serial.println(response);
    client.stop();
    if (serial) Serial.println("-----------");  
  }else{
    if (serial) Serial.println("connection failed :/");
    if (serial) Serial.println("wait 0.5 sec...");
    client.stop();
  }
}

void hear_command() {
  //Serial.println("listening...");
  record_audio();
  //Serial.println("sending...");
  //Serial.print("\nStarting connection to server...");
  delay(300);
  conn = false;
  for (int i = 0; i < 10; i++) {
    int val = (int)client2.connect(SERVER, 443, 4000);
    Serial.print(i); Serial.print(": "); Serial.println(val);
    if (val != 0) {
      conn = true;
      break;
    }
    Serial.print(".");
    delay(300);
  }
  if (!conn) {
    Serial.println("Connection failed!");
    return;
  }
  else {
    Serial.println("Connected to server!");
    Serial.println(client2.connected());
    int len = strlen(speech_data);
    // Make a HTTP request:
    client2.print("POST /v1/speech:recognize?key="); client2.print(API_KEY); client2.print(" HTTP/1.1\r\n");
    client2.print("Host: speech.googleapis.com\r\n");
    client2.print("Content-Type: application/json\r\n");
    client2.print("cache-control: no-cache\r\n");
    client2.print("Content-Length: "); client2.print(len);
    client2.print("\r\n\r\n");
    int ind = 0;
    int jump_size = 1000;
    char temp_holder[jump_size + 10] = {0};
    Serial.println("sending data");
    while (ind < len) {
      delay(80);//experiment with this number!
      //if (ind + jump_size < len) client.print(speech_data.substring(ind, ind + jump_size));
      strncat(temp_holder, speech_data + ind, jump_size);
      client2.print(temp_holder);
      ind += jump_size;
      memset(temp_holder, 0, sizeof(temp_holder));
    }
    client2.print("\r\n");
    //Serial.print("\r\n\r\n");
    //Serial.println("Through send...");
    unsigned long count = millis();
    while (client2.connected()) {
      //Serial.println("IN!");
      String line = client2.readStringUntil('\n');
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
    while (!client2.available()) {
      delay(100);
      //Serial.print(".");
      if (millis() - count > RESPONSE_TIMEOUT) break;
    }
    Serial.println();
    //Serial.println("-----------");
    memset(response, 0, sizeof(response));
    while (client2.available()) {
      char_append(response, client2.read(), OUT_BUFFER_SIZE);
    }
    //Serial.println(response); //comment this out if needed for debugging
    char* trans_id = strstr(response, "transcript");
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
    client2.stop();
    Serial.println("done");
  }
}

void record_audio() {
  int sample_num = 0;    // counter for samples
  int enc_index = strlen(PREFIX) - 1;  // index counter for encoded samples
  float time_between_samples = 1000000 / SAMPLE_FREQ;
  int value = 0;
  char raw_samples[3];   // 8-bit raw sample data array
  memset(speech_data, 0, sizeof(speech_data));
  sprintf(speech_data, "%s", PREFIX);
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
  sprintf(speech_data + strlen(speech_data), "%s", SUFFIX);
  Serial.println("out");
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