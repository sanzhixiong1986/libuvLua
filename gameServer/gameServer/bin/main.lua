log_debug("HelloWorld")

key = ""
function PrintTable(table , level)
  level = level or 1
  local indent = ""
  for i = 1, level do
    indent = indent.."  "
  end

  if key ~= "" then
    print(indent..key.." ".."=".." ".."{")
  else
    print(indent .. "{")
  end

  key = ""
  for k,v in pairs(table) do
     if type(v) == "table" then
        key = k
        PrintTable(v, level + 1)
     else
        local content = string.format("%s%s = %s", indent .. "  ",tostring(k), tostring(v))
      print(content)  
      end
  end
  print(indent .. "}")

end

--[[
mysql_wrapper.connect("127.0.0.1", 3306, "class_sql", "root", "123456", function(err, context) 
	log_debug("event call");

	if(err)  then
		print(err)
		return
	end


	-- 
	mysql_wrapper.query(context, "select * from class_test", function (err, ret)
		if err then 
			print(err)
			return;
		end

		print("success")
		PrintTable(ret)
	end)
end)
]]

--[[
redis_wrapper.connect("127.0.0.1", 6379, function (err, context)
  if err then 
    print(err)
    return
  end

  print("redis connect success")

  --redis_wrapper.close_redis(context);

  redis_wrapper.query(context, "hgetall 002001", function (err, result)
    if err then
      print(err)
      return
    end

    PrintTable(result)
  end)
end);
]]

local my_service = {
  -- msg {1: stype, 2 ctype, 3 utag, 4 body_table_or_str}
  on_session_recv_cmd = function(session, msg)
    
  end,

  on_session_disconnect = function(session)
  end
}

local ret = service.register(100, my_service)
print("服务器注册是否成功")
print(ret)
