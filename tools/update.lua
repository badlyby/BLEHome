local weather = {}

function node_add(addr)
  local len = #weather
  weather[len+1] = {addr=addr, pwr=nil, rssi=nil, temp=nil, humi=nil, pressure=nil}
  return len+1
end

function node_print(pipe)
  local file=io.open(pipe,"w")
  if file == nil then
    os.execute([[mkfifo ]]..pipe)
    file=io.open(pipe,"w")
  end
  if file then
    if weather then
      file:write([[return {]])
      for i=1,#weather do
        file:write([[{ addr="]]..weather[i].addr..[[", pwr=]]..weather[i].pwr..[[, rssi=]]..weather[i].rssi..[[, temp=]]..weather[i].temp..[[, humi=]]..weather[i].humi..[[, pressure=]]..weather[i].pressure..[[},]])
      end
      file:write([[}]]);
    end
    file:close()
  end
end

function node_find(addr)
  if weather == nil then
    weather = {}
  end
  for i=1,#weather do
    if weather[i].addr == addr then
      return i
    end
  end
  return node_add(addr)
end

function update_init()
end

function update_deinit()
end

function update_pwr(addr, pwr)
  local idx = node_find(addr)
  weather[idx].pwr = pwr
end

function update_rssi(addr, rssi)
  local idx = node_find(addr)
  weather[idx].rssi = rssi
end

function update_temp(addr, temp)
  local idx = node_find(addr)
  weather[idx].temp = temp
end

function update_humi(addr, humi)
  local idx = node_find(addr)
  weather[idx].humi = humi
end

function update_pressure(addr, pressure)
  local idx = node_find(addr)
  weather[idx].pressure = pressure
end

function do_update()
end

function update_cmd(cmd)
  if cmd == "get" then
    node_print([[/tmp/weather_home_out]])
  end
end

