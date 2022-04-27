#include <mpu6050_esp32.h> //Used for ESP32 stuff
#include<string.h> //used for some string handling and processing.
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h> //Used in support of TFT Display
#include <WiFi.h> //Connect to WiFi Network
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
MPU6050 imu; //imu object called, appropriately, imu

//Button Setup
const int BUTTON1 = 45; //Button1 at pin 45
const int BUTTON2 = 39; //Button2 at pin 39
const int BUTTON3 = 38; //Button3 at pin 38
const int BUTTON4 = 34; //Button4 at pin 34

//State machine variables
int overallstate = 0;
int mapstate = 0;
int catchstate = 0;

//variables
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

//TFT Location to LCD related variables
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



static const uint8_t  PROGMEM my_map[] = {
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
	0xff, 0xff, 0xff, 0xff, 0xff, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const uint8_t  PROGMEM my_map2[] = {
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
  0x00, 0x00, 0x00, 0x0c, 0x18, 0x00, 0x78, 0x00, 0x1c, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x03, 0xff
};




//uint8_t flipped_map[2336];
uint8_t flipped_map[2560];


void setup() {
    //LCD + SERIAL SETUP
    tft.init();
    tft.setRotation(3);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    for(int i=0;i<2560;i++) {
      flipped_map[i] = my_map[2559-i]; 
    }
    tft.drawXBitmap(0, 0, flipped_map, 160, 128, 0xFFFF);
    Serial.begin(115200);
    Serial.println("Welcome to Prof-emons Go! - Serial View");

    //Button setup
    pinMode(BUTTON1, INPUT_PULLUP); //set input pin as an input button1
    pinMode(BUTTON2, INPUT_PULLUP); //set input pin as an input button2
    pinMode(BUTTON3, INPUT_PULLUP); //set input pin as an input button3
    pinMode(BUTTON4, INPUT_PULLUP); //set input pin as an input button4

  delay(4000);

  // for(int i=0;i<160;i++) {
  //  for(int j=0;j<16;j++) {
  //    flipped_map[i*16+(15-j)] = my_map[i*16+j];
  //  }
  // }

  for(int i=0;i<2560;i++) {
    flipped_map[i] = my_map2[2559-i];
  }

    tft.fillScreen(TFT_BLACK);
    tft.drawXBitmap(0, 0, flipped_map, 160, 128, 0xFFFF);
    userdisplaytimer = millis();


}

void loop() {
  //READ DATA
    uint8_t button1 = digitalRead(BUTTON1);
    uint8_t button2 = digitalRead(BUTTON2); 
    uint8_t button3 = digitalRead(BUTTON3);
    uint8_t button4 = digitalRead(BUTTON4);
    
  //DISPLAY TEXT
    if (displaytext == true){
    Serial.println("Current OverallState:");
    Serial.println(overallstate);
    Serial.println("Current MapState: ");
    Serial.println(mapstate);
    Serial.println("Current CatchState: ");
    Serial.println(catchstate);
    displaytext = false;
    }

  //PROF Detection code
    if (userlocationx > testproflocationx){
      deltax = abs ( (userlocationx) - (testproflocationx) );
    }
    else if (userlocationx < testproflocationx){
      deltax = abs ( (testproflocationx) - (userlocationx) );
    }
    if (userlocationy > testproflocationy){
      deltay = abs ( (userlocationy) - (testproflocationy) );
    }
    else if (userlocationy < testproflocationy){
      deltay = abs ( (testproflocationy) - (userlocationy) );
    }
    rdistance = sqrt(pow(deltax, 2) + pow(deltay, 2));
    // Serial.print("DeltaX: ");
    // Serial.println(deltax, 6);
    // Serial.print("DeltaY: ");    
    // Serial.println(deltay, 6);
    //Serial.print("rDistance: ");
    //Serial.println(rdistance, 6);

    if (rdistance <= 0.0015){
      askifcatch = true;
    }
    

    if (askifcatch == true){
      tft.setCursor(0,0);
      tft.print("DO YOU WANT TO CATCH THIS PROFEMON?");
      catchbuttons = true;
    }
    if (catchbuttons == true)
      if (button1 == 0){
      overallstate = 1;
      displaytext = true;
      }
      if (button2 == 0){
      overallstate = 0;
      displaytext = true;
      }

  //Display User on map
    if (millis() - userdisplaytimer > 2000){
      Serial.println("TIMER RESET");
      userdisplaytimer = millis();
      float percentagex = abs ( (userlocationx-tft_x_min_geolocation)/(tft_x_max_geolocation-tft_x_min_geolocation) );
      float percentagey = abs ( (userlocationy-tft_y_min_geolocation)/(tft_y_max_geolocation-tft_y_min_geolocation) );
      Serial.println(percentagex, 6);
      Serial.println(percentagey, 6);
      
      tft.drawCircle(percentagex*160, 128 - percentagey*128, 3, TFT_BLUE);
      tft.drawCircle(percentagex*160, 128 - percentagey*128, 4, TFT_RED);
      tft.drawCircle(percentagex*160, 128 - percentagey*128, 5, TFT_BLUE);
      tft.drawCircle(percentagex*160, 128 - percentagey*128, 6, TFT_RED);
    }

  overallstatefunction(button1, button2, button3, button4);
  //fakebuttontrigger(button1, button2, button3, button4);


}

void overallstatefunction(uint8_t button1, uint8_t button2, uint8_t button3, uint8_t button4){
  switch(overallstate){
    case(0):
      //Map Mode
      //tft.setCursor(0,100);
      //Serial.println("Displaying map");
      break;
    case(1):
      //Display nice UI graphics
      overallstate = 2;
      displaytext = true;
      Serial.println("State 1");
      Serial.println("State 2");
      break;
    case(2):
      //Catch Mode
      //tft.setCursor(0,100);
      //Serial.println("Catching Pokemon");
      break;
    case(3):
      //Display nice UI graphics
      overallstate = 0;
      displaytext = true;
      Serial.println("State 3");
      Serial.println("State 0");
      break;
    case(4):
      //Display nice UI graphics + Store Prof-emon in Database
      Serial.println("YOU CAUGHT A POKEMON");
      overallstate = 0;
      displaytext = true;
      Serial.println("State 4");
      Serial.println("State 0");
      break;
    case(5):
      //Display nice UI graphics 
      overallstate = 6;
      displaytext = true;
      Serial.println("State 5");
      break;
    case(6):
      //Fight Mode
      //tft.setCursor(0,100);
      //Serial.println("Fighting a Pokemon");
      break;
    case(7):
      //Display nice UI graphics that say YOU LOST
      Serial.println("YOU LOST");
      overallstate = 0;
      displaytext = true;
      Serial.println("State 7");
      break;
    case(8):
      Serial.println("YOU WON");
      overallstate = 0;
      displaytext = true;
      Serial.println("State 8");
      break;
  }
}


void mapstatefunction(uint8_t button1, uint8_t button2, uint8_t button3, uint8_t button4){

  // switch(mapstate){
  //   case(0):
  //     //Idle mode
  //     // if professor is < distance
  //       //mapstate = 1;
  //     break;
  //   case(1):
  //     //Propose catch
  //     break;
  //   case(2):
  //     //change to CATCHING POKEMON overall state
  //     mapstate = 0;
  //     break;
  //   case(3):
  //     //set timer to ask again
  //     mapstate = 0;
  //     break;
  // }

}



void fakebuttontrigger(uint8_t button1, uint8_t button2, uint8_t button3, uint8_t button4){

  if (button1 == 0){
    if (overallstate == 0){
      overallstate = 1;
      displaytext = true;
      delay(100);
    }
  }
  if (button2 == 0){
    if (overallstate == 2){
      overallstate = 4;
      displaytext = true;
      delay(100);
    }
  }
  if (button3 == 0){
    if (overallstate == 2){
      overallstate = 3;
      displaytext = true;
      delay(100);
    }
  }
  if (button2 == 0){
    if (overallstate == 0){
      overallstate = 5;
      displaytext = true;
      delay(100);
    }
  }
  if (button3 == 0){
    if (overallstate == 6){
      overallstate = 8;
      displaytext = true;
      delay(100);
    }
  }
  if (button4 == 0){
    if (overallstate == 6){
      overallstate = 7;
      displaytext = true;
      delay(100);
    }
  }

}


