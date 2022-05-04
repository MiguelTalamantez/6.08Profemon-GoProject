void setup() {
  // VARIABLES THAT THE USER SHOULD'VE ALREADY CHOSEN
  strcpy(profemon_id,"Max Goldman");

  // show the loading "battle is just beginning screen"
  tft.printf("You have chosen %s as your Prof-emon!\n",profemon_id);
  tft.printf("Loading...");

  b1 = 1;
  b2 = 1;
  b3 = 1;

  old_b1_input = 1;
  old_b2_input = 1;
  old_b3_input = 1;

  delay(4000);

  // Wifi setup
  client2.setCACert(CA_CERT); //set cert for https
  
  // variable initialization
  num_turns = 0;
  battle_state = 0;

  // send a POST request_buffer labeled "start" with the user id and the prof-emon, send another request_buffer for prof-emon's attack value, send ANOTHER request_buffer for prof-emon's two moves
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
  if(battle_state==0) {
    // waiting for it to be our move. every [time period], we send a GET request_buffer labeled "turn" to ask whose turn it is
    if(millis()-time_since_turn_check > turn_check_period) {
      make_server_request(turn);
    }

    // if it's our turn, the function changed state to 1. we need to update the hp values, the screen (depends on whether the second move can be used yet), and the state value
    if(battle_state==1) {
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
        battle_state = 3;
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
        battle_state = 3;
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
  else if(battle_state==1) {
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
        battle_state = 3;
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
        battle_state = 0;
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
        battle_state = 3;
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
        battle_state = 0;
        time_since_turn_check = millis();
      }
    }
    
    // if the third button is pressed, or they haven't done anything in five minutes, they forfeit the match
    else if((old_b3_input!=b3 && b3==1) || (millis()-time_since_turn_started>max_turn_length)) {
      Serial.println("you have FORFEITED the match :(");
      // how does that work, you may ask? send a POST request_buffer with the game_id and your player_id labeled "forfeit"
      make_server_request(forfeit);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0,0,1);
      tft.printf("You have FORFEITED!\n\n");
      tft.printf("Better luck next time :(\n");
      Serial.printf("state is now 3\n");
      battle_state = 3;
    }
  }
  // i just realized i skipped state 2. that one's an accident -- this should technically be state 2 but since i'm guessing it's gonna be changed in integration anyway i don't care
  else if(battle_state==3) {
    // you lost/you won state. revert to status quo now!
  }

  old_b1_input = b1;
  old_b2_input = b2;
  old_b3_input = b3;
}