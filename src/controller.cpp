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

bool Controller::send() {
	if (m_tainted_telemetry_keys.empty()) {
		return false;
	}

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

	return true;
}

void Controller::loop() {
	m_mqtt_client.loop();
}

void Controller::sendTelemetry(nlohmann::json&& payload) {
	m_mqtt_client.publish(THINGSMQTT_TELEMETRY_TOPIC, payload.dump());
}
