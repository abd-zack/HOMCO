#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "admin123";
const char* password = "Unja20237#";
const char* mqtt_server = "broker.emqx.io"; // Tanpa wss:// karena ESP32 pakai TCP
const int mqtt_port = 1883;
 
WiFiClient espClient;
PubSubClient client(espClient);
 
// Mapping topik ke pin
struct RelayControl {
  const char* topic;
  int pin;
};

RelayControl relays[] = {
  {"home/relay1", 5},
  {"home/relay2", 18},
  {"home/relay3", 19},
  {"home/relay4", 21}
};
const int numRelays = sizeof(relays) / sizeof(relays[0]);

const int buzzerPin = 4; // Buzzer di pin GPIO4

void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void bunyikanBuzzer() {
  digitalWrite(buzzerPin, HIGH);
  delay(100);
  digitalWrite(buzzerPin, LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];
  msg.trim();

  Serial.print("Pesan diterima: ");
  Serial.print(topic);
  Serial.print(" -> ");
  Serial.println(msg);

  for (int i = 0; i < numRelays; i++) {
    if (String(topic) == relays[i].topic) {
      if (msg.equalsIgnoreCase("on")) {
        digitalWrite(relays[i].pin, HIGH);
        Serial.print("Relay ON di pin: ");
        Serial.println(relays[i].pin);
        bunyikanBuzzer();
      } else if (msg.equalsIgnoreCase("off")) {
        digitalWrite(relays[i].pin, LOW);
        Serial.print("Relay OFF di pin: ");
        Serial.println(relays[i].pin);
        bunyikanBuzzer();
      }
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT Broker...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung!");
      for (int i = 0; i < numRelays; i++) {
        client.subscribe(relays[i].topic);
        Serial.print("Subscribe ke topik: ");
        Serial.println(relays[i].topic);
      }
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" mencoba lagi dalam 5 detik...");
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  for (int i = 0; i < numRelays; i++) {
    pinMode(relays[i].pin, OUTPUT);
    digitalWrite(relays[i].pin, LOW);
  }

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
}
