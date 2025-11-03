#pragma once

#include <mosquitto.h>
#include <functional>
#include <string>
#include <string_view>

const int MQTT_DEFAULT_PORT = 1883;
const int MQTTS_DEFAULT_PORT = 8883;

enum class MqttQos : uint8_t {
	AtMostOnce = 0,
	AtLeastOnce = 1,
	ExactlyOnce = 2
};

enum class MqttConnectRc {
	Accepted = 0,
	ClientDisconnected = 0,	 // Same as Accepted, but for disconnect callback
	// MQTT 3.1.1 return codes
	// See Section 3.2.2.3 of
	// https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/mqtt-v3.1.1.html
	RefusedProtocolVersion = 1,
	RefusedIdentifierRejected = 2,
	RefusedServerUnavailable_v3 = 3,
	RefusedBadUsernameOrPassword_v3 = 4,
	RefusedNotAuthorized_v3 = 5,
	// MQTT 5.0 return codes
	// See Section 3.2.2.2 of
	// https://docs.oasis-open.org/mqtt/mqtt/v5.0/os/mqtt-v5.0-os.html
	RefusedUnspecifiedError = 128,
	RefusedMalformedPacket = 129,
	RefusedProtocolError = 130,
	RefusedImplementationSpecificError = 131,
	RefusedUnsupportedProtocolVersion = 132,
	RefusedClientIdentifierNotValid = 133,
	RefusedBadUserNameOrPassword_v5 = 134,
	RefusedNotAuthorized_v5 = 135,
	RefusedServerUnavailable_v5 = 136,
	RefusedServerBusy = 137,
	RefusedBanned = 138,
	RefusedBadAuthenticationMethod = 140,
	RefusedTopicNameInvalid = 144,
	RefusedPacketTooLarge = 149,
	RefusedQuotaExceeded = 151,
	RefusedPayloadFormatInvalid = 153,
	RefusedRetainNotSupported = 154,
	RefusedQosNotSupported = 155,
	RefusedUseAnotherServer = 156,
	RefusedServerMoved = 157,
	RefusedConnectionRateExceeded = 159
};

struct MqttSslConfig {
	const char* ca_file{nullptr};
	const char* cert_file{nullptr};
	const char* key_file{nullptr};
	// Doesn't handle password-protected keys
	bool verify_peer{true};
	bool verify_hostname{true};
};

class MqttClient {
   public:
	/**
	 * Callback type for connection events.
	 * Any value other than MqttConnectRc::Accepted indicates a failed
	 * connection.
	 */
	using ConnectCallback = std::function<void(MqttConnectRc rc)>;
	/**
	 * Callback type for disconnection events.
	 * The integer parameter is the result code: 0 for a clean disconnect,
	 * non-zero for an unexpected disconnect.
	 */
	using DisconnectCallback = std::function<void(MqttConnectRc rc)>;
	/**
	 * Callback type for when a message has been published to the broker.
	 * @param message_id The message ID.
	 */
	using PublishCallback = std::function<void(int message_id)>;
	/**
	 * Callback type for when a message has been received from the broker.
	 * @param message_id The message ID.
	 * @param topic The topic the message was received on.
	 * @param payload The message payload.
	 * @param qos The Quality of Service level of the message.
	 * @param retain Whether the message is a retained message.
	 */
	using MessageCallback = std::function<void(int message_id,
											   const char* topic,
											   std::string_view payload,
											   MqttQos qos,
											   bool retain)>;
	/** Callback type for when a subscription is acknowledged by the broker.
	 * @param message_id The message ID.
	 * @note The number of granted QoS levels is not provided.
	 */
	using SubscribeCallback = std::function<void(int message_id)>;
	/** Callback type for when an unsubscription is acknowledged by the broker.
	 * @param message_id The message ID.
	 */
	using UnsubscribeCallback = std::function<void(int message_id)>;

	virtual ~MqttClient();

	void configure(const char* client_id = nullptr,
				   const char* username = nullptr,
				   const char* password = nullptr,
				   const MqttSslConfig* ssl_config = nullptr);

