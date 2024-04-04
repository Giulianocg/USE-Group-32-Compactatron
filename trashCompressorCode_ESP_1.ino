//Connection Vars

#include <WiFi.h>
#include <HTTPClient.h>
const char* Wifi_name = "KPNC254F6";
const char* Wifi_pass =  "password";
const char* host = "engdesblinds.000webhostapp.com";

const char* ID =  "1";
const char* Priv_pass =  "Bussiness1";
const char* Pub_pass = "Password123456-";

String base_url = "http://engdesblinds.000webhostapp.com/TX.php?pubpw=%27" + String(Pub_pass) + "%27&privpw=" + String(Priv_pass) + "&un=1&id=" + String(ID);

//#define RXp2 16
//#define TXp2 17

char c;
String dataIn;

void setup() {
  Serial.begin(115200); //ESP tp monitor
  Serial2.begin(9600); //ESP to UNO

  //internet connection
  WiFi.begin(Wifi_name, Wifi_pass);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());  //If you dont see this, then you put the wrong password or name
}

void loop() {
c=0;
dataIn="";

if (WiFi.status() == WL_CONNECTED) { //connect to wifi
  Serial2.print(1); //to flicker green LEDs
  WiFiClient client;
  HTTPClient http;

  while(Serial2.available()){
    c=Serial2.read();

    if(c != '\n'){
      dataIn += c;
    } else{
      break;
    }
  }

  if(c =='\n'){
    Serial.println(dataIn); //monitor
  
    if (http.begin(client, base_url + dataIn)) {
      Serial.print("[HTTP] Sent...\n");
      Serial2.print(3); //to long-flicker green LEDs
      int httpCode = http.GET();
      if (httpCode <= 0) {
        Serial.print("[HTTP] Failed to update. \n");
      }
      http.end();
    } else {
      Serial.printf("[HTTP] Unable to connect to website \n");
      Serial2.print(4); //to long-flicker red LEDs
    }
  }


} else{
  Serial.println("WiFi Disconnected");
  Serial2.print(0); //to flicker red LEDs
}
delay(300);
}