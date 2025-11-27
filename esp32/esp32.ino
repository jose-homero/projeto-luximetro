/*
  Cliente MQTT para ESP32 com Sensor AS7341 
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Adafruit_AS7341.h> //Biblioteca da Adafruit para o AS7341
#include <ArduinoJson.h>

// CREDENCIAIS WIFI
const char* WIFI_SSID = "USUARIO";
const char* WIFI_PASS = "SENHA";

// CREDENCIAIS DO BROKER MQTT
const char* MQTT_SERVER = "0000000000000.s1.eu.hivemq.cloud"; 
const int MQTT_PORT = 0000;
const char* MQTT_USER = "USUARIO"; 
const char* MQTT_PASS = "SENHA"; 

// CREDENCIAIS MQTT
const char* JSON_TOPIC = "luximetro/sala/sensores"; 
const char* LOG_TOPIC = "luximetro/sala/logs";
const char* STATUS_TOPIC = "luximetro/sala/status";
const char* CLIENT_ID = "esp32-sensor-spectral";

// Deep Sleep
#define uS_TO_S_FACTOR 1000000ULL /*Fator de conversao*/
#define TIME_TO_SLEEP 30 /* 30s */

WiFiClientSecure espClient;
PubSubClient client(espClient);
Adafruit_AS7341 as7341;

// DECLARACAO DAS FUNCOES
void reconnect();
void setup_wifi();
bool enviarDadosMQTT(float valores_a_enviar[]);
void logToSerialAndMQTT(String message); 

// FUNÇÃO SETUP
void setup() {
  Serial.begin(115200);
  delay(1000);

  logToSerialAndMQTT("\n--- ACORDANDO E INICIANDO ---");

// IMPRIME MAC DO ESP32
  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.println("========================================");
  Serial.println("O Endereco MAC deste microcontrolador e: " + WiFi.macAddress());

// STATUS DO AS7341
  Serial.print("1. Status do Sensor AS7341...... ");
  if (!as7341.begin()) {
    Serial.println("[FALHA]");
    esp_sleep_enable_timer_wakeup(10 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }
  logToSerialAndMQTT("[OK]");

// CONFIGURACOES DO SENSOR
  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);

// STATUS WIFI
  setup_wifi();

// CONFIGURACAO MQTT
  espClient.setInsecure();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  logToSerialAndMQTT("3. Configurando Broker MQTT em: " + String(MQTT_SERVER));

  if (!client.connected()) {
    reconnect();
  }

  logToSerialAndMQTT("--- VERIFICACAO CONCLUIDA ---");

  logToSerialAndMQTT("\nRealizando nova leitura do sensor...");
  uint16_t rawReadings[12];
  if (!as7341.readAllChannels(rawReadings)) {
    logToSerialAndMQTT("Erro na leitura do sensor!");  //FLAG DE ERRO
  } else {
    logToSerialAndMQTT("Leitura concluida.");

    // ARRAY P/ ARMAZENAR DADOS (AGORA DIRETO, SEM MULTIPLICACAO)
    float dados_a_enviar[10];
    
    dados_a_enviar[0] = (float)rawReadings[0];
    dados_a_enviar[1] = (float)rawReadings[1];
    dados_a_enviar[2] = (float)rawReadings[2];
    dados_a_enviar[3] = (float)rawReadings[3];
    dados_a_enviar[4] = (float)rawReadings[6]; // Pula indices 4 e 5
    dados_a_enviar[5] = (float)rawReadings[7];
    dados_a_enviar[6] = (float)rawReadings[8];
    dados_a_enviar[7] = (float)rawReadings[9];
    dados_a_enviar[8] = (float)rawReadings[10];
    dados_a_enviar[9] = (float)rawReadings[11];

    if (enviarDadosMQTT(dados_a_enviar)) {
      logToSerialAndMQTT("-> Dados enviados com sucesso.");
    } else {
      logToSerialAndMQTT("-> Falha ao enviar dados para o broker.");  //FLAG DE ERRO
    }
  }

  logToSerialAndMQTT("\nEntrando em Deep Sleep...");
  delay(500); // Garante envio do MQTT antes de desligar

// Despertador
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
// Inicia o sono
  esp_deep_sleep_start();
}

// FUNÇÃO LOOP (VAZIO NO DEEP SLEEP)
void loop() {
}

// FUNÇÃO PARA ENVIAR DADOS VIA MQTT
bool enviarDadosMQTT(float valores_a_enviar[]) {
  if (!client.connected()) return false;

  StaticJsonDocument<1024> doc;
  doc["f1_415nm"] = valores_a_enviar[0]; doc["f2_445nm"] = valores_a_enviar[1];
  doc["f3_480nm"] = valores_a_enviar[2]; doc["f4_515nm"] = valores_a_enviar[3];
  doc["f5_555nm"] = valores_a_enviar[4]; doc["f6_590nm"] = valores_a_enviar[5];
  doc["f7_630nm"] = valores_a_enviar[6]; doc["f8_680nm"] = valores_a_enviar[7];
  doc["clear"] = valores_a_enviar[8]; doc["nir"] = valores_a_enviar[9];

  char jsonPayload[1024];
  serializeJson(doc, jsonPayload);

  Serial.println("\nPublicando no topico de DADOS: " + String(JSON_TOPIC));
  Serial.println("Payload: " + String(jsonPayload));
  
  return client.publish(JSON_TOPIC, jsonPayload);
}

// FUNÇÃO WIFI
void setup_wifi() {
  Serial.print("2. Status da Conexao WiFi....... ");
  delay(100);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500); Serial.print("."); retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" [OK]");
    Serial.print(" -> Conectado com IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println(" [FALHA]");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }
}

// FUNÇÃO RECONEXAO
void reconnect() {
  if (!client.connected()) {
    logToSerialAndMQTT("\n4. Tentando conectar ao Broker MQTT na Nuvem... ");
    if (client.connect(CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      logToSerialAndMQTT("[OK]");
      client.publish(STATUS_TOPIC, "online");
    } else {
      String errorMsg = "[FALHA] - Codigo de erro: " + String(client.state());
      Serial.println(errorMsg);
    }
  }
}
  
void logToSerialAndMQTT(String message) {
  Serial.println(message);
  if (client.connected()) {
    client.publish(LOG_TOPIC, message.c_str());
    delay(10); 
  }
}