	/**
	 * Connect to an MQTT broker.
	 * @param host The hostname or IP address of the broker.
	 * @param port The port number of the broker. Default is 1883.
	 * @param keepalive The keepalive interval in seconds. Default is 60.
	 * @throws std::runtime_error if the connection fails.
	 */
	void connect(const char* host,
				 int port = MQTT_DEFAULT_PORT,
				 int keepalive = 60);

	/**
	 * Connect to an MQTT broker, binding to a specific local address.
	 * @param host The hostname or IP address of the broker.
	 * @param bind_address The local address to bind to.
	 * @param port The port number of the broker. Default is 1883.
	 * @param keepalive The keepalive interval in seconds. Default is 60.
	 * @throws std::runtime_error if the connection fails.
	 */
	void connect(const char* host,
				 const char* bind_address,
				 int port = MQTT_DEFAULT_PORT,
				 int keepalive = 60);

	/**
	 * Disconnect from the MQTT broker.
	 */
	void disconnect();

	/**
	 * Subscribe to a topic.
	 * @note If the client is not currently connected, the subscription will be
	 * applied when the client connects.
	 * @param topic The topic to subscribe to.
	 * @param qos The Quality of Service level. Default is AtLeastOnce.
	 */
	void subscribe(const char* topic, MqttQos qos = MqttQos::AtLeastOnce);

	/**
	 * Unsubscribe from a topic.
	 * @note If the client is not currently connected, the resubscription will
	 * not be applied when the client connects.
	 * @param topic The topic to unsubscribe from.
	 */
	void unsubscribe(const char* topic);

	/**
	 * Publish a message to a topic.
	 * @param topic The topic to publish to.
	 * @param payload The message payload.
	 * @param qos The Quality of Service level. Default is AtLeastOnce.
	 * @param message_id Pointer to an integer to receive the message ID.
	 * @param retain Whether the message should be retained by the broker.
	 * Default is false.
	 * @return true if the message was successfully published, false if
	 * disconnected.
	 * @throws std::runtime_error if publishing fails for reasons other than
	 * being disconnected.
	 */
	bool publish(const char* topic,
				 std::string_view payload,
				 MqttQos qos = MqttQos::AtLeastOnce,
				 int* message_id = nullptr,
				 bool retain = false);

	/**
	 * Call regularly from the main thread to process pending events.
	 */
	virtual void loop() = 0;

	/**
	 * Check if the client is currently connected to the broker.
	 * @return true if connected, false otherwise.
	 * @note This will be called from the same thread that calls loop().
	 */
	virtual bool is_connected() const = 0;

	inline void set_connect_callback(
		ConnectCallback callback = ConnectCallback{}) {
		m_connect_callback = std::move(callback);
	}

	inline void set_disconnect_callback(
		DisconnectCallback callback = DisconnectCallback{}) {
		m_disconnect_callback = std::move(callback);
	}

	inline void set_publish_callback(
		PublishCallback callback = PublishCallback{}) {
		m_publish_callback = std::move(callback);
	}

	inline void set_message_callback(
		MessageCallback callback = MessageCallback{}) {
		m_message_callback = std::move(callback);
	}

	inline void set_subscribe_callback(
		SubscribeCallback callback = SubscribeCallback{}) {
		m_subscribe_callback = std::move(callback);
	}

	inline void set_unsubscribe_callback(
		UnsubscribeCallback callback = UnsubscribeCallback{}) {
		m_unsubscribe_callback = std::move(callback);
	}

   protected:
	/**
	 * Initialize the Mosquitto library.
	 * This should call mosquitto_lib_init() and return the result code in a
	 * thread-appropriate manner.
	 */
	virtual int lib_init() = 0;

	/**
	 * Hook called after configuration is done, but before any connection is
	 * made.
	 * Can be used to set callbacks or additional options on m_mosq.
	 */
	virtual void after_configure() {}

	mosquitto* m_mosq{nullptr};

	// Subscriptions to be (re)applied on reconnection
	std::array<std::vector<std::string>, 3> m_qos_subscriptions{};

	// Callbacks
	// These should only be called from the same thread that calls loop().
	// Setting a callback to an empty std::function will clear the callback.
	ConnectCallback m_connect_callback;
	DisconnectCallback m_disconnect_callback;
	PublishCallback m_publish_callback;
	MessageCallback m_message_callback;
	SubscribeCallback m_subscribe_callback;
	UnsubscribeCallback m_unsubscribe_callback;
};
