#include <mpu6050_esp32.h> //Used for ESP32 stuff
#include <math.h>
#include <string.h>   //used for some string handling and processing.
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>      //Used in support of TFT Display
#include <WiFi.h>     //Connect to WiFi Network
#include <stdlib.h>
#include <time.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <stdio.h>
#include <cstring>

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
MPU6050 imu;               // imu object called, appropriately, imu

// Button Setup
const int BUTTON1 = 45; // Button1 at pin 45
const int BUTTON2 = 39; // Button2 at pin 39
const int BUTTON3 = 38; // Button3 at pin 38
const int BUTTON4 = 34; // Button4 at pin 34
uint8_t button1;        // for reading buttons
uint8_t button2;
uint8_t button3;
uint8_t button4;
uint8_t old_button1; // for storing old vals
uint8_t old_button2;
uint8_t old_button3;
uint8_t old_button4;

// State machine variables
int overallstate = 0;
//   int mapstate = 0;
//   int catchstate = 0;
bool battle_or_catch = false; // battle = true, catch = false

// Miscl variables
bool displaytext = true;
bool askifcatch = false;
bool catchbuttons = false;
float userlocationx = -71.101543;     // Simmons     -71.092526;//Bridge     -71.097723;//Dlab
float userlocationy = 42.357113;      // Simmons      42.357008;//Bridge      42.361863;//Dlab
float testproflocationx = -71.102543; // 092482
float testproflocationy = 42.357113;  // 360263
float deltax = 0;
float deltay = 0;
float rdistance = 0;

// Location to TFT variables
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

