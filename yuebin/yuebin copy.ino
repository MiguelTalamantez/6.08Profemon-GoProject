#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <string.h>

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

// TFT Setup
TFT_eSPI tft = TFT_eSPI();
const int tft_x_max_LCD = 160;
const int ftf_y_max_LCD = 128;
const int LOOP_PERIOD = 40;

// HTTP Setup
const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 3500;  // size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; // size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 3000;
// Json bodies for lat/lon
char request[IN_BUFFER_SIZE];   // char array buffer to hold HTTP request
char response[OUT_BUFFER_SIZE]; // char array buffer to hold HTTP response
char response2[OUT_BUFFER_SIZE];
char json_body[JSON_BODY_SIZE];
char nearby[1000];
char challenge[100];
char yes_no[100];
char bufferb[5];
char profemon[500];
// Stuff for Request
const char PREFIX[] = "{\"wifiAccessPoints\": ["; // beginning of json body
const char SUFFIX[] = "]}";                       // suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8";
// offsets and requests
int offset; // for geolocation
int index1; // offset for POST request Google API
int index2; // offset for POST request get building
int index3; // offset for POST request get nearby players
int index4;
int len; // body lengths
const int MAX_APS = 5;
char SERVER[] = "608dev-2.net";
// HARD-CODED USER NAME
char user[] = "Pikachu";

double latitude = 0.0;
double longitude = 0.0;

// State Machine Constants
const uint8_t BUTTON1 = 45;
const uint8_t BUTTON2 = 39;
const uint8_t BUTTON3 = 38;
uint8_t state1;
uint8_t old_state1;
uint8_t state2;
uint8_t old_state2;
uint8_t state3;
uint8_t old_state3;
uint8_t game_state;
int accept = 0;
int timer;
const uint8_t IDLE = 0;
const uint8_t READY = 1;
const uint8_t BATTLE = 2;
int idx = 0;

uint8_t button1;
uint8_t button2;
uint8_t button3;

// WiFi Setup
WiFiClientSecure client; // global WiFiClient Secure object
WiFiClient client2;      // global WiFiClient Secure object
StaticJsonDocument<500> doc;

const char NETWORK[] = "MIT";
const char PASSWORD[] = "";
uint8_t channel = 1;                                 // network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; // 6 byte MAC address of AP you're targeting.

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

void setup()
{
    Serial.begin(115200); // Set up serial port

    // SET UP SCREEN:
    tft.init();                            // init screen
    tft.setRotation(1);                    // adjust rotation
    tft.setTextSize(1);                    // default font size, change if you want
    tft.fillScreen(TFT_BLACK);             // fill background
    tft.setTextColor(TFT_PINK, TFT_BLACK); // set color of font to hot pink foreground, black background

    // SET UP BUTTON:
    delay(100); // wait a bit (100 ms)
    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUTTON3, INPUT_PULLUP);

    // PRINT OUT WIFI NETWORKS NEARBY
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
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
    // WiFi.begin(network, password, channel, bssid);

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
    // timer = millis();
}

// Helper Functions
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
        int try_counter = 0;
        while (strncmp(response, "There was an error:<pre>[Errno 24]", 34) == 0)
        {
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
            try_counter++;
            if (try_counter > 5)
            {
                break;
            }
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
        int try_counter = 0;
        while (strncmp(response, "There was an error:<pre>[Errno 24]", 34) == 0)
        {
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
            try_counter++;
            if (try_counter > 5)
            {
                break;
            }
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

void update_state()
{

    if (game_state == IDLE)
    {
        offset = sprintf(json_body, "%s", PREFIX);
        int n = WiFi.scanNetworks(); // run a new scan. could also modify to use original scan from setup so quicker (though older info)
        Serial.println("scan done");
        if (n == 0)
        {
            Serial.println("no networks found");
        }
        else
        {
            tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0);
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
            len = strlen(json_body);
            // Send Post Request to Receive Own Location
            Serial.println("SENDING REQUEST");
            request[0] = '\0'; // set 0th byte to null
            index1 = 0;        // reset offset variable for sprintf-ing
            index1 += sprintf(request + index1, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
            index1 += sprintf(request + index1, "Host: googleapis.com\r\n");
            index1 += sprintf(request + index1, "Content-Type: application/json\r\n");
            index1 += sprintf(request + index1, "cache-control: no-cache\r\n");
            index1 += sprintf(request + index1, "Content-Length: %d\r\n\r\n", len);
            index1 += sprintf(request + index1, "%s", json_body);
            do_https_request("googleapis.com", request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
            Serial.println("-----------");
            Serial.println(response);
            Serial.println("-----------");
            // Deserialize Location (Json)
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
            latitude = doc["location"]["lat"];
            longitude = doc["location"]["lng"];
        }
    }

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
                do_http_request(SERVER, request, challenge, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
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
                    timer = millis();
                    while (millis() - timer < 30000 && accept == 0)
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
                do_http_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
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
                do_http_request(SERVER, request, profemon, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
                Serial.println("-----------");
                Serial.println(profemon);
                Serial.println("-----------");
                tft.printf("Selected: %s", profemon);
                game_state = BATTLE;
            }

            break;

        case BATTLE:
            tft.println("Battling");
            timer = millis();
            while (millis() - timer < 10000)
                ;
            game_state = IDLE;
            // index1 = 0;
            // strcpy(response, "");
            // response[0] = '\0';
            // strcpy(request, "");
            // request[0] = '\0';
            // strcpy(bufferb, "");
            // bufferb[0] = '\0';
            // sprintf(bufferb, )
            break;
        }
}

void loop()
{
    button1 = digitalRead(BUTTON1);
    button2 = digitalRead(BUTTON2);
    button3 = digitalRead(BUTTON3);

    update_state();
}