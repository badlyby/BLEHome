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

send_cmd("get")
node_read("/tmp/weather_home_out")

local m, s, o
m = Map("smart_home", translate("天气"), translate("显示气温，气压，湿度等信息"))
s = m:section(TypedSection, "smart_home", translate("传感器"))
s.anonymous = true
s.addremove = false

if weather then
  for i=1,#weather do
    s:tab(weather[i].addr,  translate("地址: "..weather[i].addr))
    if weather[i].pwr ~= nil then
      o = s:taboption(weather[i].addr, DummyValue, "pwr", translate("电量: "..weather[i].pwr.."%"))
    end
    if weather[i].rssi ~= nil then
      o = s:taboption(weather[i].addr, DummyValue, "rssi", translate("信号: "..weather[i].rssi.."dB"))
    end
    if weather[i].temp ~= nil then
      o = s:taboption(weather[i].addr, DummyValue, "temp", translate("温度: "..weather[i].temp.."℃"))
    end
    if weather[i].humi ~= nil then
      o = s:taboption(weather[i].addr, DummyValue, "humi", translate("湿度: "..weather[i].humi.."%"))
    end
    if weather[i].pressure ~= nil then
      o = s:taboption(weather[i].addr, DummyValue, "pressure", translate("气压: "..weather[i].pressure.."hPa"))
    end
  end
end

return m

