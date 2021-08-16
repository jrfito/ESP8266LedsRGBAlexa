#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"

// Rename the credentials.sample.h file to credentials.h and 
// edit it according to your router configuration


fauxmoESP fauxmo;

// -----------------------------------------------------------------------------

#define WIFI_SSID "SANDO_2.4"
#define WIFI_PASS "Tgbed259A"
#define SERIAL_BAUDRATE     115200

#define LED_RED             5
#define LED_GREEN           16
#define LED_BLUE            4
#define LED_PINK            2
#define LED_WHITE           15

#define ID_RED              "TV luz roja"
#define ID_GREEN            "TV luz verde"
#define ID_BLUE             "TV luz Azul"
#define ID_PINK             "TV luz rosa"
#define ID_WHITE            "TV luz blanca"

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------

void wifiSetup() {

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);

    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();

    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}

void setup() {

    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();

    // LEDs
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(LED_PINK, OUTPUT);
    pinMode(LED_WHITE, OUTPUT);
    analogWrite(LED_RED, 0);
    analogWrite(LED_GREEN, 0);
    analogWrite(LED_BLUE, 0);
    analogWrite(LED_PINK, 0);
    analogWrite(LED_WHITE, 0);

    // Wifi
    wifiSetup();

    // By default, fauxmoESP creates it's own webserver on the defined port
    // The TCP port must be 80 for gen3 devices (default is 1901)
    // This has to be done before the call to enable()
    fauxmo.createServer(true); // not needed, this is the default value
    fauxmo.setPort(80); // This is required for gen3 devices

    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo.enable(true);

    // You can use different ways to invoke alexa to modify the devices state:
    // "Alexa, turn yellow lamp on"
    // "Alexa, turn on yellow lamp
    // "Alexa, set yellow lamp to fifty" (50 means 50% of brightness, note, this example does not use this functionality)

    // Add virtual devices
    fauxmo.addDevice(ID_RED);
    fauxmo.addDevice(ID_GREEN);
    fauxmo.addDevice(ID_BLUE);
    fauxmo.addDevice(ID_PINK);
    fauxmo.addDevice(ID_WHITE);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        
        // Callback when a command from Alexa is received. 
        // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
        // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
        // Just remember not to delay too much here, this is a callback, exit as soon as possible.
        // If you have to do something more involved here set a flag and process it in your main loop.
        
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
        // Otherwise comparing the device_name is safer.

        if (strcmp(device_name, ID_RED)==0) {
            analogWrite(LED_RED, state ? value : 0);
            analogWrite(LED_GREEN, 0);
            analogWrite(LED_BLUE, 0);
        } else if (strcmp(device_name, ID_GREEN)==0) {
            analogWrite(LED_GREEN, state ? value : 0);
            analogWrite(LED_RED, 0);
            analogWrite(LED_BLUE, 0);
        } else if (strcmp(device_name, ID_BLUE)==0) {
            analogWrite(LED_GREEN,state ? value * .56 : 0 );
            analogWrite(LED_RED, 0);
            analogWrite(LED_BLUE, state ? value * .61 : 0);
        } else if (strcmp(device_name, ID_PINK)==0) {
            analogWrite(LED_GREEN, state ? 0 : 0 );
            analogWrite(LED_RED,  state ? value * .71 : 0);
            analogWrite(LED_BLUE, state ? value * .59 : 0);
        } else if (strcmp(device_name, ID_WHITE)==0) {
            analogWrite(LED_GREEN, state ? value : 0 );
            analogWrite(LED_RED,  state ? value : 0);
            analogWrite(LED_BLUE,  state ? value : 0);
        }

    });

}

void loop() {

    // fauxmoESP uses an async TCP server but a sync UDP server
    // Therefore, we have to manually poll for UDP packets
    fauxmo.handle();

    // This is a sample code to output free heap every 5 seconds
    // This is a cheap way to detect memory leaks
    static unsigned long last = millis();
    if (millis() - last > 5000) {
        last = millis();
        Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
    }

    // If your device state is changed by any other means (MQTT, physical button,...)
    // you can instruct the library to report the new state to Alexa on next request:
    // fauxmo.setState(ID_YELLOW, true, 255);

}
