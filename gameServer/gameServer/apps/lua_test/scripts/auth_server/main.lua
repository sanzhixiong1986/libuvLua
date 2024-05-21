--��ʼ����־ģ��
Logger.init("logger/auth_server/", "auth", true)
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
local servers = game_config.servers
local Stype = require("Stype")

-- �������ض˿ڼ���
Netbus.tcp_listen(servers[Stype.Auth].port)
print("Auth Server Start at ".. servers[Stype.Auth].port)
--Netbus.udp_listen(8002)
--end

local auth_service = require("auth_server/auth_service")
local ret = Service.register(Stype.Auth, auth_service)
if ret then 
  print("register Auth service:[" .. Stype.Auth.. "] success!!!")
else
  print("register Auth service:[" .. Stype.Auth.. "] failed!!!")
end