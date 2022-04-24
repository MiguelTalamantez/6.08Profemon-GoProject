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

void setup() {
    //LCD + SERIAL SETUP
    tft.init();
    tft.setRotation(2);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0,1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Insert nice animation here");
    tft.println("");
    tft.println("WELCOME TO PROF-EMON GO!");

    Serial.begin(115200);
    Serial.println("Welcome to Prof-emons Go! - Serial View");

    //Button setup
    pinMode(BUTTON1, INPUT_PULLUP); //set input pin as an input button1
    pinMode(BUTTON2, INPUT_PULLUP); //set input pin as an input button2
    pinMode(BUTTON3, INPUT_PULLUP); //set input pin as an input button3
    pinMode(BUTTON4, INPUT_PULLUP); //set input pin as an input button4

  delay(5000);
  tft.fillScreen(TFT_BLACK);  
  tft.setCursor(0,0,1);


}

void loop() {
    //READ DATA
    uint8_t button1 = digitalRead(BUTTON1);
    uint8_t button2 = digitalRead(BUTTON2); 
    uint8_t button3 = digitalRead(BUTTON3);
    uint8_t button4 = digitalRead(BUTTON4);
    
    //DISPLAY TEXT
    if (displaytext == true){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,1);
    tft.print("Current OverallState:");
    tft.print(overallstate);
    tft.setCursor(0,25);
    tft.println("Current MapState: ");
    tft.print(mapstate);
    tft.setCursor(0,50);
    tft.println("Current CatchState: ");
    tft.print(catchstate);
    displaytext = false;
    }


  overallstatefunction(button1, button2, button3, button4);
  fakebuttontrigger(button1, button2, button3, button4);


}

void overallstatefunction(uint8_t button1, uint8_t button2, uint8_t button3, uint8_t button4){
  switch(overallstate){
    case(0):
      //Map Mode
      tft.setCursor(0,100);
      tft.println("Displaying map");
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
      tft.setCursor(0,100);
      tft.println("Catching Pokemon");
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
      tft.print("YOU CAUGHT A POKEMON");
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
      tft.setCursor(0,100);
      tft.println("Fighting a Pokemon");
      break;
    case(7):
      //Display nice UI graphics that say YOU LOST
      tft.print("YOU LOST");
      overallstate = 0;
      displaytext = true;
      Serial.println("State 7");
      break;
    case(8):
      tft.print("YOU WON");
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




