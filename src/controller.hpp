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
	typedef std::function<nlohmann::json(const std::string& method,
										 const nlohmann::json& params)>
		RpcHandler;

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

	size_t addRpcHandler(RpcHandler handler);
	bool removeRpcHandler(size_t handler_id);

   protected:
	virtual void sendTelemetry(nlohmann::json&& payload);
	virtual void sendAttributes(nlohmann::json&& payload);

   private:
	MqttClientSingleThread m_mqtt_client;

	std::unordered_map<std::string, nlohmann::json> m_telemetry_data;
	std::unordered_set<std::string> m_tainted_telemetry_keys;

	std::unordered_map<std::string, nlohmann::json> m_attribute_data;
	std::unordered_set<std::string> m_tainted_attribute_keys;

	std::deque<std::string> m_pending_telemetry;

	std::unordered_map<size_t, RpcHandler> m_rpc_handlers;

	void onMqttConnect(MqttConnectRc rc);
	void onMqttMessage(int message_id,
					   const char* topic,
					   std::string_view payload,
					   MqttQos qos,
					   bool retain);
};
