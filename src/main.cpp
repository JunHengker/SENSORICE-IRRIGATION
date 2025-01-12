#include <Arduino.h>
#include "constants.h"
#include "WiFiHandler.h"
#include "MQTTHandler.h"
#include "MessageHandler.h"

// WiFi and MQTT
WiFiHandler wifiHandler("SensoRiceIrrigationAP");
MQTTHandler mqttHandler(MQTT_SERVER, MQTT_PORT, MQTT_TOPIC_COMMAND);

void mqttCallback(char *topic, byte *payload, uint16_t length)
{
  String data = "";
  for (int i = 0; i < length; i++)
  {
    data += (char)payload[i];
  }

  Serial.printf("[MQTT:INFO] Message received on topic '%s': %s\n", topic, data.c_str());

  // Validate the message
  if (!MessageHandler::isMessageValid(data, MACHINE_ID))
  {
    Serial.println(F("[MQTT:ERROR] Invalid message"));
    return;
  }

  // Parse command and value from the message
  String command = MessageHandler::getValue(data, '#', 1);
  String value = MessageHandler::getValue(data, '#', 2);

  Serial.printf("[MQTT:INFO] Command: %s, Value: %s\n", command.c_str(), value.c_str());

  // Handle the command
  if (command == "startIrrigation")
  {
    Serial.println(F("[MQTT:ACTION] Handling irrigation command..."));
    // Turn on the valve
    digitalWrite(PIN_valve, HIGH);
    String response = "{ \"machine_id\": \"" + String(MACHINE_ID) + "\", \"valve_status\": \"ON\" }";
    mqttHandler.publishMessage(MQTT_TOPIC_RESPONSE, response);
  }
  else if (command == "stopIrrigation")
  {
    Serial.println(F("[MQTT:ACTION] Handling stop irrigation command..."));
    // Turn off the valve
    digitalWrite(PIN_valve, LOW);
    String response = "{ \"machine_id\": \"" + String(MACHINE_ID) + "\", \"valve_status\": \"OFF\" }";
    mqttHandler.publishMessage(MQTT_TOPIC_RESPONSE, response);
  }
  else if (command == "status")
  {
    Serial.println(F("[MQTT:ACTION] Handling get status command..."));
    // Get the status of the valve
    String status = digitalRead(PIN_valve) == HIGH ? "ON" : "OFF";
    String response = "{ \"machine_id\": \"" + String(MACHINE_ID) + "\", \"valve_status\": \"" + status + "\" }";
    mqttHandler.publishMessage(MQTT_TOPIC_RESPONSE, response);
  }
  else
  {
    Serial.println(F("[MQTT:ERROR] Unknown command"));
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("[INFO] Starting SensoRice Irrigation System..."));

  // Setup WiFi and MQTT
  wifiHandler.setupWiFi();
  mqttHandler.setup(mqttCallback);

  // PIN setup
  pinMode(PIN_valve, OUTPUT);

  Serial.println(F("[INFO] System setup complete"));
}

void loop()
{
  // Ensure WiFi and MQTT are connected
  wifiHandler.ensureConnection();
  mqttHandler.ensureConnection();

  mqttHandler.loop();

  delay(2000);
}
