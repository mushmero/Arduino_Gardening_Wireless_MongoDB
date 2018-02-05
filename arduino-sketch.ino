#include <PubSubClient.h>						//MQTT library
#include <WiFiEsp.h>							//ESP8266 WiFi library
#include <WiFiEspClient.h>
#include <WiFiEspUdp.h>
#include <SoftwareSerial.h>						//Serial Communication library
#include <Wire.h>
#include <SeeedOLED.h>							//SeeedStudio OLED library
#include "DHT.h"								//DHT sensor library
#include "SI114X.h"

#define DHTPIN    A0						
#define DHTTYPE   DHT11								//define sensor type eg: DHT11, DHT22, DHT23
#define ssid "SSID"				//wifi ssid
#define pass "PASSWORD"							//wifi password
#define mqtt_server "HOST"				//mqtt broker server hostname or ip address
#define mqtt_port 17528								//mqtt broker port
#define mqtt_user "USERNAME"							//mqtt broker username
#define mqtt_pass "PASSWORD"							//mqtt broker password

SI114X sensor = SI114X();
DHT dht(DHTPIN, DHTTYPE);

String clientName = String("arduino");						//unique client name for mqtt broker
String topicName = String("/monitoring/");					//topic name to publish and subscribe later

char tt[15];
char hh[15];
  
float h = 0;
float t = 0;
float vs = 0;
float ir = 0;
float uv = 0;

const int rxpin = 2;										//arduino pin to connect from esp8266 txpin
const int txpin = 3;										//arduino pin to connect from esp8266 rxpin
int status = WL_IDLE_STATUS;
SoftwareSerial ss(rxpin,txpin);
WiFiEspClient espClient;
PubSubClient client(mqtt_server,mqtt_port,espClient);

void setup()
{
  Wire.begin();
  SeeedOled.init();
  DDRB|=0x21;
  PORTB|=0x21;
  SeeedOled.clearDisplay();
  SeeedOled.setNormalDisplay();
  SeeedOled.setPageMode();
  SeeedOled.setTextXY(3,0);
  SeeedOled.putString("Starting up....");

  Serial.begin(9600);												//serial monitor baud rate
  ss.begin(9600);													//esp8266 baud rate
  WiFi.init(&ss);

  if(WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield is unavailable!"));
    SeeedOled.clearDisplay();
    SeeedOled.setTextXY(1,2);
    SeeedOled.putString("WiFi shield ");
    SeeedOled.setTextXY(3,1);
    SeeedOled.putString("is unavailable!");    
    while (true);
  }

  while(status != WL_CONNECTED){
    Serial.print(F("Attempting to connect SSID: "));
    Serial.println(ssid);
    SeeedOled.clearDisplay();
    SeeedOled.setTextXY(1,1);
    SeeedOled.putString("Attempting to ");
    SeeedOled.setTextXY(2,1);
    SeeedOled.putString("connect SSID: ");
    SeeedOled.setTextXY(4,0);
    SeeedOled.putString(ssid);

    status = WiFi.begin(ssid,pass);
  }
  Serial.println(F("Connected to network"));
  SeeedOled.clearDisplay();
  SeeedOled.setTextXY(3,0);
  SeeedOled.putString("Connected!");  
  dht.begin();
  while(!sensor.Begin()) {
    delay(1000);
  }
}

//get UV, visibility and infrared value from sunlight sensor
void getUV(){
  SeeedOled.clearDisplay();
  SeeedOled.setTextXY(2,0);
  SeeedOled.putString("Reading from ");
  SeeedOled.setTextXY(3,0);
  SeeedOled.putString("UV sensor");

   vs = sensor.ReadVisible();
   ir = sensor.ReadIR();
   uv = (float)sensor.ReadUV()/100 + 0.5;
  
  Serial.print(F("Visibility: "));
  Serial.println(vs);
  Serial.print(F("Infrared: "));
  Serial.println(ir);
  Serial.print(F("UV: "));
  Serial.println(uv);
  return;
}

//get humidity and temperature value from DHT11 sensor
void getTempHumidity(){
  SeeedOled.clearDisplay();
  SeeedOled.setTextXY(2,0);
  SeeedOled.putString("Reading from ");
  SeeedOled.setTextXY(3,0);
  SeeedOled.putString("DHT sensor");
  
  h = dht.readHumidity();
  t = dht.readTemperature();

  sprintf(hh,"%d.%d%%",(int)(h),(int)((int)(h*100) % 100));
  sprintf(tt,"%d.%d*C",(int)(t),(int)((int)(t*100)%100));

  if(isnan(h) || isnan(t)){
    Serial.println("Failed to read from DHT");
    SeeedOled.clearDisplay();
    SeeedOled.setTextXY(1,0);
    SeeedOled.putString("Failed to read");
    SeeedOled.setTextXY(2,0);
    SeeedOled.putString("from DHT sensor");
  } else {
    Serial.print(F("Humidity: "));
    Serial.println(hh);
    Serial.print(F("Temperature: "));
    Serial.println(tt);
  }
  return;
}

//setup json like string
String buildJSON(){
  String data = "{";
  data+="\n";
  data+="\"UltraViolet\": ";
  data+=(float)uv;
  data+=",";
  data+="\n";
  data+="\"Humidity\": ";
  data+=(int)h;
  data+=",";
  data+="\n";
  data+="\"Temperature\": ";
  data+=(int)t;
  data+=",";
  data+="\n";
  data+="\"Timestamp\": ";
  data+=time();
  data+="\n";
  data+="}";
  return data;
}
 
void loop() {
  char clientStr[30];
  char topicStr[30];
  clientName.toCharArray(clientStr,30);
  topicName.toCharArray(topicStr,30);
  delay(1000);
  getUV();
  delay(1000);
  getTempHumidity();
  delay(1000);

  if(!client.connected()){
    client.connect(clientStr,mqtt_user,mqtt_pass);
  }
  if(client.connected()){
    Serial.println(F("MQTT connected"));
    String aa = buildJSON();
    char jsonStr[100];
    aa.toCharArray(jsonStr,aa.length()+1);

    Serial.println(jsonStr);

    SeeedOled.clearDisplay();
    SeeedOled.setTextXY(3,0);
    SeeedOled.putString("Sending to ");
    SeeedOled.setTextXY(4,0);
    SeeedOled.putString("MQTT server");

    client.publish(topicStr,jsonStr);
    delay(1000);

    Serial.println(F("Sent!"));
    SeeedOled.clearDisplay();
    SeeedOled.setTextXY(3,0);
    SeeedOled.putString("Sent! ");
  }
  delay(900000);								//delay 15min
}