// Image Arrays
static const uint8_t PROGMEM my_map[] = {
    0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xf1, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0x1f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1,
    0x1f, 0xff, 0xff, 0xff, 0xff, 0xe3, 0xe1, 0xe1, 0xe1, 0xe0, 0xf0, 0xf0, 0x78, 0x78, 0x7f, 0xff,
    0xff, 0xff, 0xff, 0xf1, 0x1f, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff, 0x01, 0xc1, 0xc1, 0xc1,
    0xc0, 0xe0, 0x70, 0x70, 0x70, 0x38, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff,
    0x01, 0xc1, 0xc1, 0xc1, 0xc0, 0xe0, 0x70, 0x70, 0x70, 0x38, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1,
    0x1f, 0xff, 0xff, 0xff, 0x01, 0xc1, 0xc1, 0xc1, 0xc0, 0xc0, 0x70, 0x70, 0x70, 0x38, 0x38, 0x0f,
    0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff, 0x01, 0xc1, 0xc1, 0xc1, 0xc0, 0xe0, 0x60, 0x70,
    0x70, 0x38, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff, 0x01, 0xc1, 0xc1, 0xc1,
    0xc0, 0xe0, 0x60, 0x70, 0x70, 0x38, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff,
    0x01, 0xc1, 0xc1, 0xc1, 0xc0, 0xe0, 0x60, 0x60, 0x70, 0x38, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1,
    0x1f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xe0, 0x60, 0x60, 0x70, 0x38, 0x38, 0x0f,
    0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xe0, 0x60, 0x60,
    0x70, 0x30, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1,
    0xc0, 0xe0, 0x60, 0x60, 0x70, 0x30, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x3f, 0xff, 0xff, 0xff,
    0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x38, 0x0f, 0xff, 0xff, 0xff, 0xe1,
    0x1f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x0f,
    0xff, 0xff, 0xff, 0xe1, 0x1f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60,
    0x70, 0x30, 0x30, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x3f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1,
    0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x0f, 0xff, 0xff, 0xff, 0xe1, 0x3f, 0xff, 0xff, 0xff,
    0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x0f, 0xff, 0xff, 0xff, 0xe3,
    0x3f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x0f,
    0xff, 0xff, 0xff, 0xc3, 0x1f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60,
    0x70, 0x30, 0x30, 0x0f, 0xff, 0xff, 0xff, 0xc3, 0x1f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1,
    0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x0f, 0xff, 0xff, 0xff, 0xc1, 0x0f, 0xff, 0xff, 0xff,
    0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x0f, 0xff, 0xff, 0xff, 0x81,
    0x0f, 0xff, 0xff, 0xff, 0x01, 0x81, 0xc1, 0xc1, 0xc0, 0xc0, 0x60, 0x60, 0x70, 0x30, 0x30, 0x0f,
    0xff, 0xff, 0xff, 0x20, 0x4f, 0xff, 0xff, 0xff, 0x01, 0x81, 0x81, 0xc1, 0xc0, 0xc0, 0x60, 0x60,
    0x70, 0x30, 0x30, 0x0f, 0xff, 0xff, 0xfe, 0x00, 0x27, 0xff, 0xff, 0xff, 0x01, 0x81, 0x81, 0xc1,
    0xc0, 0xc0, 0xe0, 0x60, 0x70, 0x30, 0x30, 0x0e, 0xbf, 0xff, 0xf8, 0xc4, 0x27, 0xff, 0xff, 0xff,
    0x01, 0x81, 0x81, 0xc1, 0xc0, 0xc0, 0xe0, 0x60, 0x70, 0x30, 0x30, 0x06, 0xfb, 0xff, 0xf3, 0xc2,
    0x07, 0xff, 0xff, 0xff, 0x01, 0x81, 0x81, 0x81, 0xc0, 0xc0, 0xe0, 0x60, 0x60, 0x30, 0x30, 0x07,
    0xfd, 0xff, 0xc7, 0xc2, 0xc3, 0xff, 0xff, 0xff, 0x00, 0x81, 0x80, 0x80, 0xc0, 0xc0, 0x60, 0x60,
    0x20, 0x30, 0x30, 0x0f, 0xde, 0x00, 0x1f, 0x80, 0xc3, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x7f, 0x3f, 0x0c, 0xf1, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf7, 0xcc, 0x7f, 0x4c,
    0xf8, 0x7f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
    0x9f, 0xe3, 0xfe, 0x4f, 0x79, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0c, 0xff, 0x07, 0xf8, 0xcf, 0x7d, 0xe7, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x67, 0xc1, 0xc7, 0x7d, 0xf9, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf9, 0xff, 0x03, 0xc7,
    0x7c, 0xfc, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0xf0, 0x8c, 0x07, 0x87, 0x3e, 0xfe, 0x67, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x02, 0x40, 0x80, 0x13, 0x07, 0x1e, 0x7f, 0x9b, 0xff, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xc0, 0x00, 0x6c, 0x43, 0x4e, 0x0f, 0xe3, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x00, 0x00, 0x41,
    0x16, 0x03, 0xf9, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x84, 0x1a, 0x80, 0x09, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x81, 0x08, 0x0f, 0xc0, 0x3f, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x25, 0xf7, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x83, 0x00,
    0x83, 0x30, 0xfb, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x07, 0xfe, 0x28, 0x81, 0x10, 0xb9, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0xfc, 0x28, 0x80, 0x20, 0x39, 0xb0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0xa8, 0x0c, 0x08, 0x1a, 0xe4,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x30, 0x88,
    0x00, 0x08, 0x20, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0c, 0x81, 0x00, 0x04, 0x08, 0x88, 0x48, 0x60, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x0f, 0xff, 0xec, 0x00, 0x05, 0x42, 0x00, 0x22, 0x01, 0x40, 0x0f, 0x06, 0x5f, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9c, 0x00, 0x05, 0x20, 0x00, 0x22, 0x00, 0x00, 0x08,
    0xce, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x38, 0x00, 0x49, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x06, 0x0c, 0x87, 0xff, 0xff, 0xff, 0x80, 0x3f, 0xff, 0xff, 0xff, 0xf9, 0x32,
    0x60, 0xe0, 0x06, 0x02, 0x00, 0x00, 0x02, 0x0c, 0x84, 0xdf, 0xff, 0xff, 0xff, 0x80, 0x3f, 0xff,
    0xff, 0xff, 0xff, 0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x84, 0x7f, 0xff, 0xff,
    0xff, 0x80, 0x1f, 0xff, 0xff, 0xff, 0xfe, 0xcc, 0x1f, 0x60, 0x00, 0x00, 0x10, 0x01, 0x00, 0x40,
    0x12, 0x7f, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xf1, 0x10, 0x3b, 0xc0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0x10,
    0x7f, 0xc0, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x7f, 0xff, 0xff, 0xff, 0xff,
    0xc1, 0xff, 0xfc, 0x03, 0xdf, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xff, 0xfc, 0x00,
    0x7f, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xfc, 0x66, 0xff, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x15,
    0x3d, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc4, 0xbf, 0x50, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x1a, 0x0f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xc1,
    0x1f, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xc2, 0xfe, 0x80, 0x04, 0x00, 0x00, 0x00, 0x00, 0x1e, 0xff, 0xff, 0xfc, 0x7f,
    0xff, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff, 0xdf, 0xfe, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
    0xff, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xfe, 0x00, 0x3f, 0xff, 0xff, 0xff,
    0xff, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x0f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff,
    0xf8, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x90, 0x01, 0x6f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0x00, 0x00,
    0x00, 0x74, 0xa7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xfc, 0x58, 0x00, 0x01, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x0c, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x5b, 0x00, 0x03, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xf8, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfa, 0x7f, 0x00, 0x83, 0xf8, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1f, 0xf8,
    0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xfe, 0x1f, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x7f, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xc0, 0xff, 0xf0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x3e, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0x1e, 0x8f, 0x8f, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x40, 0xce, 0x80, 0x67,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,
    0x8c, 0x4e, 0x86, 0x27, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfd, 0xbf, 0x26, 0xdf, 0x93, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x3f, 0x26, 0x9f, 0x93, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x3f, 0xb6, 0x83, 0xcb,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb,
    0x7f, 0xb6, 0xfb, 0xcb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfb, 0x7f, 0xb6, 0x03, 0xcb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0x7f, 0xb6, 0x03, 0xcb, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9, 0x7f, 0xb7, 0xff, 0xcb,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf9,
    0x3f, 0xb6, 0x1f, 0xcb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xf9, 0x3f, 0x26, 0x1f, 0x93, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0x9e, 0x6e, 0x4f, 0x93, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xc0, 0x4e, 0x60, 0x27,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0x61, 0x9f, 0x30, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x1e, 0x1f, 0x8f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xc3,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd7, 0x9f, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xf9, 0xff,
    0xff, 0xff, 0xff, 0xcb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb7, 0x87, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xb7,
    0xb7, 0x81, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xe9, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xb7, 0xae, 0x3c, 0x78, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xfa, 0xff, 0xed,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xa7, 0x2e, 0xc3, 0x78, 0xf1, 0xe3, 0xe0, 0xfc, 0x3c,
    0x71, 0xfa, 0x7f, 0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2f, 0x2d, 0x99, 0xb2, 0xed,
    0xc3, 0xce, 0x7d, 0x39, 0x8c, 0xfa, 0x7f, 0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6f,
    0x6d, 0xbd, 0xb6, 0xed, 0xcb, 0xbb, 0xbd, 0xbb, 0x26, 0xfb, 0x7e, 0x0c, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x6f, 0x6d, 0xbc, 0xb6, 0xed, 0xcb, 0x60, 0xdd, 0xb2, 0xf2, 0x7b, 0x78, 0xfc,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6f, 0x69, 0x7e, 0xb6, 0xed, 0xdb, 0x0e, 0x5d, 0xb6,
    0xfb, 0x7b, 0x73, 0xc6, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6e, 0xc9, 0x7c, 0xb6, 0xed,
    0xdb, 0xff, 0x5d, 0xb6, 0xfb, 0x7b, 0x66, 0x36, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x31,
    0xc9, 0x7c, 0xb6, 0xed, 0xdb, 0x00, 0x4d, 0xb6, 0xfb, 0x7b, 0x6d, 0xf6, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0x9e, 0x5d, 0xbd, 0xb4, 0xed, 0xdb, 0x7f, 0xed, 0xb6, 0xfb, 0x7b, 0x2d, 0xf2,
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x5d, 0x91, 0x34, 0xed, 0xdb, 0x40, 0x4d, 0xb2,
    0xfa, 0x45, 0x2d, 0xf2, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1e, 0xe7, 0x76, 0xcd,
    0xdb, 0x6e, 0x5d, 0xbb, 0x76, 0xdd, 0xad, 0xf3, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0x3c, 0xf7, 0x1c, 0x3b, 0x6e, 0xdd, 0x8b, 0x8c, 0xd0, 0x26, 0x07, 0x7f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0xf3, 0xe7, 0xdb, 0x31, 0x91, 0xec, 0xf9, 0xc7, 0xf7, 0xfc,
    0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x13, 0x1b, 0x9f, 0x31, 0x8e,
    0x07, 0xff, 0xf9, 0xc1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,
    0x43, 0xc0, 0x70, 0xbf, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xfe, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xf1, 0xbf, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0xfb, 0x3f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xdb, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static const uint8_t PROGMEM my_map2[] = {
    0x00, 0x70, 0x00, 0x00, 0x79, 0x03, 0xff, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1f, 0xff, 0x00, 0x20, 0x00, 0x03, 0xe9, 0x17, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x20, 0x00, 0x1f, 0x0c, 0x7f, 0xf8, 0x0f,
    0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0xfe,
    0x00, 0x7f, 0x9e, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x07, 0xe3, 0x07, 0xdf, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x02, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x01, 0xff, 0xff, 0xce, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x07,
    0xff, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01,
    0x00, 0x3f, 0x00, 0x3f, 0xe0, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x19, 0x99, 0x19, 0x01, 0xfc, 0x01, 0xff, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x90, 0x98, 0x1f, 0xc0, 0x0f, 0xe0, 0x07, 0xf8, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x91, 0x90, 0x95, 0xff, 0xc0, 0x3f, 0x80, 0x3f, 0xe0,
    0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x98, 0x80, 0x33, 0xb7, 0xff, 0xc0,
    0xfc, 0x07, 0xff, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x81, 0x80, 0x00,
    0x1b, 0xb5, 0x5a, 0x00, 0xe0, 0x0f, 0xf8, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
    0x19, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x01, 0xc0,
    0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd4,
    0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0xff, 0xff, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xfe, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60,
    0x01, 0x80, 0x00, 0x00, 0xae, 0xf7, 0x60, 0x00, 0x03, 0xf0, 0x3f, 0xf7, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x03, 0xfc, 0xff, 0x20, 0x00, 0x0f, 0x81, 0xff, 0xf7,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xc0, 0x00, 0x00, 0x00, 0x0c, 0x31, 0x80, 0x00,
    0x7d, 0x03, 0xff, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0xe1, 0x1f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x6d, 0xef, 0xef, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x06, 0x6d, 0xff, 0xdf,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e,
    0x03, 0xf5, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x1c, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xe5, 0x86, 0xf6, 0xfd, 0xfb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x87, 0x81, 0x7e, 0xfb, 0xb7, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x60, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x81, 0xf3, 0xe7, 0xcf,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfe,
    0x1d, 0xf3, 0x1f, 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xff, 0xff, 0xbb, 0xdc, 0xf3, 0x3e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x07,
    0x00, 0x00, 0x00, 0x06, 0x07, 0xff, 0xff, 0x8f, 0xdc, 0xfb, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0f, 0x1f, 0xff, 0xff, 0x8f, 0xf0, 0xfe, 0xf3, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0f, 0x7f, 0xfe, 0xfd, 0x0f,
    0xf9, 0xe6, 0xef, 0x80, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x07,
    0xff, 0xdf, 0xf0, 0x07, 0xff, 0xee, 0x3e, 0x60, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x0f, 0xff, 0xef, 0x80, 0x07, 0xff, 0xfe, 0x7c, 0x30, 0x00, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x3f, 0xff, 0xef, 0x00, 0x03, 0xfc, 0x79, 0xf8, 0x10,
    0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0xff, 0xff, 0xdc, 0x00, 0x03,
    0xf8, 0x73, 0xf9, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x03, 0xff,
    0xf7, 0x10, 0x00, 0x03, 0xf3, 0xcf, 0xdb, 0xe0, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
    0xc0, 0x00, 0x0f, 0xbf, 0xfb, 0x00, 0x00, 0x01, 0xe7, 0x9f, 0x8e, 0x00, 0x00, 0x00, 0x0c, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x3d, 0xff, 0xec, 0x00, 0x00, 0x00, 0x9e, 0x0f, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0xf7, 0xff, 0xe0, 0x00, 0x00, 0x00,
    0x3c, 0x07, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x03, 0x9e, 0xf8,
    0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x70, 0x0f, 0x7b, 0xe0, 0x04, 0x00, 0x00, 0x07, 0x76, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x30, 0x3f, 0xff, 0xc0, 0x1e, 0x00, 0x00, 0x07, 0xdf, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0xff, 0xbf, 0x80, 0x62, 0x00, 0x00, 0x1f,
    0x3f, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xde, 0x02,
    0xc2, 0x00, 0x00, 0x34, 0x7f, 0xe0, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1f, 0xff, 0xb8, 0x00, 0x48, 0x00, 0x01, 0xf8, 0xff, 0xe0, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xfb, 0xa0, 0x00, 0x20, 0x00, 0x05, 0xe3, 0xff, 0xc0, 0x00, 0x00,
    0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x0f, 0x47,
    0xfd, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x5e, 0x07, 0x80,
    0x00, 0x00, 0x0f, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f,
    0xff, 0xec, 0x02, 0xc0, 0x00, 0x00, 0x3c, 0x7f, 0xfc, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xff, 0xe7, 0xdc, 0x02, 0xc0, 0x00, 0x00, 0x7c, 0xff, 0xfe, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xf7, 0xf9, 0x83, 0xc0, 0x00, 0x01, 0xf9, 0xff,
    0xff, 0x80, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0xf9, 0xff, 0xe5, 0x4f, 0xe0,
    0x00, 0x07, 0xe3, 0xfe, 0xff, 0xc0, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb, 0xe1,
    0x3f, 0xe0, 0x4d, 0xd0, 0x00, 0x0d, 0xcf, 0xf8, 0xff, 0xe0, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x03, 0xef, 0x80, 0x3f, 0xe4, 0x68, 0x68, 0x00, 0x0f, 0x3f, 0xfe, 0xff, 0xf8, 0x00, 0x1c,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xbf, 0x00, 0x7f, 0xee, 0x31, 0xc0, 0x00, 0x0e, 0xff, 0xfe,
    0xdf, 0xfc, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x03, 0xbc, 0xff, 0x00, 0xff, 0x8c, 0x03, 0xc4,
    0x04, 0xe3, 0xff, 0x7f, 0xff, 0xfe, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00,
    0x7f, 0xc0, 0x0c, 0x62, 0x02, 0xe3, 0xfe, 0xef, 0xbf, 0xfe, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,
    0x07, 0xdb, 0x3c, 0x00, 0x7f, 0xc0, 0x18, 0x02, 0x66, 0x9f, 0xbb, 0x4f, 0x7f, 0xf8, 0x00, 0xe0,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x48, 0x1c, 0x00, 0x7e, 0xc0, 0x68, 0x20, 0x5f, 0x31, 0xf5, 0x5d,
    0xff, 0xf8, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x7d, 0xc3, 0xff, 0xc0, 0x3f, 0xe3, 0x8c, 0xf7,
    0x16, 0xe0, 0xc4, 0xb9, 0xff, 0xf0, 0x1d, 0x80, 0x00, 0x00, 0x00, 0x01, 0xef, 0x2e, 0x0c, 0xa0,
    0x1f, 0xe2, 0x0f, 0x3e, 0xf9, 0x81, 0x1c, 0xb7, 0xff, 0xe0, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x07,
    0x3c, 0xdf, 0xff, 0xa0, 0x3f, 0xf2, 0x3e, 0x1c, 0xb3, 0x82, 0x6d, 0x5f, 0xff, 0x80, 0x03, 0x80,
    0x00, 0x00, 0x00, 0x1e, 0x70, 0x93, 0xdf, 0x60, 0x3f, 0xb1, 0x72, 0x30, 0xcf, 0x81, 0xf7, 0xbf,
    0xfe, 0x06, 0x07, 0x80, 0x00, 0x00, 0x00, 0xf3, 0xc0, 0x0e, 0xe5, 0xd0, 0xef, 0xd8, 0xc7, 0x6c,
    0x37, 0x87, 0xfc, 0x7f, 0xff, 0x05, 0x0c, 0xc0, 0x00, 0x00, 0x03, 0xdf, 0xf0, 0x4b, 0x02, 0xeb,
    0x7f, 0xec, 0xf3, 0xb2, 0x63, 0x7f, 0x7c, 0xdc, 0x7f, 0x08, 0xb8, 0x60, 0x00, 0x00, 0x1f, 0x79,
    0xe0, 0x70, 0x03, 0xaf, 0xef, 0xfe, 0xc1, 0x64, 0xc1, 0x7f, 0x30, 0x3a, 0x7f, 0xd8, 0x38, 0x40,
    0x00, 0x00, 0x79, 0xf3, 0xc1, 0x80, 0x71, 0xd7, 0xfd, 0xf7, 0x59, 0x9b, 0x23, 0xfe, 0xc0, 0x1c,
    0xc7, 0xf2, 0x78, 0x00, 0x00, 0x01, 0xf7, 0x99, 0xd9, 0x18, 0x71, 0x7f, 0xf1, 0xf3, 0xf3, 0x77,
    0x37, 0xf6, 0x80, 0x0f, 0xaf, 0xec, 0xf0, 0x00, 0x00, 0x0f, 0xbe, 0x19, 0xf9, 0x98, 0x39, 0xbf,
    0x87, 0xf1, 0xcd, 0xcd, 0x9f, 0x78, 0x01, 0x05, 0xc7, 0xf8, 0xe0, 0x01, 0x00, 0x3e, 0xff, 0x1f,
    0xed, 0x98, 0x38, 0xf1, 0xbf, 0xfe, 0xfb, 0xb8, 0xff, 0xf8, 0x03, 0x81, 0x81, 0xef, 0xc0, 0x07,
    0x01, 0xfb, 0xef, 0xff, 0xec, 0xcd, 0xfc, 0xe0, 0xfe, 0xff, 0xff, 0xfe, 0xbb, 0xe0, 0x06, 0x81,
    0x80, 0xdf, 0x80, 0x06, 0x07, 0xef, 0xff, 0xff, 0xe2, 0xcf, 0x1f, 0xf3, 0xfc, 0x7f, 0xfd, 0xfe,
    0x37, 0xc0, 0x3e, 0x00, 0x00, 0x3f, 0x80, 0x0e, 0x1f, 0xff, 0xdf, 0xff, 0xe3, 0x66, 0x1f, 0xff,
    0xfd, 0xff, 0xff, 0xf8, 0x1f, 0x80, 0x78, 0x00, 0x00, 0x3f, 0x00, 0x0c, 0xff, 0xff, 0xdf, 0xf9,
    0xfb, 0x66, 0x7f, 0xff, 0x3f, 0xff, 0xff, 0xe0, 0x09, 0x01, 0x70, 0x00, 0x00, 0x0e, 0x00, 0x1c,
    0xff, 0xd8, 0xff, 0x3d, 0xf8, 0x67, 0xfd, 0xee, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x03, 0x80, 0x00,
    0x00, 0x1c, 0x00, 0x38, 0xff, 0x3e, 0xbf, 0x3d, 0xf8, 0xba, 0xd3, 0xff, 0xfb, 0xff, 0xff, 0xe0,
    0x00, 0x0f, 0x80, 0x00, 0x00, 0x18, 0x00, 0x30, 0xff, 0xfe, 0x1f, 0x3c, 0xe9, 0x9f, 0xcf, 0xff,
    0xff, 0xcf, 0xff, 0xf0, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0xff, 0x3b, 0x18, 0x38,
    0x84, 0x1c, 0x1b, 0xdf, 0xdf, 0xff, 0xfe, 0x38, 0x00, 0x30, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00,
    0xbf, 0x39, 0x26, 0x03, 0x80, 0x18, 0x1f, 0xbf, 0xff, 0xf7, 0xfe, 0x1c, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xe0, 0x00, 0x00, 0xfe, 0xff, 0xb8, 0x03, 0xc3, 0xf0, 0x1c, 0x1f, 0xbf, 0xfb, 0xf4, 0x07,
    0xc1, 0xc0, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x03, 0xc0, 0x70, 0x18, 0x1c,
    0xff, 0xdf, 0xc0, 0x07, 0xc1, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0xff, 0x3f, 0x80, 0xe3,
    0x3f, 0xf4, 0x1c, 0x07, 0xcf, 0x8f, 0x06, 0x03, 0xc7, 0xc0, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0xfd, 0x7f, 0x80, 0xf3, 0xe4, 0x74, 0x18, 0x0f, 0x5f, 0x9c, 0x0c, 0x02, 0xef, 0xe0, 0x00, 0x00,
    0x07, 0x00, 0x00, 0x00, 0xff, 0x3b, 0x80, 0xb0, 0x0d, 0xd4, 0x18, 0x7f, 0xfd, 0xf8, 0x0c, 0x03,
    0xfd, 0xe0, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x01, 0xf1, 0xff, 0x70, 0x1e, 0xfc,
    0xf8, 0x98, 0x0c, 0x0d, 0xff, 0x80, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x00, 0xe0,
    0x3f, 0x5f, 0xeb, 0x64, 0xf3, 0x30, 0x04, 0x0f, 0xcf, 0x0f, 0x80, 0x00, 0x1c, 0x00, 0x00, 0x00,
    0x0f, 0xff, 0xfd, 0xe0, 0x3f, 0x0c, 0x31, 0xf8, 0x64, 0x30, 0x0c, 0x1f, 0xff, 0x9f, 0x80, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xdf, 0x4c, 0x33, 0xf0, 0x78, 0x30, 0x0c, 0x1f,
    0xe7, 0xfc, 0x20, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xce, 0x1f, 0xe0,
    0x20, 0x30, 0x04, 0x1f, 0xff, 0xf8, 0x18, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x67, 0xfc, 0xcf,
    0xff, 0x1f, 0xff, 0xc2, 0x40, 0x30, 0x04, 0x1f, 0xff, 0xf8, 0x1c, 0x00, 0xe0, 0x00, 0x04, 0x00,
    0x00, 0x60, 0xfe, 0x40, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x30, 0x0c, 0x03, 0xff, 0xfc, 0x07, 0x00,
    0xc0, 0x00, 0x0e, 0x00, 0x00, 0x61, 0x3f, 0xc0, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x60, 0x0c, 0x03,
    0xff, 0xfe, 0x03, 0x01, 0xc0, 0x00, 0x06, 0x80, 0x00, 0x61, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0x07,
    0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0x01, 0x83, 0x80, 0x00, 0x01, 0xc0, 0x00, 0x0e, 0x07, 0xf0,
    0x00, 0x00, 0x00, 0x07, 0x1f, 0xff, 0xff, 0x87, 0xff, 0xff, 0x81, 0x03, 0x00, 0x00, 0x00, 0xe0,
    0x00, 0x02, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x05, 0x3e, 0x60, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07,
    0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x04, 0x3e, 0xe0, 0x00, 0xff,
    0xff, 0xfe, 0xf0, 0x0e, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x0d,
    0x3e, 0xc0, 0x00, 0x00, 0x0f, 0xfe, 0x38, 0x0c, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x0f,
    0xf0, 0x00, 0x00, 0x09, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x07,
    0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x08, 0x07, 0x3d, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x18,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x06,
    0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0f, 0xc0, 0x00, 0x04, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x10, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38,
    0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x13, 0xf8, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x07, 0x1c, 0x00, 0x00, 0x00, 0x11, 0x7e, 0x00, 0x00,
    0x0e, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00,
    0x08, 0x3f, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x78,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x0f, 0xc0, 0x00, 0x1e, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x03,
    0xc0, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x00, 0x1e, 0x00, 0x00, 0x01,
    0x80, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00,
    0x1c, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x78, 0x00, 0x3e, 0x13, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1e, 0x00, 0x18, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x1c, 0x00, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x38, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x0e, 0x07, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x30, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78,
    0x70, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x3e, 0x60, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xe0, 0x00, 0x00, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x01, 0xbc, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x78,
    0xf8, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x03, 0x1e, 0x00, 0x00, 0x00, 0x00, 0xef, 0x80,
    0x00, 0x00, 0x00, 0xfd, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0f, 0x00, 0x00,
    0x00, 0x00, 0xa7, 0xf0, 0x00, 0x00, 0x01, 0xef, 0x8f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0e, 0x07, 0x80, 0x00, 0xe8, 0x00, 0x00, 0x78, 0x00, 0x00, 0x01, 0x87, 0x83, 0xe0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1c, 0x03, 0xc0, 0x00, 0xef, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0x80, 0xf0, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0xe0, 0x00, 0x03, 0xf8, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x03, 0xc0, 0x3c, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x70, 0x00,
    0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xe0, 0x0f, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x00,
    0x70, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x70, 0x03, 0x80, 0x03,
    0xc0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,
    0x38, 0x01, 0xe0, 0x00, 0xf0, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x07, 0xf7, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x0c, 0x18, 0x00, 0x78, 0x00, 0x1c, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x03, 0xff};

// WIFI/GETREQUEST-Related **Variables**
const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 4000;  // size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 4000; // size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 3000;
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE];        // char array buffer to hold HTTP request
char request_buffer[IN_BUFFER_SIZE];   // char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; // char array buffer to hold HTTP response
char json_body[JSON_BODY_SIZE];
const int BUTTON_TIMEOUT = 1000; // button timeout in milliseconds

