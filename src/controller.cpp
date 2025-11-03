#include "controller.hpp"
#include <ctime>
#include <sstream>
#include <stdexcept>
#include "thingsmqtt-config.hpp"

void Controller::connect(const ControllerConfig& config) {
	ControllerConfig cfg = config;	// Make a copy to modify

	if (cfg.host == nullptr) {
		throw std::runtime_error("MQTT host is not set");
	}
	if (cfg.port <= 0 || cfg.port > 65535) {
		throw std::runtime_error("Invalid MQTT port");
	}
	if (cfg.keepalive <= 0) {
		cfg.keepalive = 60;
	}

	m_mqtt_client.configure(cfg.client_id, cfg.username, cfg.password,
							&cfg.ssl_config);
	if (cfg.bind_address != nullptr) {
		m_mqtt_client.connect(cfg.host, cfg.bind_address, cfg.port,
							  cfg.keepalive);
	} else {
		m_mqtt_client.connect(cfg.host, cfg.port, cfg.keepalive);
	}
}

void Controller::disconnect() {
	m_mqtt_client.disconnect();
}

void Controller::publishTelemetry(const char* key, nlohmann::json&& value) {
	// Check if the old value is different
	auto it = m_telemetry_data.find(key);
	if (it == m_telemetry_data.end()) {
		// New telemetry key
		m_telemetry_data.emplace(key, std::move(value));
		m_tainted_telemetry_keys.insert(key);
	} else if (it->second != value) {
		// Existing telemetry key with a new value
		it->second = std::move(value);
		m_tainted_telemetry_keys.insert(key);
	} else {
		// No change in telemetry value
	}
}

void Controller::setAttribute(const char* key, nlohmann::json&& value) {
	// Check if the old value is different
	auto it = m_attribute_data.find(key);
	if (it == m_attribute_data.end()) {
		// New attribute key
		m_attribute_data.emplace(key, std::move(value));
		m_tainted_attribute_keys.insert(key);
	} else if (it->second != value) {
		// Existing attribute key with a new value
		it->second = std::move(value);
		m_tainted_attribute_keys.insert(key);
	} else {
		// No change in attribute value
	}
}

bool Controller::send() {
	bool data_to_send = false;

	if (!m_tainted_telemetry_keys.empty()) {
		// Create a JSON object for the telemetry values
		nlohmann::json telemetry_values;
		for (const auto& key : m_tainted_telemetry_keys) {
			telemetry_values[key] = m_telemetry_data[key];
		}

		// Create the JSON payload
		nlohmann::json payload;
		payload["values"] = telemetry_values;
		payload["ts"] = std::time(nullptr) * 1000;

		// Publish the telemetry data
		sendTelemetry(std::move(payload));

		// Clear the tainted keys
		m_tainted_telemetry_keys.clear();

		data_to_send = true;
	}

	if (!m_tainted_attribute_keys.empty()) {
		// Create a JSON object for the attribute values
		nlohmann::json attribute_values;
		for (const auto& key : m_tainted_attribute_keys) {
			attribute_values[key] = m_attribute_data[key];
		}

		// Publish the attribute data
		sendAttributes(std::move(attribute_values));

		// Clear the tainted keys
		m_tainted_attribute_keys.clear();

		data_to_send = true;
	}

	return data_to_send;
}

void Controller::loop() {
	m_mqtt_client.loop();
}

size_t Controller::addRpcHandler(RpcHandler handler) {
	static size_t next_id = 1;
	m_rpc_handlers[next_id] = std::move(handler);
	return next_id++;
}

bool Controller::removeRpcHandler(size_t handler_id) {
	return m_rpc_handlers.erase(handler_id) > 0;
}

void Controller::sendTelemetry(nlohmann::json&& payload) {
	// If we are connected, publish immediately
	if (m_mqtt_client.is_connected()) {
		m_mqtt_client.publish(THINGSMQTT_TELEMETRY_TOPIC, payload.dump());
	} else {
		// Queue the telemetry data for later sending
		m_pending_telemetry.push_back(payload.dump());
	}
}

void Controller::sendAttributes(nlohmann::json&& payload) {
	// If we are connected, publish immediately
	if (m_mqtt_client.is_connected()) {
		m_mqtt_client.publish(THINGSMQTT_ATTRIBUTES_TOPIC, payload.dump());
	} else {
		// All attributes must be sent when connected, so we'll just send the
		// latest then
	}
}

void Controller::onMqttConnect(MqttConnectRc rc) {
	if (rc != MqttConnectRc::Accepted) {
		// Connection failed
		return;
	}

	// Send all attributes
	if (!m_attribute_data.empty()) {
		nlohmann::json attribute_values;
		for (const auto& [key, value] : m_attribute_data) {
			attribute_values[key] = value;
		}
		sendAttributes(std::move(attribute_values));
	}

	// Send any pending telemetry data
	while (!m_pending_telemetry.empty()) {
		const std::string& payload = m_pending_telemetry.front();
		m_mqtt_client.publish(THINGSMQTT_TELEMETRY_TOPIC, payload);
		m_pending_telemetry.pop_front();
	}
}
