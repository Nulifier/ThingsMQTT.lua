#pragma once

#include "mqtt-client.hpp"

/**
 * A single-threaded MQTT client.
 */
class MqttClientSingleThread : public MqttClient {
   public:
	~MqttClientSingleThread() override;

	void loop() override;

	bool is_connected() const override { return m_connected; }

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

	bool m_connected{false};
};
