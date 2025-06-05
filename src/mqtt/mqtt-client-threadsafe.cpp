#include "mqtt-client-threadsafe.hpp"
#include <algorithm>
#include <stdexcept>

MqttClientThreadSafe::~MqttClientThreadSafe() {
	if (m_mosq != nullptr) {
		mosquitto_disconnect(m_mosq);
		mosquitto_loop_stop(m_mosq, true);
	}
}

void MqttClientThreadSafe::loop() {
	MqttEvent event;
	while (auto event = m_event_queue.pop()) {
		switch (event->type) {
			case MqttEventType::Connect:
				if (m_connect_callback) {
					m_connect_callback(static_cast<MqttConnectRc>(event->rc));
				}
				break;
			case MqttEventType::Disconnect:
				if (m_disconnect_callback) {
					m_disconnect_callback(event->rc);
				}
				break;
			case MqttEventType::Publish:
				if (m_publish_callback) {
					m_publish_callback(event->message_id);
				}
				break;
			case MqttEventType::Message:
				if (m_message_callback) {
					m_message_callback(event->message_id, event->topic.c_str(),
									   std::string_view(event->payload),
									   event->qos, event->retain);
				}
				break;
			case MqttEventType::Subscribe:
				if (m_subscribe_callback) {
					m_subscribe_callback(event->message_id);
				}
				break;
			case MqttEventType::Unsubscribe:
				if (m_unsubscribe_callback) {
					m_unsubscribe_callback(event->message_id);
				}
				break;
			default:
				// Unknown event type, should not happen
				break;
		}
	}
}

int MqttClientThreadSafe::lib_init() {
	std::lock_guard<std::mutex> lock(m_lib_init_mutex);
	return mosquitto_lib_init();
}

void MqttClientThreadSafe::after_configure() {
	// Set callbacks
	mosquitto_connect_callback_set(m_mosq, on_connect);
	mosquitto_disconnect_callback_set(m_mosq, on_disconnect);
	mosquitto_publish_callback_set(m_mosq, on_publish);
	mosquitto_message_callback_set(m_mosq, on_message);
	mosquitto_subscribe_callback_set(m_mosq, on_subscribe);
	mosquitto_unsubscribe_callback_set(m_mosq, on_unsubscribe);
	mosquitto_log_callback_set(m_mosq, on_log);

	// Start the network loop in a background thread
	if (mosquitto_loop_start(m_mosq) != MOSQ_ERR_SUCCESS) {
		mosquitto_destroy(m_mosq);
		m_mosq = nullptr;
		throw std::runtime_error("Failed to start mosquitto network loop");
	}
}

void MqttClientThreadSafe::on_connect(struct mosquitto* mosq,
									  void* obj,
									  int rc) {
	auto* client = static_cast<MqttClientThreadSafe*>(obj);

	// Reapply all subscriptions
	for (size_t qos = 0; qos < client->m_qos_subscriptions.size(); ++qos) {
		for (const auto& topic : client->m_qos_subscriptions[qos]) {
			mosquitto_subscribe(mosq, nullptr, topic.c_str(),
								static_cast<int>(qos));
		}
	}

	client->m_connected =
		static_cast<MqttConnectRc>(rc) == MqttConnectRc::Accepted;

	client->m_event_queue.emplace(MqttEvent{
		.rc = static_cast<MqttConnectRc>(rc),
		.type = MqttEventType::Connect,
	});
}

void MqttClientThreadSafe::on_disconnect(struct mosquitto* mosq,
										 void* obj,
										 int rc) {
	auto* client = static_cast<MqttClientThreadSafe*>(obj);

	client->m_connected = false;

	client->m_event_queue.emplace(MqttEvent{
		.rc = static_cast<MqttConnectRc>(rc),
		.type = MqttEventType::Disconnect,
	});
}

void MqttClientThreadSafe::on_publish(struct mosquitto* mosq,
									  void* obj,
									  int message_id) {
	auto* client = static_cast<MqttClientThreadSafe*>(obj);
	client->m_event_queue.emplace(MqttEvent{
		.message_id = message_id,
		.type = MqttEventType::Publish,
	});
}

void MqttClientThreadSafe::on_message(struct mosquitto* mosq,
									  void* obj,
									  const struct mosquitto_message* message) {
	auto* client = static_cast<MqttClientThreadSafe*>(obj);
	client->m_event_queue.emplace(MqttEvent{
		.topic = message->topic,
		.payload = std::string(static_cast<const char*>(message->payload),
							   message->payloadlen),
		.message_id = message->mid,
		.type = MqttEventType::Message,
		.qos = static_cast<MqttQos>(message->qos),
		.retain = message->retain != 0,
	});
}

void MqttClientThreadSafe::on_subscribe(struct mosquitto* mosq,
										void* obj,
										int mid,
										int qos_count,
										const int* granted_qos) {
	auto* client = static_cast<MqttClientThreadSafe*>(obj);
	client->m_event_queue.emplace(MqttEvent{
		.message_id = mid,
		.type = MqttEventType::Subscribe,
	});
}

void MqttClientThreadSafe::on_unsubscribe(struct mosquitto* mosq,
										  void* obj,
										  int mid) {
	auto* client = static_cast<MqttClientThreadSafe*>(obj);
	client->m_event_queue.emplace(MqttEvent{
		.message_id = mid,
		.type = MqttEventType::Unsubscribe,
	});
}

void MqttClientThreadSafe::on_log(struct mosquitto* mosq,
								  void* obj,
								  int level,
								  const char* msg) {
	// Currently ignored
	(void)mosq;
	(void)obj;
	(void)level;
	(void)msg;
}
