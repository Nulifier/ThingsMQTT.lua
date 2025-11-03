--- @meta

--- @alias ThingsMqttSslConfig { ca_file: string, cert_file: string, key_file: string, verify_peer?: boolean, verify_hostname?: boolean }
--- @alias ThingsMqttConfig { host: string, port: integer?, bind_address: string?, keepalive: integer?, client_id: string?, username: string?, password: string?, ssl_config?: ThingsMqttSslConfig }

--- @class ThingsMqtt
local ThingsMqtt = {}

--- @return ThingsMqtt
function ThingsMqtt.new() end

--- Converts a Lua value to a JSON string.
--- @param val any The Lua value to convert.
--- @return string The JSON string representation of the Lua value.
function ThingsMqtt.json_stringify(val) end

--- Parses a JSON string into a Lua value.
--- @param str string The JSON string to parse.
--- @return any The Lua value represented by the JSON string.
function ThingsMqtt.json_parse(str) end

--- @param config ThingsMqttConfig
function ThingsMqtt:connect(config) end

function ThingsMqtt:disconnect() end

--- Checks if the client is connected to the MQTT broker.
--- @return boolean True if the client is connected, false otherwise.
function ThingsMqtt:is_connected() end

--- Main loop to be called periodically to process MQTT events.
--- @return nil
function ThingsMqtt:loop() end

--- Sets telemetry to send to server
--- @param key string name of the telemetry data
--- @param value any value of the telemetry data
function ThingsMqtt:telemetry(key, value) end

--- Sets an attribute to send to server
--- @param key string name of the attribute
--- @param value any value of the attribute
function ThingsMqtt:set_attribute(key, value) end

function ThingsMqtt:send() end

function ThingsMqtt:add_attribute_handler() end

function ThingsMqtt:remove_attribute_handler() end

--- @alias ThingsMqttRpcHandler fun(method: string, params: table): any

--- Adds a remote procedure call (RPC) handler.
--- @param handler ThingsMqttRpcHandler The RPC handler function.
--- @return integer The ID of the added RPC handler.
function ThingsMqtt:add_rpc_handler(handler) end

--- Removes a remote procedure call (RPC) handler.
--- @param handler_id integer The ID of the RPC handler to remove.
--- @return boolean True if the handler was removed, false otherwise.
function ThingsMqtt:remove_rpc_handler(handler_id) end

return ThingsMqtt
