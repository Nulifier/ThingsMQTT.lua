#include "controller.hpp"
#include <ctime>
#include <sstream>
#include <stdexcept>

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

void Controller::publishTelemetry(std::string_view json) {
	if (!m_mqtt_client.is_connected()) {
		throw std::runtime_error("MQTT client is not connected");
	}

	// Build the payload with timestamp
	std::ostringstream payload;
	payload << "{\"ts\":" << std::time(nullptr) * 1000 << ",\"values\":" << json
			<< "}";

	if (!m_mqtt_client.publish("v1/devices/me/telemetry", payload.str(),
							   MqttQos::AtLeastOnce, false)) {
		// Client is disconnected, no way to handle currently
	}
}
