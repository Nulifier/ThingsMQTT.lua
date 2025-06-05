#pragma once

#include <atomic>
#include "mqtt-client.hpp"
#include "threadsafe-queue.hpp"

class MqttClientThreadSafe : public MqttClient {
   private:
	enum class MqttEventType : uint8_t {
		Connect,
		Disconnect,
		Publish,
		Message,
		Subscribe,
		Unsubscribe
	};

	struct MqttEvent {
		std::string topic;
		std::string payload;
		union {
			MqttConnectRc rc;
			int message_id;
		};
		MqttEventType type;
		MqttQos qos;
		bool retain;
	};

   public:
	~MqttClientThreadSafe() override;

	void loop() override;

	bool is_connected() const override { return m_connected.load(); }

   private:
	int lib_init() override;
	void after_configure() override;

	static void on_connect(struct mosquitto* mosq, void* obj, int rc);
	static void on_disconnect(struct mosquitto* mosq, void* obj, int rc);
	static void on_publish(struct mosquitto* mosq, void* obj, int message_id);
	static void on_message(struct mosquitto* mosq,
						   void* obj,
						   const struct mosquitto_message* message);
	static void on_subscribe(struct mosquitto* mosq,
							 void* obj,
							 int mid,
							 int qos_count,
							 const int* granted_qos);
	static void on_unsubscribe(struct mosquitto* mosq, void* obj, int mid);
	static void on_log(struct mosquitto* mosq,
					   void* obj,
					   int level,
					   const char* msg);

	// Holds if the client is connected
	// Set by the network thread, read by the main thread
	std::atomic<bool> m_connected{false};

	std::mutex m_lib_init_mutex;
	ThreadSafeQueue<MqttEvent> m_event_queue;
};
