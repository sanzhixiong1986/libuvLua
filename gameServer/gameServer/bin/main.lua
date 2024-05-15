log_debug("HelloWorld")

-- ��ӡ�������
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

--�����������ݿ�Ĳ���
mysql_wrapper.connect("127.0.0.1", 3306, "games", "root", "sanzhixiong", function(err, context) 
	log_debug("event call");

	if(err)  then
		print(err)
		return
	end


	-- 
	mysql_wrapper.query(context, "select * from note", function (err, ret)
		if err then 
			print(err)
			return;
		end

		print("success")
		PrintTable(ret)
	end)
end)