// WIFI-Related **CONSTANTS**
const char PREFIX[] = "{\"wifiAccessPoints\": [";                 // beginning of json body
const char SUFFIX[] = "]}";                                       // suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; // don't change this and don't share this
const uint8_t BUTTON = 45;
const int MAX_APS = 5;
const char NETWORK[] = "MIT";
const char PASSWORD[] = "";
uint8_t channel = 1;                                 // network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; // 6 byte MAC address of AP you're targeting.
int offset = 0;

//   const char AUDIO_PREFIX[] = "{\"config\":{\"encoding\":\"MULAW\",\"sampleRateHertz\":8000,\"languageCode\": \"en-US\",\"speechContexts\":[{\"phrases\":[\"riemann\", \"jacobian\"]}]}, \"audio\": {\"content\":\"";
//   const char AUDIO_SUFFIX[] = "\"}}"; //suffix to POST request_buffer
//   const int AUDIO_IN = 1; //pin where microphone is connected

// WIFI-Related **Global Variables**
uint8_t button_state;       // used for containing button state and detecting edges
int old_button_state;       // used for detecting button edges
uint32_t time_since_sample; // used for microsecond timing
WiFiClientSecure client;    // global WiFiClient Secure object
WiFiClient client2;         // global WiFiClient Secure object
StaticJsonDocument<500> doc;

// WIFI Miscl
int wifi_object_builder(char *object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t *mac_address)
{
    char buffer[100];
    int num_chars = sprintf(buffer, "{\"macAddress\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"signalStrength\":%d,\"age\":0,\"channel\": %d}", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5], signal_strength, channel);
    if (num_chars < os_len)
    {
        sprintf(object_string, "%s", buffer);
        return num_chars;
    }
    return 0;
}
char *SERVER = "googleapis.com"; // Server URL
uint32_t timer;

