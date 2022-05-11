void update_state()
{
        switch (game_state)
        {

        case IDLE:
            // Get Building (Also updates location of user on database)
            strcpy(request, "");
            request[0] = '\0'; // set 0th byte to null
            index2 = 0;        // reset offset variable for sprintf-ing
            sprintf(json_body, "user=%s&lat=%f&lon=%f\r\n", user, latitude, longitude);
            Serial.println(json_body);
            len = strlen(json_body);
            index2 += sprintf(request + index2, "POST http://608dev-2.net/sandbox/sc/team3/mil4/new_geolocation.py HTTP/1.1\r\n");
            index2 += sprintf(request + index2, "Host: 608dev-2.net\r\n");
            index2 += sprintf(request + index2, "Content-Type: application/x-www-form-urlencoded\r\n");
            index2 += sprintf(request + index2, "Content-Length: %d\r\n\r\n", len);
            index2 += sprintf(request + index2, "%s", json_body);
            Serial.println(request);
            do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
            Serial.println("-----------");
            Serial.println(response);
            Serial.println("-----------");
            tft.printf("Current Location:\n");
            tft.printf("Lat: %f  \n", latitude);
            tft.printf("Lon: %f  \n", longitude);
            tft.printf("%s    \n", response);

            index3 = 0;
            strcpy(request, "");
            request[0] = '\0';
            strcpy(response, "");
            response[0] = '\0';
            strcpy(nearby, "");
            nearby[0] = '\0';
            sprintf(nearby, "user=%s&lat=%f&lon=%f", user, latitude, longitude);
            len = strlen(nearby);
            index3 += sprintf(request + index3, "POST http://608dev-2.net/sandbox/sc/team3/mil4/new_vicinity.py HTTP/1.1\r\n");
            index3 += sprintf(request + index3, "Host: 608dev-2.net\r\n");
            index3 += sprintf(request + index3, "Content-Type: application/x-www-form-urlencoded\r\n");
            index3 += sprintf(request + index3, "Content-Length: %d\r\n\r\n", len);
            index3 += sprintf(request + index3, "%s", nearby);
            Serial.println(request);
            do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
            Serial.println(response);

            // Check to move to next state
            len = strcmp(response, "none"); // NOTE: ADD \n
            if (strcmp(response, "none") != 0)
            { // there is someone within vicinity
                Serial.printf("here: %d\n", len);

                len = strlen(response);
                response[len - 1] = '\0';
                index4 = 0;
                strcpy(request, "");
                Serial.println("up");
                request[0] = '\0';
                Serial.println("here");
                strcpy(challenge, "");
                challenge[0] = '\0';
                Serial.println("there");

                index4 += sprintf(request + index4, "GET http://608dev-2.net/sandbox/sc/team3/mil4/battling_modes.py?user=%s&opp=%s HTTP/1.1\r\n", user, response);
                index4 += sprintf(request + index4, "Host: 608dev-2.net\r\n\r\n");
                Serial.println(request);
                do_http_request("608dev-2.net", request, challenge, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
                strcpy(request, "");
                Serial.printf("Challenge: %s\n", challenge);

                if (strcmp(challenge, "ready\n") == 0)
                { // battle mode is ready
                    Serial.println("in here");
                    tft.printf("Closest user: %s", response);
                    if (strcmp(response, "none") != 0) {
                      tft.printf("%s is within vicinity! Would you like to challenge them?", response);
                    }
                    // Press button1 for yes and button2 for no
                    battle_timer = millis();
                    while (millis() - battle_timer < 30000 && accept == 0)
                    {
                        button1 = digitalRead(BUTTON1);
                        button2 = digitalRead(BUTTON2);
                        if (button1 == 0)
                        {
                            accept = 1;
                            sprintf(bufferb, "yes");
                        }
                        if (button2 == 0)
                        {
                            accept = 2;
                            sprintf(bufferb, "no");
                        }
                    }
                    index4 = 0;
                    strcpy(request, "");
                    request[0] = '\0';
                    strcpy(yes_no, "");
                    yes_no[0] = '\0';
                    strcpy(response2, "");
                    response2[0] = '\0';
                    Serial.printf("yes or no: %s\n", bufferb);
                    sprintf(yes_no, "user=%s&opp=%s&play=%s", user, response, bufferb);
                    len = strlen(yes_no);
                    index4 += sprintf(request + index4, "POST http://608dev-2.net/sandbox/sc/team3/mil4/battling_modes.py HTTP/1.1\r\n", user, response);
                    index4 += sprintf(request + index4, "Host: 608dev-2.net\r\n");
                    index4 += sprintf(request + index4, "Content-Type: application/x-www-form-urlencoded\r\n");
                    index4 += sprintf(request + index4, "Content-Length: %d\r\n\r\n", len);
                    index4 += sprintf(request + index4, "%s", yes_no);
                    Serial.println(request);
                    do_http_request("608dev-2.net", request, response2, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
                    Serial.println(response2);

                    if (strcmp(response2, "yes\n") == 0)
                    {
                        game_state = READY;
                    }
                }
                else
                {
                    tft.printf("Closest user: %s", response);
                }
            }
            else
            {
                tft.printf("Closest user: %s", response);
            }
            break;

        case READY:
            Serial.println("Made it here");

            if (button1 == 0 || button2 == 0)
            {
                if (button2 == 0)
                {
                    idx += 1;
                }
                if (button1 == 0)
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
                request[0] = '\0';
                strcpy(response, "");
                response[0] = '\0';
                offset = 0;
                offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team3/mil4/new_viewing.py?index=%f&user=%s  HTTP/1.1\r\n", float(idx), user);
                offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
                Serial.println(request);
                do_http_request("608dev-2.net", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
                Serial.println("-----------");
                Serial.println(response);
                Serial.println("-----------");
                tft.println(response);
            }
            if (button3 == 0)
            {
                Serial.println("Here");
                strcpy(request, "");
                request[0] = '\0';
                Serial.println("Here1");
                strcpy(profemon, "");
                profemon[0] = '\0';
                Serial.println("Here2");
                strcpy(bufferb, "");
                bufferb[0] = '\0';
                offset = 0;
                offset += sprintf(request + offset, "GET http://608dev-2.net/sandbox/sc/team3/mil4/new_viewing.py?index=%f&user=%s&select=True  HTTP/1.1\r\n", float(idx), user);
                offset += sprintf(request + offset, "Host: 608dev-2.net\r\n\r\n");
                Serial.println(request);
                do_http_request("608dev-2.net", request, profemon, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
                Serial.println("-----------");
                Serial.println(profemon);
                Serial.println("-----------");
                tft.printf("Selected: %s", profemon);
                game_state = BATTLE;
            }

            break;

        case BATTLE:
            tft.println("Battling");
            battle_timer = millis();
            while (millis() - battle_timer < 10000)
                ;
            game_state = IDLE;
            break;
        }
}

void loop()
{

    update_state();
}