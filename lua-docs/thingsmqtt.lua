--- @meta

--- @class ThingsMqtt
local ThingsMqtt = {}

function ThingsMqtt.new() end

function ThingsMqtt:connect() end

function ThingsMqtt:disconnect() end

function ThingsMqtt:publish_telemetry() end

function ThingsMqtt:publish_attribute() end

function ThingsMqtt:add_attribute_handler() end

function ThingsMqtt:remove_attribute_handler() end

function ThingsMqtt:add_rpc_handler() end

function ThingsMqtt:remove_rpc_handler() end

return ThingsMqtt
