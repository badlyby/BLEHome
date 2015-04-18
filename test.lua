local weather = {}

function send_cmd(cmd)
  local file=io.open("/tmp/weather_home_in","w")
  if file then
    file:write(cmd)
    file:close()
  end
end

function node_read(pipe)
  local file=io.open(pipe,"r")
  local str = file:read()
  local dat = loadstring(str)
  file:close()
  if dat then
    weather = dat()
  end
end

function node_print()
  if weather then
    for i=1,#weather do
      print("地址: "..weather[i].addr.."\n电量: "..weather[i].pwr.."%\n信号: "..weather[i].rssi.."dB\n温度: "..weather[i].temp.."℃\n湿度: "..weather[i].humi.."%\n气压: "..weather[i].pressure.."hPa\n")
    end
  end
end

send_cmd("get")
node_read("/tmp/weather_home_out")
node_print()


