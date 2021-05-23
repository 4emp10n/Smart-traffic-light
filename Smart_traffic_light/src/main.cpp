// Dziuba Aleksandr Variant - 1
// Form the functioning of the TLC, which when flickering
// a green light for a pedestrian provides an audible signal
// squeaks (passive buzzer). The mode is connected by
// by external control
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "373";
const char* password =  "2899117fifc";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

//light for car 
int greenCar = D1;
int redCar = D2; 
int yellowCar = D3; 

//light for pedestrian 
int greenPed = D4; 
int redPed= D7; 
//buzer pin
int buzzer = D5;
//pedestrian control 
int button = D6; 
// 

const int crossTime=5000; 
unsigned long changeTime; 
bool but_state = 0;
int sys_state = 0;
void changeLights();
void MQTTcallback(char* topic, byte* payload, unsigned int length) //function is called to check whether any updated information is available or not.
{
  Serial.print("Message received in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String message;
  for (int i = 0; i < length; i++) 
  {
    message = message + (char)payload[i];//convert the messages into a string
  }
  Serial.print(message);

  if (message == "1" && sys_state == 0) {sys_state = 1;} //checked for any triggering actions
  else if(message == "1" && sys_state == 1) {sys_state = 0;}
  Serial.println();
  Serial.println("-----------------------");
}

void setup() 
{
  pinMode(redCar, OUTPUT); 
  pinMode(yellowCar, OUTPUT); 
  pinMode(greenCar, OUTPUT); 
  pinMode(redPed, OUTPUT); 
  pinMode(greenPed, OUTPUT); 
  pinMode(button, INPUT); 
  //turn on the green light 
  digitalWrite(greenCar, HIGH); 
  digitalWrite(redPed, HIGH); 
  digitalWrite(redCar, LOW); 
  digitalWrite(yellowCar, LOW); 
  digitalWrite(greenPed, LOW); 
  changeTime = millis(); 
  
  Serial.begin(9600);
  WiFi.begin(ssid, password); //connect the ESP to your preferred HotSpot.
  while (WiFi.status() != WL_CONNECTED) //check for a successful network connection
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.print("Connected to WiFi :");
  Serial.println(WiFi.SSID());
  client.setServer(mqtt_server, mqtt_port); // create a broker
  client.setCallback(MQTTcallback);//to receive messages from the server, we need to create
  while (!client.connected()) 
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266")) //connect to the ESP8266 client
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  client.subscribe("esp/test"); //subscribe to a particular topic.
}

void loop() 
{
  //if the green light is on for more than 2 seconds and the pedestrian pressed the button,
  //then turn on the green one for the pedestrian
  if(digitalRead(D6)==1 && millis() - changeTime >= 2000) 
  {
    but_state = 1;
    Serial.print("state == 1");
  }
  else if (millis() - changeTime >= 2000) client.loop();
  
  if (but_state == HIGH || (millis() - changeTime) > crossTime)  changeLights();
  
}

void changeLights() 
{ 
  digitalWrite(greenCar, LOW);    // the green LED will turn off 
  digitalWrite(yellowCar, HIGH); // the yellow LED will turn on for 2 second 
  digitalWrite(redCar, LOW); 
  digitalWrite(redPed, HIGH); 
  digitalWrite(greenPed, LOW); 
  delay(2000);  
   
  digitalWrite(yellowCar, LOW); // the yellow LED will turn off 
  digitalWrite(redCar, HIGH); // the red LED will turn on for 5 seconds 
  digitalWrite(redPed, LOW); 
  digitalWrite(greenPed, HIGH); 
  delay(crossTime); 
   
  //Turn on buzzer
  if(sys_state == 1) tone(buzzer, 1000, 500);
  // flash the ped green 
  for (int x=0; x<10; x++) 
  { 
    digitalWrite(greenPed, LOW); 
    delay(100); 
    
    digitalWrite(greenPed, HIGH); 
    delay(100); 
  } 

  //Turn off buzzer
  if (sys_state == 1) noTone(buzzer);

  digitalWrite(greenPed, LOW); 
  digitalWrite(redCar, LOW); 
  digitalWrite(redPed, HIGH); 
  digitalWrite(greenCar, HIGH); 
   
  but_state = 0;
  changeTime = millis(); 
 }

