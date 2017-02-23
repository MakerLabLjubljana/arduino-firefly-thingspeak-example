/*
 * An example sketch featuring the process of reading data from a FireFly(www.firefly-iot.com)
 * sensor board and sending it to a ThingSpeak channel.
 * 
 * This example was created by Anže Kožar(@nzkozar) for Makerlab Ljubljana(www.maker.si)
 * and adapted from code by L-Tek Elektronika(http://www.l-tek.si)
 * 
 * This example was tested on Arduino Uno and Arduino Mega ADK using a Ethernet Shield and an XBee shield.
 */
#include <ArduinoJson.h>
#include "FireFly.h"
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
EthernetClient client; 

// ThingSpeak constants
const String writeAPIKey = "TS54AH7N8U8ECH2L"; //Your channel's write key

int SerialMessageLength = 200;
char data[200] = {}; //serial buffer
int index = 0;
bool cloudSend = false;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;


void tsUpdate(String data){
  client.stop(); //Stop a previously started client to prevent any hickups in communication
  if (client.connect("api.thingspeak.com", 80)){
    Serial.println("connected");
    
    // Make a HTTP request:
    client.println("GET /update?api_key="+writeAPIKey+"&"+data+" HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println();
    client.flush();
    Serial.println("Data sent!");
  }else{
    Serial.println("Connection failed");  
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Serial ready!");
  
  Ethernet.begin(mac);
  Serial.println("Ethernet connected!");
  
  FFSetSensors("909", 1, 1, 1, 1, 1, 1);
  FFContinuousResponse("909", "3", "15");
}

void loop() {
  //Print server response
  while(client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  //Read sensor response
  while(Serial.available() > 0){
    //reading serial from FF1502 BLE module
    if(index < SerialMessageLength){
      data[index] = Serial.read();
      //Serial.print(data[index]);
      if(index == 0){
        if(data[index] == '!'){
          data[index] = ' ';
          index++;
        }
      }else{
        if(data[index] == '?'){ //Response message end
          data[index] = ' \0';
          index = 0;
          cloudSend = true;
        }else if(data[index] == '!'){ //Response message start
          data[0] == ' ';
          index=1;
        }else{
          index++;
        }
      }
    }else{
      index = 0;
    }
    if(cloudSend == true){
      cloudSend = false;
      //Serial.print(data);
      //Serial.print("\r");
      
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(data);

      if (!root.success()){
        Serial.println("parseObject() failed");
        return;
      }
      //here you extract parsed json data like you would from a table
      int lux = root["d"]["Lum"];
      float temp = root["d"]["Temp"];
      float hum = root["d"]["RelHum"];
      /*
      const char* id = root["d"]["ID"].asString();
      float gx = root["d"] ["gX"];
      float gy = root["d"] ["gY"];
      float gz = root["d"] ["gZ"];
      float ax = root["d"] ["aX"];
      float ay = root["d"] ["aY"];
      float az = root["d"] ["aZ"];
      float mx = root["d"] ["mX"];
      float my = root["d"] ["mY"];
      float mz = root["d"] ["mZ"];
      int analog = root["d"] ["Analog"]; //p0.26
      int accthr = root["d"] ["AccThr"];
      int button = root["d"] ["Button"];
      */
      //here you write values into fields of your ThingSpeak channel
      String data = "field1="+String(temp)+"&field2="+String(hum)+"&field3="+String(lux);
      Serial.println(data);
      // Write the fields that you've set all at once.
      // ThingSpeak will only accept updates every 15 seconds.
      tsUpdate(data);
      //Serial.println("T: "+String(temp)+" H: "+String(hum)+" L: "+String(lux));

      data[0] = '\0'; //clear data array
      data[0] = (char)0; //Just to be sure :D
      break;
    } 
  }
}