// Acceleration Variables
float x, x_avg, x_old, x_older;
float y, y_avg, y_old, y_older;
float z, z_avg, z_old, z_older;
const float ZOOM = 9.81; // for display (converts readings into m/s^2)...used for visualizing only

// State Machine Variables
enum game_mode
{
    Map,
    Catch
};
game_mode game_state = Map;
enum idle_mode
{
    Idle,
    Yes,
    No
};
idle_mode idle_state = Idle;
uint32_t ask_timer;
enum catch_mode
{
    Catch_Idle,
    Quitting,
    Waiting,
    Motion,
    Throw_End,
    Catch_Fail,
    Catch_Success
};
uint32_t state_timer = 0;
catch_mode catch_state = Catch_Idle; // state variable

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
char *begin_profedex = 0;

// Location Variables
double latitude = 0.0;
double longitude = 0.0;
uint32_t loc_timer = 0;

// audio variables
// const int DELAY = 1000;
// const int SAMPLE_FREQ = 8000;                          // Hz, telephone sample rate
// const int SAMPLE_DURATION = 5;                        // duration of fixed sampling (seconds)
// const int NUM_SAMPLES = SAMPLE_FREQ * SAMPLE_DURATION;  // number of of samples
// const int ENC_LEN = (NUM_SAMPLES + 2 - ((NUM_SAMPLES + 2) % 3)) / 3 * 4;  // Encoded length of clip
// bool conn;
// uint32_t time_since_sample;      // used for microsecond timing
// char speech_data[ENC_LEN + 200] = {0}; //global used for collecting speech data
// char heard[200];

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

// PROFEDEX Variables
int state;
const int IDLE = 0;
const int BUFFER = 1;
const int VIEWING = 2;
const int BUFFER2 = 3;
int idx;
// int offset;
int firsttime;

void setup()
{

    // LCD + SERIAL SETUP
    tft.init();
    tft.setRotation(1);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    for (int i = 0; i < 2560; i++)
    {
        flipped_map[i] = my_map[2559 - i];
    }
    tft.drawXBitmap(0, 0, flipped_map, 160, 128, 0xFFFF);
    Serial.begin(115200);
    delay(50); // pause to make sure comms get set up
    Wire.begin();
    delay(50); // pause to make sure comms get set up
    Serial.println("Welcome to Prof-emons Go! - Serial View");

    // IMU Setup
    if (imu.setupIMU(1))
    {
        Serial.println("IMU Connected!");
    }
    else
    {
        Serial.println("IMU Not Connected :/");
        Serial.println("Restarting");
        ESP.restart(); // restart the ESP (proper way)
    }

    // Button setup
    pinMode(BUTTON1, INPUT_PULLUP); // set input pin as an input button1
    pinMode(BUTTON2, INPUT_PULLUP); // set input pin as an input button2
    pinMode(BUTTON3, INPUT_PULLUP); // set input pin as an input button3
    pinMode(BUTTON4, INPUT_PULLUP); // set input pin as an input button4
    delay(4000);

    // Print start map screen
    for (int i = 0; i < 2560; i++)
    {
        flipped_map[i] = my_map2[2559 - i];
    }
    tft.fillScreen(TFT_BLACK);
    // tft.drawXBitmap(0, 0, flipped_map, 160, 128, 0xFFFF);
    tft.printf("Load Progress:\n");
    userdisplaytimer = millis();

    // Profedex Related Setup
    idx = 0;
    firsttime = 1;

    // SCAN WIFI AND CONNECT
    tft.printf("Scanning Wifi Networks...\n");
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    tft.printf("Analyzing Wifi Options...\n");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i)
        {
            Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
            uint8_t *cc = WiFi.BSSID(i);
            for (int k = 0; k < 6; k++)
            {
                Serial.print(*cc, HEX);
                if (k != 5)
                    Serial.print(":");
                cc++;
            }
            Serial.println("");
        }
    }
    delay(100); // wait a bit (100 ms)

    // if using regular connection use line below:
    WiFi.begin(NETWORK, PASSWORD);
    // if using channel/mac specification for crowded bands use the following:
    //  WiFi.begin(network, password, channel, bssid);

    uint8_t count = 0; // count used for Wifi check times
    Serial.print("Attempting to connect to ");
    Serial.println(NETWORK);
    while (WiFi.status() != WL_CONNECTED && count < 12)
    {
        delay(500);
        Serial.print(".");
        count++;
    }
    delay(2000);
    if (WiFi.isConnected())
    { // if we connected then print our IP, Mac, and SSID we're on
        Serial.println("CONNECTED!");
        tft.printf("Connected to WIFI!\n");
        Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                      WiFi.localIP()[1], WiFi.localIP()[0],
                      WiFi.macAddress().c_str(), WiFi.SSID().c_str());
        delay(500);
    }
    else
    { // if we failed to connect just Try again.
        Serial.println("Failed to Connect :/  Going to restart");
        Serial.println(WiFi.status());
        ESP.restart(); // restart the ESP (proper way)
    }
    timer = millis();
    loc_timer = millis();
    tft.fillScreen(TFT_BLACK);
}

void loop()
{
    //   Serial.println(overallstate);
    // READ DATA
    uint8_t button1 = digitalRead(BUTTON1);
    uint8_t button2 = digitalRead(BUTTON2);
    uint8_t button3 = digitalRead(BUTTON3);
    uint8_t button4 = digitalRead(BUTTON4);

    // DISPLAY DEBUGGING TEXT
    if (displaytext == true)
    {
        Serial.println("Current OverallState:");
        Serial.println(overallstate);
        // Serial.println("Current MapState: ");
        // Serial.println(mapstate);
        // Serial.println("Current CatchState: ");
        // Serial.println(catchstate);
        displaytext = false;
    }

    tft.setCursor(0, 0);
    //   update_location_and_profs();

    if (change_display == 1)
    {
        tft.fillScreen(TFT_BLACK);
        change_display = 0;
    }

    // MAIN STATES:
    if (overallstate == 0)
    {

        // Display User on map + update location/profemon detection
        if (millis() - userdisplaytimer > 10000)
        {
            Serial.println("Location Updating in map loop");
            userdisplaytimer = millis();
            // GET LOCATION VIA GEOLOCATION
            int offset = sprintf(json_body, "%s", PREFIX);
            int n = WiFi.scanNetworks(); // run a new scan. could also modify to use original scan from setup so quicker (though older info)
            Serial.println("scan done");
            if (n == 0)
            {
                Serial.println("no networks found");
            }
            else
            {
                // tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 120);
                tft.println("Updating location...");
                int max_aps = max(min(MAX_APS, n), 1);
                for (int i = 0; i < max_aps; ++i)
                {                                                                                                                             // for each valid access point
                    uint8_t *mac = WiFi.BSSID(i);                                                                                             // get the MAC Address
                    offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE - offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); // generate the query
                    if (i != max_aps - 1)
                    {
                        offset += sprintf(json_body + offset, ","); // add comma between entries except trailing.
                    }
                }
                sprintf(json_body + offset, "%s", SUFFIX);
                Serial.println(json_body);
                int len = strlen(json_body);
                // Make a HTTP request:
                Serial.println("SENDING REQUEST");
                request[0] = '\0'; // set 0th byte to null
                offset = 0;        // reset offset variable for sprintf-ing
                offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
                offset += sprintf(request + offset, "Host: googleapis.com\r\n");
                offset += sprintf(request + offset, "Content-Type: application/json\r\n");
                offset += sprintf(request + offset, "cache-control: no-cache\r\n");
                offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
                offset += sprintf(request + offset, "%s", json_body);
                do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
                Serial.println("-----------");
                Serial.println(response);
                Serial.println("-----------");

                if (strlen(response) > 1)
                {
                    char *begin = strchr(response, '{');
                    char *end = strrchr(response, '}');
                    char djd[500];
                    strncpy(djd, begin, end - begin + 1);
                    DeserializationError error = deserializeJson(doc, djd);
                    if (error)
                    {
                        Serial.print(F("deserializeJson() failed: "));
                        Serial.println(error.f_str());
                    }
                    userlocationy = doc["location"]["lat"];
                    userlocationx = doc["location"]["lng"];
                    latitude = doc["location"]["lat"];
                    longitude = doc["location"]["lng"];

                    strcpy(request, "");
                    request[0] = '\0'; // set 0th byte to null
                    offset = 0;        // reset offset variable for sprintf-ing
                    sprintf(json_body, "lat=%f&lon=%f\r\n", latitude, longitude);
                    Serial.println(json_body);
                    len = strlen(json_body);
                    offset += sprintf(request + offset, "POST http://608dev-2.net/sandbox/sc/team3/profemon_geolocation.py HTTP/1.1\r\n");
                    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
                    offset += sprintf(request + offset, "Content-Type: application/x-www-form-urlencoded\r\n");
                    offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
                    offset += sprintf(request + offset, "%s", json_body);
                    Serial.println(request);
                    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
                    Serial.println("-----------");
                    Serial.println(response);
                    Serial.println("-----------");

                    strcpy(request_buffer, "");
                    strcpy(response_buffer, "");
                    request_buffer[0] = '\0';  // set 0th byte to null
                    response_buffer[0] = '\0'; // set 0th byte to null
                    offset = 0;                // reset offset variable for sprintf-ing

                    sprintf(request_buffer, "GET /sandbox/sc/team3/nearby_profemon.py?lat=%f&lon=%f HTTP/1.1\r\nHost: 608dev-2.net\r\n\r\n", latitude, longitude);
                    do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
                    Serial.printf("%s, %d", response_buffer, strlen(response_buffer));
                    char *comma = strchr(response_buffer, ',');
                    if (comma != NULL)
                    {
                        strncpy(profemon_name, response_buffer, comma - response_buffer);
                        strcpy(display_name, comma + 1);
                    }
                    else if (overallstate == 0)
                    { // game_state = Map
                        strcpy(profemon_name, "");
                        strcpy(display_name, "");
                    }

                    Serial.println(profemon_name);
                    Serial.println(display_name);
                    // change_display = 1;
                }
            }

            // PRINTTFT LOCATION
            Serial.println(userlocationy, 6);
            Serial.println(userlocationx, 6);
            tft.fillScreen(TFT_BLACK);
            tft.drawXBitmap(0, 0, flipped_map, 160, 128, 0xFFFF);
            userdisplaytimer = millis();
            float percentagex = abs((userlocationx - tft_x_min_geolocation) / (tft_x_max_geolocation - tft_x_min_geolocation));
            float percentagey = abs((userlocationy - tft_y_min_geolocation) / (tft_y_max_geolocation - tft_y_min_geolocation));
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 3, TFT_BLUE);
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 4, TFT_RED);
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 5, TFT_BLUE);
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 6, TFT_RED);
        }

        update_idle_mode(digitalRead(BUTTON1), digitalRead(BUTTON2));
        if (idle_state == Idle)
        {
            if (button3 == 0)
            {
                tft.printf("Player %s is nearby...\nWould you like to fight them?\n\nYES: Button 1\nNO: Button 2\n", display_name);
                battle_or_catch = true;
                // if other player nearby to battle
            }
            else if (strlen(profemon_name) != 0 && strlen(display_name) != 0)
            {
                tft.printf("There is a %s nearby...\nWould you like to catch them?\n\nYES: Button 1\nNO: Button 2\n", display_name);
                // Serial.println("Bootleg text: do you want to catch them? Yes=Button1, No=Button2");
                // Serial.println("PROFEMON DETECTED");
            }
            else
            {
                //   Serial.println("IDLE");
            }
        }

        if (button4 == 0)
        {
            overallstate = 9;
            //   delay(500);
            //   Serial.println("It gets here");
        }
    }

    else if (overallstate == 2)
    {
        // tft.fillScreen(TFT_BLACK);
        update_catch_mode(digitalRead(BUTTON1), digitalRead(BUTTON2), motion);
        is_moving();
        catch_display();
    }

    else if (overallstate == 9)
    {
        switch_state(button1);
        profedex_navigator(button2, button3);
        if (button4 == 0)
        {
            overallstate = 0;
            Serial.println("It goes back to main");
            firsttime = 1;
            // GET LOCATION VIA GEOLOCATION
            int offset = sprintf(json_body, "%s", PREFIX);
            int n = WiFi.scanNetworks(); // run a new scan. could also modify to use original scan from setup so quicker (though older info)
            Serial.println("scan done");
            if (n == 0)
            {
                Serial.println("no networks found");
            }
            else
            {
                // tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 120);
                tft.println("Updating location...");
                int max_aps = max(min(MAX_APS, n), 1);
                for (int i = 0; i < max_aps; ++i)
                {                                                                                                                             // for each valid access point
                    uint8_t *mac = WiFi.BSSID(i);                                                                                             // get the MAC Address
                    offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE - offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); // generate the query
                    if (i != max_aps - 1)
                    {
                        offset += sprintf(json_body + offset, ","); // add comma between entries except trailing.
                    }
                }
                sprintf(json_body + offset, "%s", SUFFIX);
                Serial.println(json_body);
                int len = strlen(json_body);
                // Make a HTTP request:
                Serial.println("SENDING REQUEST");
                request[0] = '\0'; // set 0th byte to null
                offset = 0;        // reset offset variable for sprintf-ing
                offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
                offset += sprintf(request + offset, "Host: googleapis.com\r\n");
                offset += sprintf(request + offset, "Content-Type: application/json\r\n");
                offset += sprintf(request + offset, "cache-control: no-cache\r\n");
                offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
                offset += sprintf(request + offset, "%s", json_body);
                do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
                Serial.println("-----------");
                Serial.println(response);
                Serial.println("-----------");

                if (strlen(response) > 0)
                {
                    char *begin = strchr(response, '{');
                    char *end = strrchr(response, '}');
                    char djd[500];
                    strncpy(djd, begin, end - begin + 1);
                    DeserializationError error = deserializeJson(doc, djd);
                    if (error)
                    {
                        Serial.print(F("deserializeJson() failed: "));
                        Serial.println(error.f_str());
                    }
                    userlocationy = doc["location"]["lat"];
                    userlocationx = doc["location"]["lng"];

                    strcpy(request, "");
                    request[0] = '\0'; // set 0th byte to null
                    offset = 0;        // reset offset variable for sprintf-ing
                    sprintf(json_body, "lat=%f&lon=%f\r\n", latitude, longitude);
                    Serial.println(json_body);
                    len = strlen(json_body);
                    offset += sprintf(request + offset, "POST http://608dev-2.net/sandbox/sc/team3/profemon_geolocation.py HTTP/1.1\r\n");
                    offset += sprintf(request + offset, "Host: 608dev-2.net\r\n");
                    offset += sprintf(request + offset, "Content-Type: application/x-www-form-urlencoded\r\n");
                    offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
                    offset += sprintf(request + offset, "%s", json_body);
                    Serial.println(request);
                    do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
                    Serial.println("-----------");
                    Serial.println(response);
                    Serial.println("-----------");
                }
            }

            // PRINTTFT LOCATION
            Serial.println(userlocationy, 6);
            Serial.println(userlocationx, 6);
            tft.fillScreen(TFT_BLACK);
            tft.drawXBitmap(0, 0, flipped_map, 160, 128, 0xFFFF);
            userdisplaytimer = millis();
            float percentagex = abs((userlocationx - tft_x_min_geolocation) / (tft_x_max_geolocation - tft_x_min_geolocation));
            float percentagey = abs((userlocationy - tft_y_min_geolocation) / (tft_y_max_geolocation - tft_y_min_geolocation));
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 3, TFT_BLUE);
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 4, TFT_RED);
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 5, TFT_BLUE);
            tft.drawCircle(percentagex * 160, 128 - percentagey * 128, 6, TFT_RED);
        }
    }

    overallstatefunction(button1, button2, button3, button4);
    old_button1 = button1;
    old_button2 = button2;
    old_button3 = button3;
    old_button4 = button4;
}

