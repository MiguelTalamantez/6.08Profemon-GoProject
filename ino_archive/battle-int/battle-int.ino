#include <mpu6050_esp32.h>
#include <math.h>
#include <string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <WiFi.h> //Connect to WiFi Network
#include <stdlib.h>
#include <time.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <stdio.h>
#include <cstring>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
MPU6050 imu; //imu object called, appropriately, imu
WiFiClientSecure client; //global WiFiClient Secure object
WiFiClient client2; //global WiFiClient Secure object

const int BUTTON1 = 45; //Button1 at pin 45
const int BUTTON2 = 39; //Button2 at pin 39
const int BUTTON3 = 38; //Button3 at pin 38
const int BUTTON4 = 34; //Button4 at pin 34

// HTTP Request Variables
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int BUTTON_TIMEOUT = 1000; //button timeout in milliseconds
const uint16_t IN_BUFFER_SIZE = 5000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 5000; //size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 3000;
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
char json_body[JSON_BODY_SIZE];
StaticJsonDocument<500> doc;
uint8_t channel = 1; //network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; //6 byte MAC address of AP you're targeting.
int offset = 0;

const char LOC_PREFIX[] = "{\"wifiAccessPoints\": ["; //beginning of json body
const char LOC_SUFFIX[] = "]}"; //suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; //don't change this and don't share this
const int MAX_APS = 5;

const char AUDIO_PREFIX[] = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\",\"speechContexts\":[{\"phrases\":[\"riemann\", \"jacobian\"]}]}, \"audio\": {\"content\":\"";
const char AUDIO_SUFFIX[] = "\"}}"; //suffix to POST request_buffer
const int AUDIO_IN = 1; //pin where microphone is connected

char network[] = "MIT";
char password[] = "";

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
const char user[50] = "Pikachu";
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


/*********************************************
Battle Stuff
*********************************************/
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

char user[50];
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


void setup() {
  Serial.begin(115200); //for debugging if needed.
  delay(50); //pause to make sure comms get set up
  Wire.begin();
  delay(50); //pause to make sure comms get set up
  if (imu.setupIMU(1)) {
    Serial.println("IMU Connected!");
  } else {
    Serial.println("IMU Not Connected :/");
    Serial.println("Restarting");
    ESP.restart(); // restart the ESP (proper way)
  }

  //if using regular connection use line below:
  WiFi.begin(network, password);
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
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

  tft.init(); //initialize the screen
  tft.setRotation(1); //set rotation for our layout
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON4, INPUT_PULLUP);
  loc_timer = millis();
}

/*********************************************
1. Get player loc
2. Check locs of profemon to see if there's one nearby
3. Display that there's one nearby if there is
4. Catch sequence
5. Add profemon to profedex
*********************************************/

void loop() {
    tft.setCursor(0, 0);
    update_location_and_profs();
    
    if (change_display == 1) {
        tft.fillScreen(TFT_BLACK);
        change_display = 0;
    }
    if (game_state == Map) {
        update_idle_mode(digitalRead(BUTTON1), digitalRead(BUTTON2));
        if (idle_state == Idle) {
            if (strlen(profemon_name) != 0 && strlen(display_name) != 0) {
                tft.printf("There is a %s nearby...\nWould you like to catch them?\n\nYES: Button 1\nNO: Button 2\n", display_name);
            } else {
                tft.printf("IDLE");
            }
        }
    } else if (game_state == Catch) {
        update_catch_mode(digitalRead(BUTTON1), digitalRead(BUTTON2), motion);
        is_moving();
        catch_display();
    }
}

// State machine that changes when profemon is nearby
// Transition from map state to catch state
void update_idle_mode(int in1, int in2) {
    switch (idle_state) {
        case Idle:
            if (in1 == 0) {
                idle_state = Yes;
            } else if (in2 == 0) {
                idle_state = No;
                ask_timer = millis();
            }
            break;
        case Yes:
            if (in1 == 1) {
                game_state = Catch;
                idle_state = Idle;
                change_display = 1;
                new_profemon = 1;
            }
            break;
        case No:
            change_display = 1;
            if (in2 == 1 && millis()-ask_timer > 5000) {
                idle_state = Idle;
            }
            break;
    }
}

