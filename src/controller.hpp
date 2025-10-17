#pragma once

#include <deque>
#include <nlohmann/json.hpp>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "mqtt/mqtt-client-singlethread.hpp"

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

	void publishTelemetry(const char* key, nlohmann::json&& value);

	void setAttribute(const char* key, nlohmann::json&& value);

	/**
	 * Sends any updated telemetry data.
	 * This function should be called regularly to ensure that telemetry data is
	 * sent in a timely manner.
	 * @return true if telemetry data was sent, false otherwise.
	 */
	bool send();

	void loop();

	bool isConnected() const { return m_mqtt_client.is_connected(); }

   protected:
	virtual void sendTelemetry(nlohmann::json&& payload);

   private:
	MqttClientSingleThread m_mqtt_client;

	std::unordered_map<std::string, nlohmann::json> m_telemetry_data;
	std::unordered_set<std::string> m_tainted_telemetry_keys;

	std::unordered_map<std::string, nlohmann::json> m_attribute_data;
	std::unordered_set<std::string> m_tainted_attribute_keys;

	std::deque<nlohmann::json> m_pending_telemetry;

	void onMqttConnect(MqttConnectRc rc);
};
