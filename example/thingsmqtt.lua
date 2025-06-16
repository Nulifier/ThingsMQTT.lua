local thingsmqtt = require("thingsmqtt")

local config = {
	host = "thingsboard.lan.sunk.io",
	port = 8883,
	-- bind_address = "0.0.0.0"
	-- keepalive = 60,
	-- client_id = "device_0001"
	-- username = "admin"
	-- password = "hunter123"
	ssl_config = {
		ca_file = "/workspaces/ThingsMQTT.lua/example/ca-chain.cert.pem",
		cert_file = "/workspaces/ThingsMQTT.lua/example/epsc-heat-000001.cert.pem",
		key_file = "/workspaces/ThingsMQTT.lua/example/epsc-heat-000001.key.pem"
	}
}

local lme_thing = thingsmqtt.new()

lme_thing:connect(config)

lme_thing:telemetry("lme_phase", 124)

print("Connecting...")

while not lme_thing:is_connected() do
	lme_thing:loop()
end

print("Connected!")

lme_thing:send()

while true do
	lme_thing:loop()
end
