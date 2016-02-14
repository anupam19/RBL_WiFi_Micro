#include <WiFi.h>
#include "DHT.h"

#define DHTTYPE DHT11 // DHT 11
#define DHTPIN 15 // what digital pin we're connected to

#define XauthToken "YOUR X AUTH TOKEN" // X-Auth-Token
#define HUMI_ID_VARIABLE "YOUR HUMIDITY ID VARIABLE"
#define TEMP_ID_VARIABLE "YOUR TEMPERATURE ID VARIABLE"
#define RSSI_ID_VARIABLE "YOUR RSSI ID VARIABLE"

#define SERVER "things.ubidots.com"
#define PORT 80
#define PATH_URL "/api/v1.6/collections/values"
#define METHOD "POST"


char *locationinfo;
char* ptr;
char loc_content[300];
char req_header[300];
char accpt_payload[300];

DHT dht(DHTPIN, DHTTYPE);

// your network name also called SSID
char ssid[] = "YOURSSID";
// your network password
char password[] = "PASSWORD";


// initialize the library instance:
WiFiClient client;

unsigned long lastConnectionTime = 0; // last time you connected to the server, in milliseconds
boolean lastConnected = false; // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000; //delay between updates to ubidots.com
char sensor_payload[256];

long rssi_new= 0;
long rssi_curr= 0;

void setup() {
    Serial.begin(115200);
    wifi_conn ();
    dht.begin();
}


void loop() {

    /*debug for incoming serial(if any)*/
    while (client.available()) {
        char c = client.read();
        Serial.print(c);
    }

    if (WiFi.status() == WL_CONNECTED) {

        /*if you're not connected, and ten seconds have passed since your last connection, then connect again and send data*/
        if (!client.connected() && (millis() - lastConnectionTime > postingInterval)) {

            /*reading humidity sensor data*/
            int h = dht.readHumidity();
            /*reading temperature sensor data*/
            int t = dht.readTemperature();
            /*optional*/
            int f = dht.readTemperature(true);

            // Check if any reads failed and exit early (to try again).
            if (isnan(h) || isnan(t) || isnan(f)) {
                Serial.println("Failed to read from DHT sensor!");
                return;
            }

            //RSSI
            rssi_new = WiFi.RSSI();
            if (rssi_new!=0) {
                rssi_curr = rssi_new;
            }

            memset(sensor_payload , '\0', strlen(sensor_payload));
            sprintf(sensor_payload, "[{ \"variable\": \"%s\", \"value\": %d }, { \"variable\": \"%s\", \"value\": %d }, { \"variable\": \"%s\", \"value\": %ld }]", TEMP_ID_VARIABLE, t, HUMI_ID_VARIABLE, h, RSSI_ID_VARIABLE, rssi_curr);
            Serial.print("Payload : ");
            Serial.println(sensor_payload);
            Serial.println(strlen(sensor_payload));

            /*Sending payload to ubidots*/
            send_ubidots(sensor_payload);
        }
        
    }
    else {
        wifi_conn ();
    }

    lastConnected = client.connected();

}

void send_ubidots(char* sensor_payload) {
    if (client.connect(SERVER, PORT)) {
        Serial.println("connected ubidots");
        delay(100);
        client.print(METHOD);
        client.print(" ");
        client.print(PATH_URL);
        client.println(" HTTP/1.1");
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(strlen(sensor_payload));
        client.print("X-Auth-Token: ");
        client.println(XauthToken);
        client.print("Host: ");
        client.println(SERVER);
        client.println();
        client.print(sensor_payload);
    }
    else {
        // if you didn't get a connection to the server:
        Serial.println("ubidots connection failed");
    }

    if (client.available()) {
        char c = client.read();
        Serial.print(c);
    }
    lastConnectionTime = millis();
}

void wifi_conn () {
    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED) {
        // print dots while we wait to connect
        Serial.print(".");
        delay(300);
    }
  
    Serial.println("\nYou're connected to the network");
    Serial.println("Waiting for an ip address");
  
    while (WiFi.localIP() == INADDR_NONE) {
        // print dots while we wait for an ip addresss
        Serial.print(".");
        delay(300);
    }

    Serial.println("\nIP Address obtained");
    printWifiStatus();

}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
