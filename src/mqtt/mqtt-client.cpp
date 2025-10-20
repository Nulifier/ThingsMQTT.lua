#include "mqtt-client.hpp"
#include <stdexcept>

MqttClient::~MqttClient() {
	if (m_mosq != nullptr) {
		mosquitto_destroy(m_mosq);
		m_mosq = nullptr;
	}

	mosquitto_lib_cleanup();
}

void MqttClient::configure(const char* client_id,
						   const char* username,
						   const char* password,
						   const MqttSslConfig* ssl_config) {
	if (lib_init() != MOSQ_ERR_SUCCESS) {
		throw std::runtime_error("Failed to initialize MQTT library");
	}

	if (m_mosq != nullptr) {
		return;	 // Already configured
	}

	m_mosq = mosquitto_new(client_id, true, this);
	if (m_mosq == nullptr) {
		if (errno == ENOMEM) {
			throw std::bad_alloc();
		} else if (errno == EINVAL) {
			throw std::invalid_argument(
				"Invalid input parameters to mosquitto_new");
		}
	}

	// Set username and password if provided
	if (username != nullptr) {
		if (mosquitto_username_pw_set(m_mosq, username, password) !=
			MOSQ_ERR_SUCCESS) {
			mosquitto_destroy(m_mosq);
			m_mosq = nullptr;
			throw std::runtime_error("Failed to set username and password");
		}
	}

	// Configure SSL/TLS if provided
	if (ssl_config != nullptr) {
		int rc = mosquitto_tls_set(m_mosq, ssl_config->ca_file, nullptr,
								   ssl_config->cert_file, ssl_config->key_file,
								   nullptr);
		if (rc != MOSQ_ERR_SUCCESS) {
			mosquitto_destroy(m_mosq);
			m_mosq = nullptr;
			throw std::runtime_error("Failed to set TLS configuration");
		}
	}

	// Set the reconnect delay parameters
	mosquitto_reconnect_delay_set(m_mosq, 1, 30, true);

	after_configure();
}

void MqttClient::connect(const char* host, int port, int keepalive) {
	if (mosquitto_connect(m_mosq, host, port, keepalive) != MOSQ_ERR_SUCCESS) {
		throw std::runtime_error("Failed to connect to MQTT broker");
	}
}

void MqttClient::connect(const char* host,
						 const char* bind_address,
						 int port,
						 int keepalive) {
	if (mosquitto_connect_bind(m_mosq, host, port, keepalive, bind_address) !=
		MOSQ_ERR_SUCCESS) {
		throw std::runtime_error("Failed to connect to MQTT broker");
	}
}

void MqttClient::disconnect() {
	mosquitto_disconnect(m_mosq);
}

void MqttClient::subscribe(const char* topic, MqttQos qos) {
	int rc = mosquitto_subscribe(m_mosq, nullptr, topic, static_cast<int>(qos));
	if (rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_NO_CONN) {
		throw std::runtime_error("Failed to subscribe to topic");
	}

	// Record the subscription for (re)applying on reconnection
	m_qos_subscriptions[static_cast<int>(qos)].emplace_back(topic);
}

void MqttClient::unsubscribe(const char* topic) {
	int rc = mosquitto_unsubscribe(m_mosq, nullptr, topic);
	if (rc != MOSQ_ERR_SUCCESS && rc != MOSQ_ERR_NO_CONN) {
		throw std::runtime_error("Failed to unsubscribe from topic");
	}

	// Remove the subscription from all QoS lists
	for (auto& qos_list : m_qos_subscriptions) {
		auto it = std::remove_if(
			qos_list.begin(), qos_list.end(),
			[topic](const std::string& t) { return t == topic; });
		if (it != qos_list.end()) {
			qos_list.erase(it, qos_list.end());
		}
	}
}

bool MqttClient::publish(const char* topic,
						 std::string_view payload,
						 MqttQos qos,
						 int* message_id,
						 bool retain) {
	int rc = mosquitto_publish(m_mosq, message_id, topic,
							   static_cast<int>(payload.size()), payload.data(),
							   static_cast<int>(qos), retain);
	if (rc == MOSQ_ERR_SUCCESS) {
		return true;
	} else if (rc == MOSQ_ERR_NO_CONN) {
		return false;
	} else {
		throw std::runtime_error("Failed to publish message");
	}
}
