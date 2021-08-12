#include <ESP8266WiFi.h>                       //library for wifi features
#include <DHT.h>                               //library for DHT sensor
#include <PubSubClient.h>                      //library for mqtt pubsub


String apiKey = "NX3T0WSEFIB2CMCU";           //   API key from ThingSpeak
 
const char *ssid =  "OnePlus 7T";             //wifi ssid 
const char *pass =  "password";               //wifi password
const char* server = "api.thingspeak.com";    //webserver where the sensor value will be sent
 
#define DHTPIN 0                              //pin where the dht11 is connected
 
DHT dht(DHTPIN, DHT11);
 
WiFiClient client;

const char* mqtt_server = "broker.mqttdashboard.com";   //mqtt broker
const int mqtt_port = 1883;                             //mqtt server port
PubSubClient mqttclient(client);                        //calling the PubSubClient method and passing client as parameter
 
void setup() 
{
       Serial.begin(115200);                      //baud rate
       delay(10);
       dht.begin();                               //activating the sensor
 
       Serial.println("Connecting to ");
       Serial.println(ssid);
 
 
       WiFi.begin(ssid, pass);
 
      while (WiFi.status() != WL_CONNECTED)        //connecting to the wifi
     {
            delay(500);
            Serial.print(".");
     }
      Serial.println("");
      Serial.println("WiFi connected");


      mqttclient.setServer(mqtt_server, mqtt_port);     
    while (!mqttclient.connected())                 //Connecting to the mqtt broker
    {
      Serial.println("Connecting to MQTT...");
      if (mqttclient.connect("ESP8266"))
      {
        Serial.println("Connected to MQTT");
      }
      else
      {
       Serial.print("failed with state ");
       Serial.println(mqttclient.state());
       delay(2000);
      }
  }
 
}
 

 
void loop() 
{
      float h = dht.readHumidity();             //reading the temperature
      float t = dht.readTemperature();          //reading the humidity


      
  

      
              if (isnan(h) || isnan(t))         // checking whether a value is an illegal number
                 {
                     Serial.println("Failed to read from DHT sensor!");
                      return;
                 }
 
                         if (client.connect(server,80))   //checking if connected to  "184.106.153.149" or api.thingspeak.com
                      {  
                            
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(t);
                             postStr +="&field2=";
                             postStr += String(h);
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");               //printing on web serever
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);

 
 
                             Serial.print("Temperature: ");                 //printing on IDE console
                             Serial.print(t);
                             Serial.print(" degrees Celcius, Humidity: ");
                             Serial.println(h);
                             
                        }
          client.stop();
          

          if (mqttclient.connect("ESP8266"))
          {
            char  buf_t[50];
            sprintf(buf_t,"%f",t);                          //converting the float value temp to string
            mqttclient.publish("mqtt/temp",buf_t);          //publishing the temperature to mqtt subscribers

            char  buf_h[50];
            sprintf(buf_h,"%f",h);                          //converting the float value temp to string
            mqttclient.publish("mqtt/hum",buf_h);           //publishing the humidity to mqtt subscribers

            mqttclient.loop();
            delay(1000);
          }
          Serial.println("Waiting...");
  
  // thingspeak needs minimum 15 sec delay between updates
  delay(1000);

}
