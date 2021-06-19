#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

BlynkTimer timer;
#define dhtpin    D5
#define soilpin   A0

int temperature;
int humidity;
int temp;
float moisture;
int relay = D2;
AsyncWebServer OTAserver(80);

WiFiClient client;

DHT dht(D5, DHT11);

char server[] = "         "; //paste the ip after running "ping blynk-cloud.com"
char auth[] = "******************"; //Blynk Auth Token
const char* ssid = "***********";  // Enter Wi-fi ID
const char* password = "*********!"; //Enter WiFi password
const char* TSserver = "api.thingspeak.com";          //Thingspeak Server 
String apiKey = "*********";    //Thingspeak apikey

void sendsensor()
{
  //Reading the values
  int temp = analogRead(A0);
  float moisture = map(temp,860,460,0,100);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  //Writing the read values to blynk server
  Blynk.virtualWrite(V6,t);
  Blynk.virtualWrite(V7,h);
  Blynk.virtualWrite(V5,moisture);

  //Writring the values of to Thingspeak Cloud Server
  if (client.connect(TSserver, 80))                                 //api.thingspeak.com
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(dht.readTemperature());
    postStr += "&field2=";
    postStr += String(dht.readHumidity());
    postStr += "&field3=";
    postStr += String(moisture);

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
  }
  client.stop();
  delay(1000);

  //Condition for the water to be pumped
  if(moisture<85)
  {
    digitalWrite(relay,HIGH);
  }
  else
  {
    digitalWrite(relay,LOW);
  }
}

void setup() {
  
  dht.begin();        //initializing DHT temp and humidity sensor
  DHT dht(D5, DHT11);
  
  Blynk.begin(auth, ssid, password, server); //Starting Blynk server
  
  Serial.begin(115200);
  
  pinMode(A0,INPUT);  //Soil Moisture sensor
  pinMode(relay, OUTPUT);  // Relay module
  delay(10);

  //Connecting to wifi
//  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  OTAserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP8266.");
  });

  AsyncElegantOTA.begin(&OTAserver);    // Start ElegantOTA
  OTAserver.begin();
  Serial.println("HTTP server started");
  timer.setInterval(1000L, sendsensor);
}

void loop() {
 Blynk.run();
 timer.run();
 AsyncElegantOTA.loop();
}