void overallstatefunction(uint8_t button1, uint8_t button2, uint8_t button3, uint8_t button4)
{
    switch (overallstate)
    {
    case (0):
        // Map Mode
        break;
    case (1):
        // Display nice UI graphics
        overallstate = 2;
        displaytext = true;
        Serial.println("State 1");
        Serial.println("State 2");
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(10, 50);
        tft.printf("Catch that Prof-emon!!!");
        delay(3000);
        break;
    case (2):
        // Catch Mode
        break;
    case (3):
        // Display nice UI graphics
        overallstate = 0;
        displaytext = true;
        Serial.println("State 3");
        Serial.println("State 0");
        break;
    case (4):
        // Display nice UI graphics + Store Prof-emon in Database
        Serial.println("YOU CAUGHT A POKEMON");
        overallstate = 0;
        displaytext = true;
        Serial.println("State 4");
        Serial.println("State 0");
        break;
    case (5):
        // Display nice UI graphics
        overallstate = 6;
        displaytext = true;
        start_battle();
        battle_state = 0;
        Serial.println("State 5");
        break;
    case (6):
        // Fight Mode
        update_battle_mode(button1, button2, button3, old_button1, old_button2, old_button3);
        break;
    // case(7):
    //   //Display nice UI graphics that say YOU LOST
    //   Serial.println("YOU LOST");
    //   overallstate = 0;
    //   displaytext = true;
    //   Serial.println("State 7");
    //   break;
    // case(8):
    //   Serial.println("YOU WON");
    //   overallstate = 0;
    //   displaytext = true;
    //   Serial.println("State 8");
    //   break;
    case (9):
        break;
    }
}

const char *CA_CERT =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n"
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n"
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n"
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n"
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n"
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n"
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n"
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n"
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n"
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n"
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n"
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n"
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n"
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n"
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n"
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n"
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n"
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n"
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n"
    "-----END CERTIFICATE-----\n";

uint8_t char_append(char *buff, char c, uint16_t buff_size)
{
    int len = strlen(buff);
    if (len > buff_size)
        return false;
    buff[len] = c;
    buff[len + 1] = '\0';
    return true;
}

void do_http_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial)
{
    if (client2.connect(host, 80))
    { // try to connect to host on port 80
        if (serial)
            Serial.print(request); // Can do one-line if statements in C without curly braces
        client2.print(request);
        memset(response, 0, response_size); // Null out (0 is the value of the null terminator '\0') entire buffer
        uint32_t count = millis();
        while (client2.connected())
        { // while we remain connected read out data coming back
            client2.readBytesUntil('\n', response, response_size);
            if (serial)
                Serial.println(response);
            if (strcmp(response, "\r") == 0)
            { // found a blank line!
                break;
            }
            memset(response, 0, response_size);
            if (millis() - count > response_timeout)
                break;
        }
        memset(response, 0, response_size);
        count = millis();
        while (client2.available())
        { // read out remaining text (body of response)
            char_append(response, client2.read(), OUT_BUFFER_SIZE);
        }
        if (serial)
            Serial.println(response);
        client2.stop();
        if (serial)
            Serial.println("-----------");
    }
    else
    {
        if (serial)
            Serial.println("connection failed :/");
        if (serial)
            Serial.println("wait 0.5 sec...");
        client2.stop();
    }
}

void do_https_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial)
{
    client.setHandshakeTimeout(30);
    client.setCACert(CA_CERT); // set cert for https
    if (client.connect(host, 443, 4000))
    { // try to connect to host on port 443
        if (serial)
            Serial.print(request); // Can do one-line if statements in C without curly braces
        client.print(request);
        response[0] = '\0';
        // memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
        uint32_t count = millis();
        while (client.connected())
        { // while we remain connected read out data coming back
            client.readBytesUntil('\n', response, response_size);
            if (serial)
                Serial.println(response);
            if (strcmp(response, "\r") == 0)
            { // found a blank line!
                break;
            }
            memset(response, 0, response_size);
            if (millis() - count > response_timeout)
                break;
        }
        memset(response, 0, response_size);
        count = millis();
        while (client.available())
        { // read out remaining text (body of response)
            char_append(response, client.read(), OUT_BUFFER_SIZE);
        }
        if (serial)
            Serial.println(response);
        client.stop();
        if (serial)
            Serial.println("-----------");
    }
    else
    {
        if (serial)
            Serial.println("connection failed :/");
        if (serial)
            Serial.println("wait 0.5 sec...");
        client.stop();
    }
}

void start_battle()
{
    // select profemon // Yuebin's code
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.printf("You have chosen %s as your Prof-emon!\n", profemon_id);
    tft.printf("Loading...");

    num_turns = 0;
    battle_state = 0;
    make_server_request(start);
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.printf("Waiting for your game to start...\n");
    time_since_turn_check = millis();
}

