#include "mqtt-client-singlethread.hpp"
#include <stdexcept>

MqttClientSingleThread::~MqttClientSingleThread() {
	if (m_mosq != nullptr) {
		mosquitto_disconnect(m_mosq);
	}
}

void MqttClientSingleThread::loop() {
	if (m_mosq == nullptr) {
		throw std::runtime_error("MQTT client is not initialized");
	}

	int rc = mosquitto_loop(m_mosq, -1, 1);
	if (rc == MOSQ_ERR_CONN_LOST || rc == MOSQ_ERR_NO_CONN) {
		m_connected = false;
		return;
	}
	if (rc != MOSQ_ERR_SUCCESS) {
		throw std::runtime_error("Error in mosquitto loop");
	}
}

int MqttClientSingleThread::lib_init() {
	return mosquitto_lib_init();
}

void MqttClientSingleThread::after_configure() {
	// Set callbacks
	mosquitto_connect_callback_set(m_mosq, on_connect);
	mosquitto_disconnect_callback_set(m_mosq, on_disconnect);
	mosquitto_publish_callback_set(m_mosq, on_publish);
	mosquitto_message_callback_set(m_mosq, on_message);
	mosquitto_subscribe_callback_set(m_mosq, on_subscribe);
	mosquitto_unsubscribe_callback_set(m_mosq, on_unsubscribe);
	mosquitto_log_callback_set(m_mosq, on_log);
}

void MqttClientSingleThread::on_connect(struct mosquitto* mosq,
										void* obj,
										int rc) {
	auto* client = static_cast<MqttClientSingleThread*>(obj);

	// Reapply all subscriptions
	for (size_t qos = 0; qos < client->m_qos_subscriptions.size(); ++qos) {
		for (const auto& topic : client->m_qos_subscriptions[qos]) {
			mosquitto_subscribe(mosq, nullptr, topic.c_str(),
								static_cast<int>(qos));
		}
	}

	client->m_connected =
		static_cast<MqttConnectRc>(rc) == MqttConnectRc::Accepted;

	if (client->m_connect_callback) {
		client->m_connect_callback(static_cast<MqttConnectRc>(rc));
	}
}

void MqttClientSingleThread::on_disconnect(struct mosquitto* mosq,
										   void* obj,
										   int rc) {
	auto* client = static_cast<MqttClientSingleThread*>(obj);

	client->m_connected = false;

	if (client->m_disconnect_callback) {
		client->m_disconnect_callback(static_cast<MqttConnectRc>(rc));
	}
}

void MqttClientSingleThread::on_publish(struct mosquitto* mosq,
										void* obj,
										int message_id) {
	auto* client = static_cast<MqttClientSingleThread*>(obj);
	if (client->m_publish_callback) {
		client->m_publish_callback(message_id);
	}
}

void MqttClientSingleThread::on_message(
	struct mosquitto* mosq,
	void* obj,
	const struct mosquitto_message* message) {
	auto* client = static_cast<MqttClientSingleThread*>(obj);
	if (client->m_message_callback) {
		client->m_message_callback(
			message->mid, message->topic,
			std::string_view(static_cast<const char*>(message->payload),
							 message->payloadlen),
			static_cast<MqttQos>(message->qos), message->retain != 0);
	}
}

void MqttClientSingleThread::on_subscribe(struct mosquitto* mosq,
										  void* obj,
										  int mid,
										  int qos_count,
										  const int* granted_qos) {
	auto* client = static_cast<MqttClientSingleThread*>(obj);
	if (client->m_subscribe_callback) {
		client->m_subscribe_callback(mid);
	}
}

void MqttClientSingleThread::on_unsubscribe(struct mosquitto* mosq,
											void* obj,
											int mid) {
	auto* client = static_cast<MqttClientSingleThread*>(obj);
	if (client->m_unsubscribe_callback) {
		client->m_unsubscribe_callback(mid);
	}
}

void MqttClientSingleThread::on_log(struct mosquitto* mosq,
									void* obj,
									int level,
									const char* msg) {
	// Optionally handle logging here
	(void)mosq;
	(void)obj;
	(void)level;
	(void)msg;
}
