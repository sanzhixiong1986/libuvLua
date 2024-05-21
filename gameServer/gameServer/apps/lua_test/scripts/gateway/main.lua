--��ʼ����־ģ��
Logger.init("logger/gateway/", "gateway", true)
--end

-- ��ʼ��Э��ģ��
local proto_type = {
    PROTO_JSON = 0,
    PROTO_BUF = 1,
}
ProtoMan.init(proto_type.PROTO_BUF)
-- �����protobufЭ�飬��Ҫע��һ��ӳ���
if ProtoMan.proto_type() == proto_type.PROTO_BUF then 
  local cmd_name_map = require("cmd_name_map")
  if cmd_name_map then 
    ProtoMan.register_protobuf_cmd_map(cmd_name_map)
  end
end
--end

local game_config = require("game_config")
-- �������ض˿ڼ���
Netbus.tcp_listen(game_config.gateway_tcp_port)
print("Tcp server listen At "..game_config.gateway_tcp_port)
Netbus.ws_listen(game_config.gateway_ws_port)
print("Ws server listen At "..game_config.gateway_ws_port)
--Netbus.udp_listen(8002)
--end

local servers = game_config.servers
local gw_service = require("gateway/gw_service")

for k, v in pairs(servers) do 
	local ret = Service.register_with_raw(v.stype, gw_service)
	if ret then 
  		print("register gw_servce:[" .. v.stype.. "] success!!!")
	else
  		print("register gw_servce:[" .. v.stype.. "] failed!!!")
	end
end