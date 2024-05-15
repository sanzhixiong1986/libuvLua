log_debug("HelloWorld")

key = ""
function PrintTable(str)
  
	log_debug(str)

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

redis_wrapper.connect("127.0.0.1", 6379, function (err, context)
  if err then 
    print(err)
    return
  end

  print("redis connect success")

  --redis_wrapper.close_redis(context);
  --[[
  redis_wrapper.query(context, "hmset 001001 name \"blake\" age \"34\"", function (err, result)
    if err then 
      print(err)
      return
    end
    print(result)
  end);
  ]]

  redis_wrapper.query(context, "get hello", function (err, result)
    if err then
      print(err)
      return
    end

    PrintTable(result)
  end)
end);