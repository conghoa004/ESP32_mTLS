#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// --- WiFi & MQTT ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_broker = "m51854c2.ala.asia-southeast1.emqxsl.com";
const int mqtt_port = 8883;
const char* mqtt_username = "hoaze";
const char* mqtt_password = "Hoa1234#";
const char* client_id = "conghoa_004";
const char* mqtt_topic_send = "esp32";
const char* mqtt_topic_sub = "mqttx";

// --- TLS Root CA ---
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";

// --- MQTT client ---
WiFiClientSecure esp_client;
PubSubClient mqtt(esp_client);

// --- Callback nhận dữ liệu ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Received ["); Serial.print(topic); Serial.print("]: ");
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  Serial.println(msg);
}

// --- Kết nối WiFi ---
void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

// --- Kết nối MQTT ---
void connectMQTT() {
  mqtt.setCallback(mqttCallback); // đặt callback trước khi connect
  while (!mqtt.connected()) {
    if (mqtt.connect(client_id, mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
      mqtt.subscribe(mqtt_topic_sub); // đăng ký nhận dữ liệu
    } else {
      Serial.print("MQTT fail, rc=");
      Serial.println(mqtt.state());
      delay(5000);
    }
  }
}

// --- Gửi dữ liệu mẫu ---
unsigned long lastSend = 0;
void sendData() {
  static int counter = 0;
  char payload[50];
  snprintf(payload, sizeof(payload), "{\"data\":%d}", counter++);
  mqtt.publish(mqtt_topic_send, payload);
  Serial.println("Sent: " + String(payload));
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  esp_client.setCACert(ca_cert);
  mqtt.setServer(mqtt_broker, mqtt_port);
  connectMQTT();
}

void loop() {
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop(); // xử lý callback
  if (millis() - lastSend > 2000) { // gửi mỗi 2 giây
    lastSend = millis();
    sendData();
  }
}
