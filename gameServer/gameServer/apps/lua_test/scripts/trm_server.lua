--获得数据
function on_trm_recv_cmd( s, msg )
	-- body
end

--客户端的session关闭
function on_trm_session_disconnect( s )
	local ip,port = session.get_address(s)
	print("trm service on recv disconnect: ".. ip .." : "..port)
end

local trm_service = {
	on_session_recv_cmd = on_trm_recv_cmd,
	on_session_disconnect = on_trm_session_disconnect
}

local trm_server = {
	stype = 1,
	service = trm_service
}

return trm_server