void update_battle_mode(int in1, int in2, int in3, int old1, int old2, int old3)
{
    if (battle_state == 0)
    {
        // waiting for it to be our move. every [time period], we send a GET request_buffer labeled "turn" to ask whose turn it is
        if (millis() - time_since_turn_check > turn_check_period)
        {
            make_server_request(turn);
        }

        // if it's our turn, the function changed state to 1. we need to update the hp values, the screen (depends on whether the second move can be used yet), and the state value
        if (battle_state == 1)
        {
            // if the other player suddenly has hp zero, they forfeited, so you win! yay!
            if (other_hp == 0)
            {
                Serial.println("they lost :D");
                tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 0);
                tft.printf("Your opponent has FORFEITED!\n\n");
                tft.printf("%s hp: %u\n", profemon_id, player_hp);
                tft.printf("Your opponent hp: 0\n");
                tft.printf("Congratulations, you won!\n");
                Serial.printf("state is now 3\n");
                battle_state = 2;
            }

            // if YOUR hp is zero, you lost! shit!
            else if (player_hp == 0)
            {
                Serial.printf("oopsie doopsie");
                tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 0);
                tft.printf("Your opponent used %s!\nIt was super effective!\n\n", last_move);
                tft.printf("%s hp: %u\n", profemon_id, player_hp);
                tft.printf("Your opponent hp: %u\n", other_hp);

                tft.printf("Unfortunately, you lost! Better luck next time.\n");
                Serial.printf("state is now 3\n");
                battle_state = 2;
            }

            // otherwise, it was just a normal move -- time to start our move
            else
            {
                Serial.printf("time to start the move\n");
                num_turns++;
                tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 0);
                // if their attack did less damage than a third of our attack value, it's considered "not very effective" (by our standards)
                if (old_player_hp - player_hp > 0 && old_player_hp - player_hp < (int)((double)max_attack / 3.0))
                {
                    tft.printf("Your opponent used %s with damage %u!\nIt was not very effective.\n\n", last_move, (old_player_hp - player_hp));
                }
                // otherwise, solid attack!
                else if (old_player_hp - player_hp > 0)
                {
                    tft.printf("Your opponent used %s with damage %u!\n\n", last_move, (old_player_hp - player_hp));
                }
                else
                {
                    tft.printf("Your turn!\n\n");
                }

                tft.printf("%s hp: %u\n", profemon_id, player_hp);
                tft.printf("Your opponent hp: %u\n\n", other_hp);

                tft.printf("Choose a move!\n");
                tft.printf("Standard attack: %s\n", standard_move_name);
                if (num_turns > 2)
                {
                    tft.printf("Special attack: %s\n", special_move_name);
                }
                tft.printf("Press the third button to forfeit the match.");

                time_since_turn_started = millis();
            }
        }
    }
    else if (battle_state == 1)
    {
        // okay, it's now our turn, so we're going to wait for a button to be pressed

        // if the first button is pressed, they want to do the gesture
        if (old1 != in1 && in1 == 1)
        {
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0);
            tft.printf("You have selected the move %s. To perform this move, follow the instructions.\n", standard_move_name);
            start_time = millis();
            // calculate the amount of damage done and send it over
            damage_done = battle_direction(gesture);
            memset(last_move, 0, 50);
            strcpy(last_move, standard_move_name);

            make_server_request(attack);
            // make_server_request(turn);

            // if the other player's new hp is zero, you win! yay!
            if (other_hp - damage_done < 1)
            {
                tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 0);
                tft.printf("You used %s with %u damage! It was super effective!\n\n", last_move, damage_done);
                tft.printf("%s hp: %u\n", profemon_id, player_hp);
                tft.printf("Your opponent hp: 0\n\n");

                tft.printf("Your opponent has fainted!\nCongratulations, you won!\n");
                Serial.printf("state is now 3\n");
                battle_state = 2;
            }

            // otherwise, standard message, and back to checking when our move begins
            else
            {
                tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 0);
                if (damage_done > (int)(3.0 * (double)max_attack / 4.0))
                {
                    tft.printf("You used %s with %u damage! It was super effective!\n\n", last_move, damage_done);
                }
                else if (damage_done < (int)((double)max_attack / 3.0))
                {
                    tft.printf("You used %s with %u damage! It was not very effective.\n\n", last_move, damage_done);
                }
                else
                {
                    tft.printf("You used %s with %u damage!\n\n", last_move, damage_done);
                }
                tft.printf("%s hp: %u\n", profemon_id, player_hp);
                tft.printf("Your opponent hp: %u\n", other_hp - damage_done);

                tft.printf("Waiting");
                delay(100);
                tft.printf(".");
                delay(100);
                tft.printf(".");
                delay(100);
                tft.printf(".\n");

                old_player_hp = player_hp;
                Serial.printf("state is now 0\n");
                battle_state = 0;
                time_since_turn_check = millis();
            }
        }

        // if the second button is pressed, they want to do the voice recognition
        else if (num_turns > 2 && old2 != in2 && in2 == 1)
        {
            Serial.println("you have selected the SPECIAL move");
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0);
            tft.printf("You have selected the move %s.\n", special_move_name);
            tft.printf("Get ready to shout in...\n");
            tft.printf("3\n");
            delay(500);
            tft.printf("2\n");
            delay(500);
            tft.printf("1\n");
            delay(500);
            tft.printf("Speak!\n");

            // voice recognition goes here. they have three chances, or they do zero damage
            int i = 0;
            //   while(i<3) {
            //     // hear_command();

            //     Serial.printf("supposedly, |%s| and |%s| are super different /s\n",heard,special_move_name);
            //     //Serial.printf("and the difference between them is %d or maybe %d\n",strncmp(heard,special_move_name,strlen(special_move_name)),strcmp(heard,special_move_name));
            //     if(strncmp(heard,special_move_name,strlen(special_move_name))==0) {
            //       break;
            //     }
            //     else {
            //       tft.fillScreen(TFT_BLACK);
            //       tft.setCursor(0,0);
            //       tft.printf("Try again!\n");
            //       tft.printf("Get ready to shout %s in...\n",special_move_name);
            //       tft.printf("3\n");
            //       delay(500);
            //       tft.printf("2\n");
            //       delay(500);
            //       tft.printf("1\n");
            //       delay(500);
            //       tft.printf("Speak!\n");
            //     }

            //     i++;
            //   }

            if (i < 3)
            {
                damage_done = max_attack;
            }
            else
            {
                damage_done = 0;
            }

            // calculate the amount of damage done and send it over. if the other player's new hp is zero, you win! yay!

            memset(last_move, 0, 50);
            strcpy(last_move, special_move_name);

            make_server_request(attack);
            // make_server_request(turn);

            if (other_hp - damage_done < 1)
            {
                tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 0);
                tft.printf("You used %s with %u damage! It was super effective!\n\n", last_move, damage_done);
                tft.printf("%s hp: %u\n", profemon_id, player_hp);
                tft.printf("Your opponent hp: 0\n\n");

                tft.printf("Your opponent has fainted!\nCongratulations, you won!\n");
                Serial.printf("state is now 3\n");
                battle_state = 2;
            }

            // otherwise, standard message, and back to checking when our move begins
            else
            {
                tft.fillScreen(TFT_BLACK);
                tft.setCursor(0, 0);
                if (damage_done > (int)(3.0 * (double)max_attack / 4.0))
                {
                    tft.printf("You used %s with %u damage! It was super effective!\n\n", last_move, damage_done);
                }
                else if (damage_done < (int)((double)max_attack / 3.0))
                {
                    tft.printf("You used %s with %u damage! It was not very effective.\n\n", last_move, damage_done);
                }
                else
                {
                    tft.printf("You used %s with %u damage!\n\n", last_move, damage_done);
                }
                tft.printf("%s hp: %u\n", profemon_id, player_hp);
                tft.printf("Your opponent hp: %u\n", other_hp - damage_done);

                tft.printf("Waiting for your opponent");
                delay(100);
                tft.printf(".");
                delay(100);
                tft.printf(".");
                delay(100);
                tft.printf(".");

                old_player_hp = player_hp;
                Serial.printf("state is now 0\n");
                battle_state = 0;
                time_since_turn_check = millis();
            }
        }

        // if the third button is pressed, or they haven't done anything in five minutes, they forfeit the match
        else if ((old3 != in3 && in3 == 0) || (millis() - time_since_turn_started > max_turn_length))
        {
            Serial.println("you have FORFEITED the match :(");
            // how does that work, you may ask? send a POST request_buffer with the game_id and your player_id labeled "forfeit"
            make_server_request(forfeit);
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0);
            tft.printf("You have FORFEITED!\n\n");
            tft.printf("Better luck next time :(\n");
            Serial.printf("state is now 3\n");
            battle_state = 2;
        }
    }

    else if (battle_state == 2)
    {
        battle_state = 0;
        overallstate = 0;
        delay(5000);
        Serial.println("game over");
        tft.fillScreen(TFT_BLACK);
    }
}

