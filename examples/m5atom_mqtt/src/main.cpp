/*
This example runs on the M5Atom, with the SIM7020 DTU, and setting the LED to indicate initialisation (yellow),
ready (green flash), message sending/receiving (blue flash), and fatal errors (red).

It is configured to connect to a test MQTT server set in Azure by the scripts from the example at:
https://github.com/sgryphon/iot-demo-build/blob/main/azure-mosquitto/README-mosquitto.md

To run, start the test server, use a terminal to connect to it and follow the logs:

./start-mosquitto.ps1
ssh iotadmin@mqdev01-0xacc5.australiaeast.cloudapp.azure.com
sudo tail -f /var/log/mosquitto/mosquitto.log

In another shell, start a mosquitto client listening on all topics:

$mqttPassword = 'YourSecretPassword'
mosquitto_sub -h mqdev01-0xacc5.australiaeast.cloudapp.azure.com -t '#' -F '%I %t [%l] %p' -p 8883 -u mqttuser -P $mqttPassword

Then, in the PIO shell, deploy (upload) to your device, and then monitor the serial output:

export PIO_MQTT_PASSWORD=YourMqttPassword3
(export PIO_VERSION=$(git describe --tags --dirty); pio run --target upload)
pio device monitor --baud 115200

To test downstream, use another terminal:

$mqttPassword = 'YourSecretPassword'
mosquitto_pub -h mqdev01-0xacc5.australiaeast.cloudapp.azure.com -t 'test/c2d' -p 8883 -u mqttuser -P $mqttPassword -m '{\"interval_s\": 60}'

For a screen shot, see: test-mqtt-tls-ipv6-nbiot.png

*/
#include "../../../src/SIM7020/SIM7020MqttClient.h"
#include <M5Atom.h>
#include <Arduino.h>

#define ST(A) #A
#define STR(A) ST(A)

const char apn[] = "telstra.iot";

// NOTE: Maximum server name length is 50 characters
const char server[] = "mqdev01-0xacc5.australiaeast.cloudapp.azure.com";
const int16_t port = 8883;
const char mqtt_user[] = "dev00001";
const char mqtt_password[] = STR(PIO_MQTT_PASSWORD);
const char version[] = STR(PIO_VERSION);

#define GSM_BAUDRATE 115200
#define GSM_TX_PIN 22
#define GSM_RX_PIN 19

// #include <StreamDebugger.h>
// StreamDebugger debugger(Serial1, Serial);
// SIM7020GsmModem sim7020(debugger);
SIM7020GsmModem sim7020(Serial1);

SIM7020TcpClient sim7020tcp(sim7020);
SIM7020MqttClient sim7020mqtt(sim7020tcp, server, port, true);

GsmModem& modem = sim7020;
MqttClient& mqtt = sim7020mqtt;

extern const uint8_t root_ca_pem_start[] asm("_binary_src_certs_ISRG_Root_X1_pem_start");
extern const uint8_t root_ca_pem_end[] asm("_binary_src_certs_ISRG_Root_X1_pem_end");
const String root_ca((char *)root_ca_pem_start);

const char publish_topic[] = "dt/demo/1/dev00001";
const char subscribe_topic[] = "cmd/demo/1/dev00001/req";
String client_id;
char message_buffer[2000] = { 0 };
int32_t counter = 0;

bool ready = false;
bool failed = false;
int32_t led_off_ms = -1;
int32_t next_connected_blink_ms = -1;
int32_t next_message_ms = -1;
int32_t disconnect_ms = -1;

// const char* message_template = \
//   "{\"battery_volts\":%.3f," \
//   "\"battery_amps\":%.3f," \
//   "\"battery_watts\":%.1f," \
//   "\"temperature_celsius\":%.2f}";

const char message_template[] = "{\"counter\":%d}";

void buildMessage() {
    counter++;
    snprintf(message_buffer, 2000, message_template, counter);
}

void setup() {
  AdvancedGsmLog.Log = &Serial;
  M5.begin(true, true, true);
  delay(1000);
  Serial.printf("MQTT example started, v.%s\n", version);
  M5.dis.fillpix(0x333300); 
  Serial1.begin(GSM_BAUDRATE, SERIAL_8N1, GSM_RX_PIN, GSM_TX_PIN);
  modem.begin(apn);
}

void loop() {
  int32_t now = millis();
  M5.update();
  modem.loop();

  if (failed) return;

  if (!modem.isActive()) {
    Serial.println("Fatal error - modem is not active");
    failed = true;
    M5.dis.fillpix(CRGB::Red); 
    return;
  }

  if (!ready) {
    if (modem.modemStatus() >= ModemStatus::PacketDataReady) {
      Serial.println("Modem is ready");
      ready = true;
      String ip_addresses[4];
      modem.getLocalIPs(ip_addresses, 4);
      Serial.printf("Device IP address: %s\n", ip_addresses[0].c_str());
      bool ca_success = modem.setRootCA(root_ca);
      if (!ca_success) {
        Serial.println("Certificate failed");
        failed = true;
        M5.dis.fillpix(CRGB::Red); 
        return;
      }
      //client_id = "imei-" + modem.IMEI();
      client_id = mqtt_user;
      Serial.printf("Setting client ID to: %s\n", client_id.c_str());
      M5.dis.fillpix(CRGB::Green);
      led_off_ms = millis() + 500;
      next_message_ms = millis() + 1000;
    }
    return;
  }

  if (led_off_ms > 0 && millis() > led_off_ms) {
    M5.dis.clear();
    led_off_ms = -1;
  }

  if (next_message_ms > 0 && millis() > next_message_ms) {
    M5.dis.fillpix(CRGB::Blue); 
    int8_t rc = mqtt.connect(client_id.c_str(), mqtt_user, mqtt_password);
    if (rc != 0) {
      Serial.println("Connect failed");
      M5.dis.fillpix(CRGB::Red);
      failed = true;
      return;
    }
    Serial.printf("Subscribing to %s\n", subscribe_topic);
    mqtt.subscribe(subscribe_topic);
    buildMessage();
    Serial.printf("Publishing: %s\n", message_buffer);
    mqtt.publish(publish_topic, message_buffer);
    next_message_ms = now + 60000;
    int32_t start = millis();
    disconnect_ms =start + 6000;
    next_connected_blink_ms = start + 1000;
    led_off_ms = start + 100;
  }

  if (next_connected_blink_ms > 0 && now > next_connected_blink_ms) {
    M5.dis.fillpix(0x333300);
    int32_t blink = millis();
    next_connected_blink_ms = blink + 2000;
    led_off_ms = blink + 50;
  }

  String receive_topic = mqtt.receiveTopic();
  if (receive_topic.length() > 0) {
    M5.dis.fillpix(CRGB::Blue);
    led_off_ms = millis() + 200;
    String receive_body = mqtt.receiveBody();
    Serial.printf("Received [%s]: %s\n", receive_topic.c_str(), receive_body.c_str());
  }

  if (disconnect_ms > 0 && now > disconnect_ms) {
    Serial.print("Disconnecting\n");
    mqtt.disconnect();
    disconnect_ms = -1;
    next_connected_blink_ms = -1;
  }
}
