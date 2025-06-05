#pragma once

#include <string_view>
#include "mqtt/mqtt-client-singlethread.hpp"

struct CJSON;  // Forward declare cJSON

struct ControllerConfig {
	const char* host{nullptr};
	int port{MQTT_DEFAULT_PORT};
	const char* bind_address{nullptr};
	int keepalive{60};

	const char* client_id{nullptr};
	const char* username{nullptr};
	const char* password{nullptr};
	MqttSslConfig ssl_config;
};

class Controller {
   public:
	void connect(const ControllerConfig& config);
	void disconnect();

	void publishTelemetry(std::string_view json);
	void publishTelemetry(CJSON* json);
	void publishAttributes(std::string_view json);
	void publishAttributes(CJSON* json);

   private:
	MqttClientSingleThread m_mqtt_client;
};