void make_server_request(int type)
{
    strcpy(request_buffer, "");
    request_buffer[0] = '\0'; // set 0th byte to null
    offset = 0;               // reset offset variable for sprintf-ing

    if (type == start)
    {
        // send a POST request_buffer labeled "start" with the player_id and prof_id
        strcpy(request_buffer, "");
        strcpy(profemon_id, "Erik Demaine");
        request_buffer[0] = '\0';
        offset = 0;
        offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=start&player_id=%s&prof_id=%s  HTTP/1.1\r\n", user, profemon_id);
        offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

        // info to store: game_id, gesture, our own hp (as both player_hp and old_player_hp)
        char breaks[2];
        breaks[0] = ',';
        breaks[1] = '\0';
        char *ptr = strtok(response_buffer, breaks);

        // Serial.printf("1: %s",ptr);
        game_id = atoi(ptr);
        ptr = strtok(NULL, breaks);

        // Serial.printf("2: %s",ptr);
        gesture = atoi(ptr);
        ptr = strtok(NULL, breaks);

        // Serial.printf("3: %s",ptr);
        player_hp = atoi(ptr);
        old_player_hp = player_hp;

        Serial.printf("GAME HAS BEGUN! id is %u\n", game_id);

        // send a GET request_buffer to profedex.py asking for attack
        strcpy(request_buffer, "");
        request_buffer[0] = '\0';
        offset = 0;
        offset += sprintf(request_buffer + offset, "GET http://608dev-2.net/sandbox/sc/team3/profedex.py?professor=%s&item=attack  HTTP/1.1\r\n", profemon_id);
        offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

        // info to store: our own attack value
        max_attack = atoi(response_buffer);

        Serial.printf("max attack score is: %u\n", max_attack);

        // send ANOTHER request_buffer to figure out what your prof-emon's two moves are
        strcpy(request_buffer, "");
        request_buffer[0] = '\0';
        offset = 0;
        offset += sprintf(request_buffer + offset, "GET http://608dev-2.net/sandbox/sc/team3/profedex.py?professor=%s&item=moves  HTTP/1.1\r\n", profemon_id);
        offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

        char *ending1 = strrchr(response_buffer, '\n');
        *(ending1 + 0) = NULL;

        // Serial.printf("%s\n",response);

        // info to store: the two moves
        ptr = strtok(response_buffer, breaks);

        memset(standard_move_name, 0, 50);
        strcpy(standard_move_name, ptr);
        ptr = strtok(NULL, breaks);

        memset(special_move_name, 0, 50);
        strcpy(special_move_name, ptr);

        Serial.printf("The real moves are %s and %s\n", standard_move_name, special_move_name);
    }
    else if (type == turn)
    {
        // send a GET request_buffer labeled "turn" with the game_id and player_id
        offset += sprintf(request_buffer + offset, "GET http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=turn&player_id=%s&game_id=%u  HTTP/1.1\r\n", user, game_id);
        offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

        // if the response is just "-1", the game hasn't started yet, and we should just keep waiting for the other player to enter the game
        if (strncmp(response_buffer, "-1", 2) == 0)
        {
            return;
        }

        else if (battle_state == 1)
        {
            Serial.printf("at the time, the turn is %s\n", response_buffer);
        }

        // info to store: the id of whose turn it is, our own hp, the other player's hp, and the last move
        char breaks[2];
        breaks[0] = ',';
        breaks[1] = '\0';
        char *ptr = strtok(response_buffer, breaks);

        if (strcmp(ptr, user) == 0)
        {
            Serial.printf("state is now 1\n");
            battle_state = 1;
        }

        ptr = strtok(NULL, breaks);
        player_hp = atoi(ptr);

        ptr = strtok(NULL, breaks);
        other_hp = atoi(ptr);

        ptr = strtok(NULL, breaks);
        if (strncmp(response_buffer, "n/a", 3) == 0)
        {
            memset(last_move, 0, 50);
        }
        else
        {
            memset(last_move, 0, 50);
            strcpy(last_move, ptr);
        }

        // Serial.printf("this will never be reached :/");
    }
    else if (type == attack)
    {
        // send a POST request_buffer labeled "attack" with the game_id, player_id, move, and damage
        strcpy(request_buffer, "");
        request_buffer[0] = '\0';
        offset = 0;
        offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=attack&player_id=%s&game_id=%u&move=%s&damage=%u  HTTP/1.1\r\n", user, game_id, last_move, damage_done);
        offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
        Serial.printf("sending an attack request_buffer with url %s\n", request_buffer);
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        Serial.printf("request_buffer has been sent\n");
    }
    else if (type == forfeit)
    {
        // send a POST request_buffer labeled "forfeit" with the game_id and player_id
        strcpy(request_buffer, "");
        request_buffer[0] = '\0';
        offset = 0;
        offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/battle_brain.py?label=forfeit&player_id=%s&game_id=%u  HTTP/1.1\r\n", user, game_id);
        offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    }
}

int battle_direction(int what_to_do)
{
    battlemode = what_to_do;
    gesture_total = 0;
    switch (battlemode)
    {
    case x_shake:
        tft.println("SHAKE LEFT AND RIGHT");
        while ((millis() - start_time) < gesture_duration)
        {
            imu.readAccelData(imu.accelCount);
            x = abs(ZOOM * imu.accelCount[0] * imu.aRes);

            x_avg = (x + x_old + x_older) / 3.0;
            x_older = x_old;
            x_old = x;

            // char output[100];
            // sprintf(output,"X:%4.2f", x_avg);
            // Serial.println(output);

            if (x_avg > 6)
            {
                gesture_total += 1;
            }
        }
        break;
    case y_shake:
        tft.println("SHAKE UP AND DOWN");
        while ((millis() - start_time) < gesture_duration)
        {
            imu.readAccelData(imu.accelCount);
            y = abs(ZOOM * imu.accelCount[1] * imu.aRes);

            y_avg = (y + y_old + y_older) / 3.0;
            y_older = y_old;
            y_old = y;

            // char output[100];
            // sprintf(output,"Y:%4.2f", y_avg);
            // Serial.println(output);

            if (y_avg > 12)
            {
                gesture_total += 1;
            }
        }
        break;
    case z_shake:
        tft.println("SHAKE FORWARD AND BACK");
        while ((millis() - start_time) < gesture_duration)
        {
            imu.readAccelData(imu.accelCount);
            z = abs(ZOOM * imu.accelCount[2] * imu.aRes);

            z_avg = (z + z_old + z_older) / 3.0;
            z_older = z_old;
            z_old = z;

            // char output[100];
            // sprintf(output,"Z:%4.2f", z_avg);
            // Serial.println(output);

            if (z_avg > 6)
            {
                gesture_total += 1;
            }
        }
        break;
    }

    return (gesture_total * max_attack / 2500);
}

// void hear_command() {
//   //Serial.println("listening...");
//   record_audio();
//   //Serial.println("sending...");
//   //Serial.print("\nStarting connection to server...");
//   delay(300);
//   conn = false;
//   for (int i = 0; i < 10; i++) {
//     int val = (int)client2.connect("speech.google.com", 443, 4000);
//     Serial.print(i); Serial.print(": "); Serial.println(val);
//     if (val != 0) {
//       conn = true;
//       break;
//     }
//     Serial.print(".");
//     delay(300);
//   }
//   if (!conn) {
//     Serial.println("Connection failed!");
//     return;
//   }
//   else {
//     Serial.println("Connected to server!");
//     Serial.println(client2.connected());
//     int len = strlen(speech_data);
//     // Make a HTTP request_buffer:
//     client2.print("POST /v1/speech:recognize?key="); client2.print(API_KEY); client2.print(" HTTP/1.1\r\n");
//     client2.print("Host: speech.googleapis.com\r\n");
//     client2.print("Content-Type: application/json\r\n");
//     client2.print("cache-control: no-cache\r\n");
//     client2.print("Content-Length: "); client2.print(len);
//     client2.print("\r\n\r\n");
//     int ind = 0;
//     int jump_size = 1000;
//     char temp_holder[jump_size + 10] = {0};
//     Serial.println("sending data");
//     while (ind < len) {
//       delay(80);//experiment with this number!
//       //if (ind + jump_size < len) client.print(speech_data.substring(ind, ind + jump_size));
//       strncat(temp_holder, speech_data + ind, jump_size);
//       client2.print(temp_holder);
//       ind += jump_size;
//       memset(temp_holder, 0, sizeof(temp_holder));
//     }
//     client2.print("\r\n");
//     //Serial.print("\r\n\r\n");
//     //Serial.println("Through send...");
//     unsigned long count = millis();
//     while (client2.connected()) {
//       //Serial.println("IN!");
//       String line = client2.readStringUntil('\n');
//       Serial.print(line);
//       if (line == "\r") { //got header of response
//         //Serial.println("headers received");
//         break;
//       }
//       if (millis() - count > RESPONSE_TIMEOUT) break;
//     }
//     //Serial.println("");
//     //Serial.println("Response...");
//     count = millis();
//     while (!client2.available()) {
//       delay(100);
//       //Serial.print(".");
//       if (millis() - count > RESPONSE_TIMEOUT) break;
//     }
//     Serial.println();
//     //Serial.println("-----------");
//     memset(response_buffer, 0, sizeof(response_buffer));
//     while (client2.available()) {
//       char_append(response_buffer, client2.read(), OUT_BUFFER_SIZE);
//     }
//     //Serial.println(response); //comment this out if needed for debugging
//     char* trans_id = strstr(response_buffer, "transcript");
//     char transcript[100] = {0};
//     if (trans_id != NULL) {
//       char* foll_coll = strstr(trans_id, ":");
//       char* starto = foll_coll + 3; //starting index
//       char* endo = strstr(starto + 1, "\""); //ending index
//       int transcript_len = endo - starto;

//       strncat(transcript, starto, transcript_len);
//       Serial.printf("We heard you say: %s\n",transcript);

//       memset(heard,0,200);
//       strcpy(heard,transcript);
//     }
//     //Serial.println("-----------");
//     client2.stop();
//     Serial.println("done");
//   }
// }

// void record_audio() {
//   int sample_num = 0;    // counter for samples
//   int enc_index = strlen(AUDIO_PREFIX) - 1;  // index counter for encoded samples
//   float time_between_samples = 1000000 / SAMPLE_FREQ;
//   int value = 0;
//   char raw_samples[3];   // 8-bit raw sample data array
//   memset(speech_data, 0, sizeof(speech_data));
//   sprintf(speech_data, "%s", AUDIO_PREFIX);
//   char holder[5] = {0};
//   Serial.println("starting");
//   uint32_t text_index = enc_index;
//   uint32_t start = millis();
//   time_since_sample = micros();
//   while (sample_num < NUM_SAMPLES) { //read in NUM_SAMPLES worth of audio data
//     value = analogRead(AUDIO_IN);  //make measurement
//     raw_samples[sample_num % 3] = mulaw_encode(value - 1800); //remove 1.5ishV offset (from 12 bit reading)
//     sample_num++;
//     if (sample_num % 3 == 0) {
//       base64_encode(holder, raw_samples, 3);
//       strncat(speech_data + text_index, holder, 4);
//       text_index += 4;
//     }
//     // wait till next time to read
//     while (micros() - time_since_sample <= time_between_samples); //wait...
//     time_since_sample = micros();
//   }
//   Serial.println(millis() - start);
//   sprintf(speech_data + strlen(speech_data), "%s", AUDIO_SUFFIX);
//   Serial.println("out");
// }

// State machine that changes when profemon is nearby
// Transition from map state to catch state
void update_idle_mode(int in1, int in2)
{
    switch (idle_state)
    {
    case Idle:
        if (in1 == 0)
        {
            idle_state = Yes;
        }
        else if (in2 == 0)
        {
            idle_state = No;
            ask_timer = millis();
        }
        break;
    case Yes:
        if (in1 == 1)
        {
            if (battle_or_catch == true)
            {
                overallstate = 5;
                change_display = 1;
            }
            else
            {
                overallstate = 1;
                change_display = 1;
                new_profemon = 1;
            }
            idle_state = Idle;
        }
        break;
    case No:
        change_display = 1;
        if (in2 == 1 && millis() - ask_timer > 5000)
        {
            idle_state = Idle;
        }
        break;
    }
}

