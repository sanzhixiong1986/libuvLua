
local session_set = {} -- �������пͻ��˵ļ���

function broadcast_except(msg, except_session)
	for i = 1, #session_set do 
		if  except_session ~= session_set[i] then 
			Session.send_msg(session_set[i], msg)
		end
	end
end

function on_recv_login_cmd(s)
	-- ��ǰ�Ƿ��Ѿ���������ϣ�����ǣ������Ѿ�����������ҵ���ʾ,
	for i = 1, #session_set do 
		if  s == session_set[i] then  -- ����״̬-1
			local msg = {1, 2, 0, {status = -1}}
			Session.send_msg(s, msg)
			return
		end
	end

	-- ���뵽��ǰ�ļ���, �������ݸ��ͻ���
	table.insert(session_set, s)
	local msg = {1, 2, 0, {status = 1}} -- ����״̬1��ʾ��½�ɹ���
	Session.send_msg(s, msg)
	--end 

	local s_ip, s_port = Session.get_address(s)
	msg = {1, 7, 0, {ip = s_ip, port = s_port}}
	broadcast_except(msg, s)
end

function on_recv_exit_cmd(s)
	for i = 1, #session_set do 
		if s == session_set[i] then  -- ����״̬-1
			table.remove(session_set, i)
			local msg = {1, 4, 0, {status = 1}} -- ����״̬1��ʾ�뿪�ɹ���
			Session.send_msg(s, msg)

			local s_ip, s_port = Session.get_address(s)
			msg = {1, 8, 0, {ip = s_ip, port = s_port}}
			broadcast_except(msg, s)
			return
		end
	end

	local msg = {1, 4, 0, {status = -1}} -- �뿪��ʱ��������������
	Session.send_msg(s, msg)
end

function on_recv_send_msg_cmd(s, str)
	for i = 1, #session_set do 
		if s == session_set[i] then  -- ����״̬-1
			local msg = {1, 6, 0, {status = 1}} -- ����״̬1��ʾ���ͳɹ���
			Session.send_msg(s, msg)

			local s_ip, s_port = Session.get_address(s)
			msg = {1, 9, 0, {ip = s_ip, port = s_port, content = str}}
			broadcast_except(msg, s)
			return
		end
	end

	local msg = {1, 6, 0, {status = -1}} -- ����״̬-1��ʾ����ʧ����
	Session.send_msg(s, msg)
end

-- {stype, ctype, utag, body}
function on_trm_recv_cmd(s, msg)
  local ctype = msg[2]
  local body = msg[4]
  if ctype == 1 then 
  	on_recv_login_cmd(s)
  elseif ctype == 3 then 
  	on_recv_exit_cmd(s)
  elseif ctype == 5 then  
  	on_recv_send_msg_cmd(s, body.content)
  end

end

function on_trm_session_disconnect(s, stype)
  local ip, port = Session.get_address(s)
  for i = 1, #session_set do 
		if  s == session_set[i] then  -- ����״̬-1
			print("remove from talk room: ".. ip .." : "..port)
			table.remove(session_set, i)
			local s_ip, s_port = Session.get_address(s)
			local msg = {1, 8, 0, {ip = s_ip, port = s_port}}
			broadcast_except(msg, s)
			return
		end
	end
  

end


local trm_service = {
  on_session_recv_cmd = on_trm_recv_cmd,
  on_session_disconnect = on_trm_session_disconnect,
}

local trm_server = {
  stype = 1,
  service = trm_service,
}

return trm_server;