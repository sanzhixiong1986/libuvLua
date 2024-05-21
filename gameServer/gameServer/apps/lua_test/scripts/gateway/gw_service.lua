local game_config = require("game_config")

-- stype --> session的一个映射
local server_session_man = {}
-- 当前正在做连接的服务器
local do_connecting = {}

function connect_to_server(stype, ip, port)
	Netbus.tcp_connect(ip, port, function(err, session)
		do_connecting[stype] = false
		if err ~= 0 then 
			Logger.error("connect error to server ["..game_config.servers[stype].desic.."]"..ip..":"..port)
			return
		end 
		server_session_man[stype] = session
		print("connect success to server ["..game_config.servers[stype].desic.."]"..ip..":"..port)
	end)
end

function check_server_connect()
	for k, v in pairs(game_config.servers) do 
		if server_session_man[v.stype] == nil and 
		    do_connecting[v.stype] == false then 
		    do_connecting[v.stype] = true
		    print("connecting to server ["..v.desic.."]"..v.ip..":"..v.port)
		    connect_to_server(v.stype, v.ip, v.port)
		end
	end	
end

function gw_service_init()
	for k, v in pairs(game_config.servers) do 
		server_session_man[v.stype] = nil
		do_connecting[v.stype] = false
	end

	-- 启动一个定时器
	Scheduler.schedule(check_server_connect, 1000, -1, 5000)
	-- end 
end

-- {stype, ctype, utag, body}
function on_gw_recv_raw_cmd(s, raw_cmd)
end

function on_gw_session_disconnect(s) 
	-- 连接到服务器的session断线了
	if Session.asclient(s) then 
		for k, v in pairs(server_session_man) do 
			if v == s then 
				print("gateway disconnect ["..game_config.servers[k].desic.."]")
				server_session_man[k] = nil
				return
			end
		end
		return
	end

	-- 连接到网关的客户端断线了
end

gw_service_init()

local gw_service = {
	on_session_recv_raw_cmd = on_gw_recv_raw_cmd,
	on_session_disconnect = on_gw_session_disconnect,
}

return gw_service