// State machine for catch mode
void update_catch_mode(int in1, int in2, int motion)
{
    switch (catch_state)
    {
    case Catch_Idle:
        if (in1 == 0)
        {
            catch_state = Waiting;
            change_display = 1;
        }
        if (in2 == 0)
        {
            catch_state = Quitting;
        }
        break;
    case Quitting:
        if (in2 == 1)
        {
            catch_state = Catch_Idle;
            overallstate = 0;
            // game_state = Map;
            idle_state = Idle;
            change_display = 1;
        }
        break;
    case Waiting:
        if (in1 == 1)
        {
            catch_state = Catch_Idle;
            change_display = 1;
        }
        else if (motion > 0)
        {
            catch_state = Motion;
            state_timer = millis();
            swing_counter = 0;
        }
        break;
    case Motion:
        if (in1 == 1)
        {
            catch_state = Catch_Idle;
            change_display = 1;
        }
        else if (millis() - state_timer > 5000)
        {
            catch_state = Waiting;
        }
        else if (motion == 2)
        {
            catch_state = Throw_End;
        }
        break;
    case Throw_End:
        if (calculate_catch_success() == 1)
        {
            Serial.println("throw successful");
            // Make a HTTP request:
            strcpy(request_buffer, "");
            strcpy(response_buffer, "");
            request_buffer[0] = '\0';  // set 0th byte to null
            response_buffer[0] = '\0'; // set 0th byte to null
            char data[500];
            sprintf(data, "lat_captured=%f&lon_captured=%f&prof_id=%s&user_id=%s&limit=5", latitude, longitude, profemon_name, user);
            Serial.println(data);
            offset = 0; // reset offset variable for sprintf-ing
            offset += sprintf(request_buffer + offset, "POST http://608dev-2.net/sandbox/sc/team3/profedex_editor.py?%s  HTTP/1.1\r\n", data);
            offset += sprintf(request_buffer + offset, "Host: 608dev-2.net\r\n\r\n");
            do_http_request("608dev-2.net", request_buffer, profedex_data, 1000, RESPONSE_TIMEOUT, true);
            Serial.println(request_buffer);
            Serial.println("-----------");
            Serial.println(profedex_data);
            begin_profedex = strchr(profedex_data, '\n') + 1;
            Serial.println("-----------");
            delay(2000);
            change_display = 1;
            catch_state = Catch_Success;
        }
        else
        {
            change_display = 1;
            catch_state = Catch_Fail;
        }
        state_timer = millis();
        break;
    case Catch_Success:
        if (millis() - state_timer > 10000)
        {
            change_display = 1;
            catch_state = Catch_Idle;
            overallstate = 0;
            // game_state = Map;
            idle_state = Idle;
        }
        break;
    case Catch_Fail:
        if (millis() - state_timer > 2000)
        {
            change_display = 1;
            catch_state = Catch_Idle;
        }
        break;
    }
}

// Function to display cool things for catch mode
void catch_display()
{
    switch (catch_state)
    {
    case Catch_Idle:
        if (new_profemon == 1)
        {
            for (int i = 0; i < 4; i++)
            {
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
        if (strlen(profemon_name) != 0 && strlen(display_name) != 0)
        {
            tft.printf("Gotcha!\n%s was caught!\n\n", display_name);
            tft.printf("%s", begin_profedex);
            // UPLOAD PROFEMON TO DATABASE
        }
        if (change_display == 0)
        {
            profemon_name[0] = '\0';
            display_name[0] = '\0';
        }
        break;
    }
}

// Function to detect motion FOR catch mode (not battle)
// 0 is no motion, 1 is swinging, 2 is throw, 3 is invalid motion
int detect_catch_motion(float x_in, float y_in, float z_in)
{
    if (abs(x_in) < 5 && abs(y_in) < 5)
    {
        return 0;
    }
    if (y_in > 10)
    {
        cycle_timer = millis();
        swing_in_progress = true;
        // Serial.println("High");
    }
    else if (x_in < -18 and y_in < -10)
    {
        if (millis() - cycle_timer < 300 && swing_in_progress)
        {
            // Serial.printf("Swing Count: %d", swing);
            swing_in_progress = false;
            return 1;
        }
        else
        {
            return 3;
        }
    }
    if (x_in > 7)
    {
        // Serial.println("Throw");
        if (peak_timer == -1)
        {
            peak_timer = millis();
        }
        else if (millis() - peak_timer > 1000)
        {
            return 2;
        }
    }
    else if (x_in < 3)
    {
        peak_timer = -1;
    }
    return 3;
}

// update acceleration values
void is_moving()
{
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

    if (motion == 1)
    {
        swing_counter++;
        if (swing_counter != 1)
        {
            swing_times[swing_counter] = millis() - swing_timer;
        }
        swing_timer = millis();
    }
    // tft.printf("# Swings: %d    \n", swing_counter);

    // Serial printing:
    char output[100];
    // sprintf(output,"X:%4.2f,Y:%4.2f,Z:%4.2f,Motion:%d",x_avg,y_avg,z_avg,motion); //render numbers with %4.2 float formatting
    // Serial.println(output); //print to serial for plotting
    // sprintf(output, "\n%4.2f  \n%4.2f  \n%4.2f  \nMotion: %d  ", x, y, z, motion); //render numbers with %4.2 float formatting
    // tft.println(output);
}

// calculate catch success
int calculate_catch_success()
{
    float avg_time = 0;
    for (int i = 2; i <= swing_counter; i++)
    {
        // Serial.printf("Time between swing %d and %d: %d millisec\n", i-1, i, swing_times[i]);
        avg_time += swing_times[i];
    }
    avg_time /= (swing_counter - 1);
    // Serial.printf("Avg swing time: %f\n", avg_time);
    float avg_error = 0;
    for (int i = 2; i <= swing_counter; i++)
    {
        //   Serial.println(abs(avg_time-swing_times[i])/(avg_time));
        avg_error += abs(avg_time - swing_times[i]) / (avg_time);
    }
    avg_error /= (swing_counter - 1);
    // Serial.printf("Avg err: %f\n", avg_error);
    float err_mult = avg_error / (swing_counter - 1);
    // Serial.printf("Err w/ mult: %f\n", err_mult);

    if (err_mult > 0.5)
    {
        err_mult = 0.5;
    }

    float probability = 0.1 + (0.5 - err_mult);
    int success_thresh = int(probability * 16000);

    srand(time(NULL));
    int r = rand() % 16000; // random number from 0 to 16000

    if (r <= success_thresh)
    {
        return 1;
    }
    return 0;
}

// Should be called every loop (or at least once every 5-ish seconds)
void update_location_and_profs()
{
    if (millis() - loc_timer > 30000)
    {
        // strcpy(json_body, "");
        // offset = sprintf(json_body, "%s", PREFIX);
        // int n = WiFi.scanNetworks(); //run a new scan. could also modify to use original scan from setup so quicker (though older info)
        // Serial.println("scan done");
        // if (n == 0) {
        // Serial.println("no networks found");
        // } else {
        // int max_aps = max(min(MAX_APS, n), 1);
        // for (int i = 0; i < max_aps; ++i) { //for each valid access point
        //     uint8_t* mac = WiFi.BSSID(i); //get the MAC Address
        //     offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE-offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); //generate the query
        //     if(i!=max_aps-1){
        //     offset +=sprintf(json_body+offset,",");//add comma between entries except trailing.
        //     }
        // }
        // sprintf(json_body + offset, "%s", SUFFIX);
        // Serial.println(json_body);
        // int len = strlen(json_body);
        // // Make a HTTP request:
        // Serial.println("SENDING REQUEST");
        // strcpy(request_buffer, "");
        // strcpy(response_buffer, "");
        // request_buffer[0] = '\0'; //set 0th byte to null
        // response_buffer[0] = '\0'; //set 0th byte to null
        // offset = 0; //reset offset variable for sprintf-ing
        // offset += sprintf(request_buffer + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
        // offset += sprintf(request_buffer + offset, "Host: googleapis.com\r\n");
        // offset += sprintf(request_buffer + offset, "Content-Type: application/json\r\n");
        // offset += sprintf(request_buffer + offset, "cache-control: no-cache\r\n");
        // offset += sprintf(request_buffer + offset, "Content-Length: %d\r\n\r\n", len);
        // offset += sprintf(request_buffer + offset, "%s", json_body);
        // do_https_request("googleapis.com", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
        // Serial.println("-----------");
        // Serial.println(response_buffer);
        // Serial.println("-----------");

        // char* begin = strchr(response_buffer, '{');
        // char* end = strrchr(response_buffer, '}');
        // char djd[500];
        // strncpy(djd, begin, end-begin+1);
        // DeserializationError error = deserializeJson(doc, djd);
        // if (error) {
        //     Serial.print(F("deserializeJson() failed: "));
        //     Serial.println(error.f_str());
        // }
        // latitude = doc["location"]["lat"];
        // longitude = doc["location"]["lng"];

        strcpy(request_buffer, "");
        strcpy(response_buffer, "");
        request_buffer[0] = '\0';  // set 0th byte to null
        response_buffer[0] = '\0'; // set 0th byte to null
        offset = 0;                // reset offset variable for sprintf-ing

        sprintf(request_buffer, "GET /sandbox/sc/team3/nearby_profemon.py?lat=%f&lon=%f HTTP/1.1\r\nHost: 608dev-2.net\r\n\r\n", latitude, longitude);
        do_http_request("608dev-2.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
        Serial.printf("%s, %d", response_buffer, strlen(response_buffer));
        char *comma = strchr(response_buffer, ',');
        if (comma != NULL)
        {
            strncpy(profemon_name, response_buffer, comma - response_buffer);
            strcpy(display_name, comma + 1);
        }
        else if (overallstate == 0)
        { // game_state = Map
            strcpy(profemon_name, "");
            strcpy(display_name, "");
        }

        Serial.println(profemon_name);
        Serial.println(display_name);
        change_display = 1;
    }
    loc_timer = millis();
}

void switch_state(int button)
{
    switch (state)
    {
    case IDLE:
        if (button == 0)
        {
            state = BUFFER;
        }
        break;
    case BUFFER:
        if (button == 1)
        {
            state = VIEWING;
        }
        break;
    case VIEWING:
        if (button == 0)
        {
            state = BUFFER2;
        }
        break;
    case BUFFER2:
        if (button == 1)
        {
            state = IDLE;
        }
        break;
    }
}

void profedex_navigator(int button_forward, int button_backward)
{
    if (button_forward == 0 || button_backward == 0 || firsttime == 1)
    {
        firsttime = 0;

        if (button_forward == 0)
        {
            idx += 1;
        }
        if (button_backward == 0)
        {
            idx -= 1;
        }
        if (idx < 0)
        {
            idx = 0;
        }
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);

        strcpy(request, "");
        request[0] = '\0'; // set 0th byte to null
        offset = 0;        // reset offset variable for sprintf-ing
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