// State machine for catch mode
void update_catch_mode(int in1, int in2, int motion) {
    switch (catch_state) {
        case Catch_Idle:
            if (in1 == 0) {
                catch_state = Waiting;
                change_display = 1;
            }
            if (in2 == 0) {
                catch_state = Quitting;
            }          
            break;
        case Quitting:
            if (in2 == 1) {
                catch_state = Catch_Idle;
                game_state = Map;
                idle_state = Idle;
                change_display = 1;
            }
            break;
        case Waiting:
            if (in1 == 1) {
                catch_state = Catch_Idle;
                change_display = 1;
            } else if (motion > 0) {
                catch_state = Motion;
                state_timer = millis();
                swing_counter = 0;
            }
            break;
        case Motion:
            if (in1 == 1) {
                catch_state = Catch_Idle;
                change_display = 1;
            } else if (millis() - state_timer > 5000) {
                catch_state = Waiting;
            } else if (motion == 2) {
                catch_state = Throw_End;
            }
            break;
        case Throw_End:
            if (calculate_catch_success() == 1) {
                Serial.println("throw successful");
                // Make a HTTP request:
                strcpy(request_buffer, "");
                strcpy(response_buffer, "");
                request_buffer[0] = '\0'; //set 0th byte to null
                response_buffer[0] = '\0'; //set 0th byte to null
                char data[500];
                sprintf(data, "lat_captured=%f&lon_captured=%f&prof_id=%s&user_id=%s&limit=5", latitude, longitude, profemon_name, user); 
                Serial.println(data);
                offset = 0; //reset offset variable for sprintf-ing
                offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/profedex_editor.py?%s  HTTP/1.1\r\n", data);
                offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
                do_http_request("608dev-2.net", request_buffer, profedex_data, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
                Serial.println(request_buffer);
                Serial.println("-----------");
                Serial.println(profedex_data);
                begin_profedex = strchr(profedex_data, '\n') + 1;
                Serial.println("-----------");
                delay(2000);
                change_display = 1;
                catch_state = Catch_Success;
            } else {
                change_display = 1;
                catch_state = Catch_Fail;
            }
            state_timer = millis();
            break;
        case Catch_Success:
            if (millis() - state_timer > 10000) {
                change_display = 1;
                catch_state = Catch_Idle;
                game_state = Map;
                idle_state = Idle;
            }
            break;
        case Catch_Fail:
            if (millis() - state_timer > 2000) {
                change_display = 1;
                catch_state = Catch_Idle;
            }
            break;
    }
}

// Function to display cool things for catch mode
void catch_display() {
    switch (catch_state) {
        case Catch_Idle:
            if (new_profemon == 1) {
                for (int i = 0; i < 4; i++) {
                    sprintf(request_buffer, "GET /sandbox/sc/team3/week2/profedex.py?professor=%s&item=%s HTTP/1.1\r\nHost: 608dev-2.net\r\n\r\n", profemon_name, profemon_data_labels[i]);
                    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
                    sprintf(profemon_data[i], "%s", response_buffer);
                    delay(2);
                }
                new_profemon = 0;
            }
            tft.printf("A wild %s has appeared!\n\n", display_name);
            tft.printf("This is a Course %sProf-emon!\nHP: %sAttack: %sMoves: %s\n\n", profemon_data[0], profemon_data[1], profemon_data[2], profemon_data[3]);
            tft.printf("Catch: HOLD Button 1\nQuit: Button 2");
            break;
        case Waiting:
            tft.printf("Swing your arm in a circle by your side to charge up your throw!\n\n");
            tft.printf("Throw underhanded and let go of the button to release the prof-eball!");
            break;
        case Catch_Fail:
            tft.printf("Aw, you missed!");
            break;
        case Catch_Success:
            if (strlen(profemon_name) != 0 && strlen(display_name) != 0) {
                tft.printf("Gotcha!\n%s was caught!\n\n", display_name);
                tft.printf("%s", begin_profedex);
            }
            if (change_display == 0) {
                profemon_name[0] = '\0';
                display_name[0] = '\0';
            }
            break;
    }
}

// Function to detect motion FOR catch mode (not battle)
// 0 is no motion, 1 is swinging, 2 is throw, 3 is invalid motion
int detect_catch_motion(float x_in, float y_in, float z_in) {
    if (abs(x_in) < 5 && abs(y_in) < 5) {
        return 0;
    }

    if (y_in > 10) {
        cycle_timer = millis();
        swing_in_progress = true;
        // Serial.println("High");
    } else if (x_in < -18 and y_in < -10) {
        if (millis()-cycle_timer < 300 && swing_in_progress) {
            // Serial.printf("Swing Count: %d", swing);
            swing_in_progress = false;
            return 1;
        } else {
            return 3;
        }
    }

    if (x_in > 7) {
        // Serial.println("Throw");
        if (peak_timer == -1) {
            peak_timer = millis();
        } else if (millis()-peak_timer > 1000) {
            return 2;
        }
    } else if (x_in < 3) {
        peak_timer = -1;
    }

    return 3;
}

// update acceleration values
void is_moving() {
    imu.readAccelData(imu.accelCount);
    x = ZOOM * imu.accelCount[0] * imu.aRes;
    y = ZOOM * imu.accelCount[1] * imu.aRes;
    z = ZOOM * imu.accelCount[2] * imu.aRes;

    x_avg = (x + x_old + x_older) / 3.0;
    y_avg = (y + y_old + y_older) / 3.0;
    z_avg = (z + z_old + z_older) / 3.0;

    x_older = x_old;
    x_old = x;
    y_older = y_old;
    y_old = y;
    z_older = z_old;
    z_old = z;
 
    motion = detect_catch_motion(x_avg, y_avg, z_avg);

    if (motion == 1) {
        swing_counter++;
        if (swing_counter != 1) {
            swing_times[swing_counter] = millis()-swing_timer;
        }
        swing_timer = millis();
    }
    // tft.printf("# Swings: %d    \n", swing_counter);

    //Serial printing:
    char output[100];
    // sprintf(output,"X:%4.2f,Y:%4.2f,Z:%4.2f,Motion:%d",x_avg,y_avg,z_avg,motion); //render numbers with %4.2 float formatting
    // Serial.println(output); //print to serial for plotting
    // sprintf(output, "\n%4.2f  \n%4.2f  \n%4.2f  \nMotion: %d  ", x, y, z, motion); //render numbers with %4.2 float formatting
    // tft.println(output);
}

// calculate catch success
int calculate_catch_success() {
    float avg_time = 0;
    for (int i = 2; i <= swing_counter; i++) {
        // Serial.printf("Time between swing %d and %d: %d millisec\n", i-1, i, swing_times[i]);
        avg_time += swing_times[i];
    }
    avg_time /= (swing_counter-1);
    // Serial.printf("Avg swing time: %f\n", avg_time);
    float avg_error = 0;
    for (int i = 2; i <= swing_counter; i++) {
    //   Serial.println(abs(avg_time-swing_times[i])/(avg_time));
        avg_error += abs(avg_time-swing_times[i])/(avg_time);
    }
    avg_error /= (swing_counter-1);
    // Serial.printf("Avg err: %f\n", avg_error);
    float err_mult = avg_error/(swing_counter-1);
    // Serial.printf("Err w/ mult: %f\n", err_mult);

    if (err_mult > 0.5) { err_mult = 0.5; }

    float probability = 0.1 + (0.5 - err_mult);
    int success_thresh = int(probability * 16000);

    srand(time(NULL));
    int r = rand() % 16000; // random number from 0 to 16000

    if (r <= success_thresh) {
        return 1;
    }
    return 0;
}

// Should be called every loop (or at least once every 5-ish seconds)
void update_location_and_profs() {
    if (millis() - loc_timer > 30000) {
        strcpy(json_body, "");
        offset = sprintf(json_body, "%s", LOC_PREFIX);
        int n = WiFi.scanNetworks(); //run a new scan. could also modify to use original scan from setup so quicker (though older info)
        Serial.println("scan done");
        if (n == 0) {
        Serial.println("no networks found");
        } else {
        int max_aps = max(min(MAX_APS, n), 1);
        for (int i = 0; i < max_aps; ++i) { //for each valid access point
            uint8_t* mac = WiFi.BSSID(i); //get the MAC Address
            offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE-offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); //generate the query
            if(i!=max_aps-1){
            offset +=sprintf(json_body+offset,",");//add comma between entries except trailing.
            }
        }
        sprintf(json_body + offset, "%s", LOC_SUFFIX);
        Serial.println(json_body);
        int len = strlen(json_body);
        // Make a HTTP request:
        Serial.println("SENDING REQUEST");
        strcpy(request_buffer, "");
        strcpy(response_buffer, "");
        request_buffer[0] = '\0'; //set 0th byte to null
        response_buffer[0] = '\0'; //set 0th byte to null
        offset = 0; //reset offset variable for sprintf-ing
        offset += sprintf(request_buffer + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
        offset += sprintf(request_buffer + offset, "Host: googleapis.com\r\n");
        offset += sprintf(request_buffer + offset, "Content-Type: application/json\r\n");
        offset += sprintf(request_buffer + offset, "cache-control: no-cache\r\n");
        offset += sprintf(request_buffer + offset, "Content-Length: %d\r\n\r\n", len);
        offset += sprintf(request_buffer + offset, "%s", json_body);
        do_https_request("googleapis.com", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
        Serial.println("-----------");
        Serial.println(response_buffer);
        Serial.println("-----------");

        char* begin = strchr(response_buffer, '{');
        char* end = strrchr(response_buffer, '}');
        char djd[500];
        strncpy(djd, begin, end-begin+1);
        DeserializationError error = deserializeJson(doc, djd);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        latitude = doc["location"]["lat"];
        longitude = doc["location"]["lng"];

        strcpy(request_buffer, "");
        strcpy(response_buffer, "");
        request_buffer[0] = '\0'; //set 0th byte to null
        response_buffer[0] = '\0'; //set 0th byte to null
        offset = 0; //reset offset variable for sprintf-ing

        sprintf(request_buffer, "GET /sandbox/sc/team3/nearby_profemon.py?lat=%f&lon=%f HTTP/1.1\r\nHost: 608dev-2.net\r\n\r\n", latitude, longitude);
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        Serial.printf("%s, %d", response_buffer, strlen(response_buffer));
        char* comma = strchr(response_buffer, ',');
        if (comma != NULL) {
            strncpy(profemon_name, response_buffer, comma-response_buffer);
            strcpy(display_name, comma+1);
        } else if (game_state == Map) {
            strcpy(profemon_name, "");
            strcpy(display_name, "");
        }

        Serial.println(profemon_name);
        Serial.println(display_name);
        change_display = 1;
        }
        loc_timer = millis();
    }
}


void make_server_request(int type) {
  request_buffer[0] = '\0'; //set 0th byte to null
  offset = 0; //reset offset variable for sprintf-ing
  
  if(type == start) {
    // send a POST request_buffer labeled "start" with the player_id and prof_id
    request_buffer[0] = '\0';
    offset = 0;
    offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=start&player_id=%s&prof_id=%s  HTTP/1.1\r\n",user,profemon_id);
    offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    // info to store: game_id, gesture, our own hp (as both player_hp and old_player_hp)
    char breaks[2];
    breaks[0] = ',';
    breaks[1] = '\0';
    char* ptr = strtok(response_buffer, breaks);

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

    // send a GET request_buffer to profedex.py asking for attack
    request_buffer[0] = '\0';
    offset = 0;
    offset += sprintf(request_buffer + offset, "GET http://608dev-2.net/sandbox/sc/team3/profedex.py?professor=%s&item=attack  HTTP/1.1\r\n",profemon_id);
    offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    // info to store: our own attack value
    max_attack = atoi(response_buffer);

    Serial.printf("max attack score is: %u\n",max_attack);

    // send ANOTHER request_buffer to figure out what your prof-emon's two moves are
    request_buffer[0] = '\0';
    offset = 0;
    offset += sprintf(request_buffer + offset, "GET http://608dev-2.net/sandbox/sc/team3/profedex.py?professor=%s&item=moves  HTTP/1.1\r\n",profemon_id);
    offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    char* ending1 = strrchr(response_buffer, '\n');
    *(ending1 + 0) = NULL;

    //Serial.printf("%s\n",response);

    // info to store: the two moves
    ptr = strtok(response_buffer, breaks);
    
    memset(standard_move_name,0,50);
    strcpy(standard_move_name,ptr);
    ptr = strtok (NULL, breaks);

    memset(special_move_name,0,50);
    strcpy(special_move_name,ptr);

    Serial.printf("The real moves are %s and %s\n", standard_move_name,special_move_name);
  }
  else if(type == turn) {
    // send a GET request_buffer labeled "turn" with the game_id and player_id
    offset += sprintf(request_buffer + offset, "GET http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=turn&player_id=%s&game_id=%u  HTTP/1.1\r\n",user,game_id);
    offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);

    // if the response is just "-1", the game hasn't started yet, and we should just keep waiting for the other player to enter the game
    if(strncmp(response_buffer,"-1",2)==0) {
      return;
    }

    else if(battle_state==1) {
      Serial.printf("at the time, the turn is %s\n",response_buffer);
    }

    // info to store: the id of whose turn it is, our own hp, the other player's hp, and the last move
    char breaks[2];
    breaks[0] = ',';
    breaks[1] = '\0';
    char* ptr = strtok(response_buffer, breaks);
    
    if(strcmp(ptr,user)==0) {
      Serial.printf("state is now 1\n");
      battle_state = 1;
    }

    ptr = strtok (NULL, breaks);
    player_hp = atoi(ptr);

    ptr = strtok (NULL, breaks);
    other_hp = atoi(ptr);

    ptr = strtok (NULL, breaks);
    if(strncmp(response_buffer,"n/a",3)==0) {
      memset(last_move,0,50);
    }
    else {
      memset(last_move,0,50);
      strcpy(last_move,ptr);
    }

    // Serial.printf("this will never be reached :/");
  }
  else if(type == attack) {
    // send a POST request_buffer labeled "attack" with the game_id, player_id, move, and damage
    request_buffer[0] = '\0';
    offset = 0;
    offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=attack&player_id=%s&game_id=%u&move=%s&damage=%u  HTTP/1.1\r\n",user,game_id,last_move,damage_done);
    offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
    Serial.printf("sending an attack request_buffer with url %s\n",request_buffer);
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
    Serial.printf("request_buffer has been sent\n");
  }
  else if(type == forfeit) {
    // send a POST request_buffer labeled "forfeit" with the game_id and player_id
    request_buffer[0] = '\0';
    offset = 0;
    offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=forfeit&player_id=%s&game_id=%u  HTTP/1.1\r\n",user,game_id);
    offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
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

void hear_command() {
  //Serial.println("listening...");
  record_audio();
  //Serial.println("sending...");
  //Serial.print("\nStarting connection to server...");
  delay(300);
  conn = false;
  for (int i = 0; i < 10; i++) {
    int val = (int)client2.connect("speech.google.com", 443, 4000);
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
    // Make a HTTP request_buffer:
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
    memset(response_buffer, 0, sizeof(response_buffer));
    while (client2.available()) {
      char_append(response_buffer, client2.read(), OUT_BUFFER_SIZE);
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
    client2.stop();
    Serial.println("done");
  }
}

void record_audio() {
  int sample_num = 0;    // counter for samples
  int enc_index = strlen(AUDIO_PREFIX) - 1;  // index counter for encoded samples
  float time_between_samples = 1000000 / SAMPLE_FREQ;
  int value = 0;
  char raw_samples[3];   // 8-bit raw sample data array
  memset(speech_data, 0, sizeof(speech_data));
  sprintf(speech_data, "%s", AUDIO_PREFIX);
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