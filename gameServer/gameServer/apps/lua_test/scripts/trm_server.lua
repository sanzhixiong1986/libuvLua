local session_set = {} -- 保存所有客户端的集合

-- 整个发布
function broadcast_except( msg,except_ssession )
	for i=1,#session_set do
		if except_ssession ~= session_set[i] then
			session.send_msg(session_set[i],msg)
		end
	end
end


-- 登录的返回的操作
function on_recv_login_cmd( s )
	-- body
	for i=1,#session_set do
		if s == session_set[i] then
			local msg = {1,2,0,{status=-1}}
			session.send_msg(s,msg)
			return
		end
	end

	table.insert(session_set,s)
	local msg = {1,2,0,{status=1}}
	session.send_msg(s,msg)

	local s_ip,s_port = session.get_address(s)
	msg = {1,7,0,{ip = s_ip, port = s_port}}
	broadcast_except(msg,s)
end

--退出操作
function on_recv_exit_cmd( s )
	-- body
	for i=1,#session_set do
		if s == session_set[i] then
			table.remove(session_set,i)
			local msg = {1,4,0,{status=1}}
			session.send_msg(s,msg)

			local s_ip,s_port = session.get_address(s)
			msg = {1,8,0,{ip=s_ip,port=s_port}}
			broadcast_except(msg,s)
			return
		end
	end
end


--收到消息
function on_recv_send_msg_cmd(s,str)
	for i = 1,#session_set do
		if s == session_set[i] then
			local msg = {1,6,0,{status=1}}
			session.send_msg(s,msg)

			local s_ip,s_port = session.get_address(s);
			msg = {1,9,0,{ip=s_ip,port=s_port,content=str}}
			broadcast_except(msg,s)
		end
	end

	local msg = {1,6,0,{status=-1}}
	session.send_msg(s,msg)
end


--获得数据
function on_trm_recv_cmd( s, msg )
	local ctype = msg[2]
	local body = msg[4]
	print("ctype====="..ctype)
	if ctype == 1 then
		on_recv_login_cmd(s)
	elseif ctype == 3 then
		on_recv_exit_cmd(s)
	elseif ctype == 5 then
		on_recv_send_msg_cmd(s,body.content)
	end
end

--客户端的session关闭
function on_trm_session_disconnect( s )
	local ip,port = session.get_address(s)
	for i=1,#session_set do
		if s == session_set[i] then
			print("remove from talk room: ".. ip .." : "..port)
			table.remove(session_set,i)
			local s_ip,s_port = session.get_address(s)
			local msg = {1,8,0,{ip=s_ip,port=s_port}}
			broadcast_except(msg,s)
		end
	end
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