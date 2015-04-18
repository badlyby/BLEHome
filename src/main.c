#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<math.h>
#include<lua.h>
#include<lauxlib.h>
#include"uart.h"
#include"init.h"

int fuart = -1;
int debug_level = 0;
int stillscan = 1;
lua_State *L;
int pipefp = -1;

typedef struct {
	unsigned char EventType;
	unsigned char AddrType;
	unsigned char Addr[6];
	unsigned char Rssi;
	unsigned char DataLength;
	unsigned char *Data;
}DeviceInformation;

typedef struct {
	unsigned char EventType;
	unsigned char AddrType;
	unsigned char Addr[6];
}DeviceList;

void printhex(char *head, char *data, int length)
{
	int i;
	printf("%s",head);
	for(i=0;i<length;i++)
	{
		if(i != 0) printf(" ");
		printf("%02X",data[i] & 0xFF);
	}
	printf("\n");
}

char half2hex(char dat)
{
	if((dat >= 0)&&(dat <= 9)) return dat+'0';
	if((dat >= 0x0a)&&(dat <= 0x0f)) return dat-0x0a+'A';
	return ' ';
}

void byte2hex(char *str, unsigned char *data, int length)
{
	int i;
	for(i=0;i<length;i++)
	{
		str[i*2] = half2hex((data[i] >> 4) & 0x0f);
		str[i*2+1] = half2hex(data[i] & 0x0f);
	}
}

void sendCommand(unsigned short Opcode, unsigned char *data, unsigned char length)
{
	unsigned char i,*buffer;
	buffer = (unsigned char *)malloc(length + 4);
	buffer[0] = 0x01;
	buffer[1] = Opcode & 0xFF;
	buffer[2] = (Opcode >> 8) & 0xFF;
	buffer[3] = length;
	if(length)
	{
		for(i=0;i<length;i++) buffer[i+4] = data[i];
	}
	write(fuart, buffer, length+4);
	free(buffer);
}

void sendEvent(unsigned char EventCode, unsigned char *data, unsigned char length)
{
	unsigned char i,*buffer;
	buffer = (unsigned char *)malloc(length + 3);
	buffer[0] = 0x04;
	buffer[1] = EventCode;
	buffer[2] = length;
	for(i=0;i<length;i++) buffer[i+3] = data[i];
	write(fuart, buffer, length+3);
	free(buffer);
}

void GAP_DeviceInit()
{
	unsigned char sbuf[38];
	unsigned char ProfileRole = 0x08;// Central
	unsigned char MaxScanRsps = 0x05;// 5
	unsigned char IRK[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned char CSRK[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned int SignCounter = 1;
	unsigned short Opcode = 0xFE00;// GAP_DeviceInit
	sbuf[0] = ProfileRole;
	sbuf[1] = MaxScanRsps;
	memcpy(sbuf+2, IRK, 16);
	memcpy(sbuf+18, CSRK, 16);
	memcpy(sbuf+34, (unsigned char *)&SignCounter, 4);
	sendCommand(Opcode, sbuf, 38);
}

void GAP_SetParam(unsigned char ParamID,unsigned short value)
{
	unsigned char sbuf[3];
	sbuf[0] = ParamID;
	memcpy(sbuf+1, &value, 2);
	sendCommand(0xFE30, sbuf, 3);
}

void GAP_GetParam(unsigned char ParamID)
{
	sendCommand(0xFE31, &ParamID, 1);
}

void GAP_DeviceDiscoveryRequest(unsigned char Mode, unsigned char ActiveScan, unsigned char WhiteList)
{
	unsigned char sbuf[3];
	sbuf[0] = Mode;
	sbuf[1] = ActiveScan;
	sbuf[2] = WhiteList;
	sendCommand(0xFE04, sbuf, 3);
}

void GAP_DeviceDiscoveryCancel()
{
	sendCommand(0xFE05, NULL, 0);
}

void GAP_HCI_ExtentionCommandStatus(unsigned char *data, unsigned char length)
{
	unsigned char Status;
	unsigned short OpCode;
	unsigned char DataLength;
	unsigned char *Data;
	unsigned short ParamValue;
	Status = data[0];
	memcpy((unsigned char*)&OpCode, data+1, 2);
	DataLength = data[3];
	Data = data+4;
	if(Status == 0)
	{
		if(debug_level > 1) printf("Success(%04X)\n",OpCode);
		if(DataLength > 0)
		{
			if(DataLength == 2)
			{
				memcpy((unsigned char*)&ParamValue, Data, 2);
				if(debug_level > 1)
				{
					printf("ParamValue: %04X(%d)\n", ParamValue, ParamValue);
				}
			}
			else
			{
				if(debug_level > 1)
				{
					printhex("Data: ", Data, DataLength);
				}
			}
		}
	}
}

void GAP_DeviceInitDone(unsigned char *data, unsigned char length)
{
	unsigned char i,Status;
	unsigned char DevAddr[6];
	unsigned short DataPktLen;
	unsigned char NumDataPkts;
	unsigned char IRK[16];
	unsigned char CSRK[16];
	Status = data[0];
	for(i=0;i<6;i++) DevAddr[5-i] = data[i+1];
	memcpy((unsigned char*)&DataPktLen, data+7, 2);
	NumDataPkts = data[9];
	memcpy(IRK, data+10, 16);
	memcpy(CSRK, data+26, 16);
	if(Status == 0)
	{
		if(debug_level > 1)
		{
			printf("Success\n");
			printhex("DevAddr: ",DevAddr,6);
			printf("DataPktLen: %d\n",DataPktLen);
			printf("NumDataPkts: %d\n",NumDataPkts);
			printhex("IRK: ",IRK,16);
			printhex("CSRK: ",CSRK,16);
		}
	}
}

void switchUserData(unsigned char *address, char rssi, unsigned short company_identifier, unsigned char *data, unsigned char length)
{
	int i,len;
	unsigned short uuid;
	float temp,humi;
	int pressure;
	double altitude;
	char addr[17],cmd[256];
	byte2hex(addr,address,6);
	addr[12] = 0;
	if(length > 2)
	{
		uuid = (data[1]<<8) | data[0];
		len = length - 2;
		if(len > 0)
		{
			switch(uuid)
			{
			case 0xAA00:
				if(debug_level > 0)
				{
					printhex("DevAddr: ",address,6);
					printf("电量: %d RSSI: %d\n", data[2], rssi);
				}
				sprintf(cmd,"update_pwr(\"%s\", %d)", addr, data[2]);
				luaL_dostring(L, cmd);
				sprintf(cmd,"update_rssi(\"%s\", %d)", addr, rssi);
				luaL_dostring(L, cmd);
				break;
			case 0xAA01:
				memcpy((unsigned char *)&temp, data+2, sizeof(float));
				if(debug_level > 0)
				{
					printf("温度: %.1f℃\n", temp);
				}
				sprintf(cmd,"update_temp(\"%s\", %.1f)", addr, temp);
				luaL_dostring(L, cmd);
				break;
			case 0xAA02:
				memcpy((unsigned char *)&humi, data+2, sizeof(float));
				if(debug_level > 0)
				{
					printf("湿度: %.1f%%\n", humi);
				}
				sprintf(cmd,"update_humi(\"%s\", %.1f)", addr, humi);
				luaL_dostring(L, cmd);
				break;
			case 0xAA03:
				memcpy((unsigned char *)&pressure, data+2, sizeof(int));
				if(debug_level > 0)
				{
					printf("气压: %.2fhPa\n", 1.0 * pressure/100);
					//altitude = 44330 * (1.0 - pow(1.0 * pressure / 101325, 0.1903));
					//printf("海拔: %.1f米\n", altitude);
				}
				sprintf(cmd,"update_pressure(\"%s\", %.2f)", addr, 1.0 * pressure/100);
				luaL_dostring(L, cmd);
				break;
			default:
				break;
			}
		}
	}
}

void unpackUserData(unsigned char *address, char rssi, unsigned short company_identifier, unsigned char *data, unsigned char length)
{
	int i = 0, j, len = 0;
	do {
		len = data[i];
		i++;
		switchUserData(address, rssi, company_identifier, data+i, len);
		i += len;
	} while(i<length - 1);
}

void unpackAdvData(unsigned char *address, char rssi, unsigned char *data, unsigned char length)
{
	unsigned char type;
	int j,len;
	unsigned short company_identifier;
	if(data[0] == 0xff)
	{
		company_identifier = (data[2]<<8) | data[1];
		type = data[3];
		if((type == 0x16) && (company_identifier == 0xFFF0))
		{
			len = data[4];
			unpackUserData(address, rssi, company_identifier, data+5, len);
		}
	}
}

void unpackScanRecord(DeviceInformation *info)
{
	int i = 0,j,len;
	unsigned char type;
	i++;
	len = info->Data[i++];
	type = info->Data[i];
	i += len;
	len = info->Data[i++];
	unpackAdvData(info->Addr, info->Rssi, info->Data+i, len);
}

void GAP_DeviceInformation(unsigned char *data, unsigned char length)
{
	char i;
	unsigned char Status;
	DeviceInformation dev;
	Status = data[0];
	dev.EventType = data[1];
	dev.AddrType = data[2];
	for(i=0;i<6;i++) dev.Addr[5-i] = data[i+3];
	dev.Rssi = data[9];
	dev.DataLength = data[10];
	dev.Data = data+11;
	if(Status == 0)
	{
		if(debug_level > 1)
		{
			printf("Success\n");
			printf("EventType: %02X\n",dev.EventType);
			printf("AddrType: %02X\n",dev.AddrType);
			printhex("Addr: ",dev.Addr,6);
			printf("Rssi: %d\n",dev.Rssi);
			printf("DataLength: %d\n",dev.DataLength);
			printhex("Data: ",dev.Data, dev.DataLength);
		}
		if((dev.EventType == 3) && (dev.AddrType == 1))
		{
			unpackScanRecord(&dev);
		}
	}
}

void GAP_DeviceDiscoveryDone(unsigned char *data, unsigned char length)
{
	unsigned char i,j;
	unsigned char Status;
	unsigned char NumDevs;
	DeviceList *list;
	Status = data[0];
	NumDevs = data[1];
	if(Status == 0)
	{
		if(debug_level > 1) printf("Success\n");
		list = (DeviceList *)malloc(sizeof(DeviceList)*NumDevs);
		for(i=0;i<NumDevs;i++)
		{
			list[i].EventType = data[i*8+2];
			list[i].AddrType = data[i*8+3];
			for(j=0;j<6;j++)
			{
				list[i].Addr[5-j] = data[i*8+4+j];
			}
			if(debug_level > 1)
			{
				printf("#%d:\n\tEventType: %02X\n\tAddrType: %02X\n", i, list[i].EventType, list[i].AddrType);
				printhex("\tAddr: ", list[i].Addr, 6);
			}
		}
		free(list);
		if(stillscan) GAP_DeviceDiscoveryRequest(0x03, 0x01, 0x00);
		luaL_dostring(L, "do_update()");
	}
}

void HCI_LE_ExtEvent(unsigned char *data, unsigned char length)
{
	unsigned short event;
	memcpy((unsigned char *)&event, data, 2);
	switch(event)
	{
	case 0x067F:
		GAP_HCI_ExtentionCommandStatus(data+2, length-2);
		break;
	case 0x0600:
		GAP_DeviceInitDone(data+2, length-2);
		break;
	case 0x060D:
		GAP_DeviceInformation(data+2, length-2);
		break;
	case 0x0601:
		GAP_DeviceDiscoveryDone(data+2, length-2);
		break;
	default:
		break;
	}
}

void getEvent(unsigned char EventCode, unsigned char *data, unsigned char length)
{
	switch(EventCode)
	{
	case 0xFF:
		HCI_LE_ExtEvent(data, length);
		break;
	default:
		break;
	}
}

void getCommand(unsigned short opcode, unsigned char *data, unsigned char length)
{
}

void readData()
{
	static char step = 0;
	static unsigned char type = 0;
	static unsigned char buffer[2048];
	static unsigned int bufcount = 0;
	static unsigned char scount = 0;
	static unsigned char length = 0;
	int i,len;
	unsigned char buf[256];
	unsigned short opcode;
	len = read(fuart, buf, 256);
	for(i=0;i<len;i++)
	{
		switch(step)
		{
		case 0:
			type = buf[i];
			bufcount = 0;
			scount = 0;
			buffer[bufcount++] = buf[i];
			step = 1;
			break;
		case 1:
			switch(type)
			{
			case 0x01:
				if(bufcount < 2048) buffer[bufcount++] = buf[i];
				scount++;
				if(scount >= 2)
				{
					scount = 0;
					step = 2;
				}
				break;
			case 0x04:
				if(bufcount < 2048) buffer[bufcount++] = buf[i];
				scount++;
				if(scount >= 1)
				{
					scount = 0;
					step = 2;
				}
				break;
			default:
				step = -1;
				break;
			}
			break;
		case 2:
			length = buf[i];
			buffer[bufcount++] = buf[i];
			scount = 0;
			step = 3;
			break;
		case 3:
			if(bufcount < 2048) buffer[bufcount++] = buf[i];
			scount++;
			if(scount >= length)
			{
				scount = 0;
				step = 0;
				if(debug_level > 2) printhex("read: ", buffer, bufcount);
				switch(type)
				{
				case 1:
					memcpy((unsigned char *)&opcode, buffer+1, 2);
					getCommand(opcode, buffer+4, buffer[3]);
					break;
				case 4:
					getEvent(buffer[1], buffer+3, buffer[2]);
					break;
				default:
					break;
				}
			}
			break;
		default:
			printf("Error: Unknow type!\n");
			usleep(500*1000);
			clean_uart(fuart);
			step = 0;
			return;
			break;
		}
	}
}

void readpipe()
{
	char cmd[256]="update_cmd(\"";
	int len;
	if(-1 == pipefp)
	{
		pipefp = open("/tmp/weather_home_in",O_RDONLY | O_NONBLOCK);
	}
	if(-1 != pipefp)
	{

		len = read(pipefp,cmd+12,200);
		if(len > 0)
		{
			cmd[len+12] = '"';
			cmd[len+13] = ')';
			cmd[len+14] = 0;
			if(debug_level > 1)
			{
				printf("#####%s\n", cmd);
			}
			luaL_dostring(L, cmd);
		}
	}
}

void readThread(int count)
{
	while(count)
	{
		readData();
		usleep(1000);
		readpipe();
		if(count > 0) count--;
	}
}

static void OnExit()
{
	if(-1 != fuart)
	{
		close(fuart);
		fuart = -1;
		luaL_dostring(L,"update_deinit()");
	}
	lua_close(L);
	if(-1 == pipefp)
	{
		close(pipefp);
	}
	system("rm /tmp/weather_home.pid");
}

static int print(lua_State *L)
{
 int n=lua_gettop(L);
 int i;
 for (i=1; i<=n; i++)
 {
  if (i>1) printf("\t");
  if (lua_isstring(L,i))
   printf("%s",lua_tostring(L,i));
  else if (lua_isnil(L,i))
   printf("%s","nil");
  else if (lua_isboolean(L,i))
   printf("%s",lua_toboolean(L,i) ? "true" : "false");
  else
   printf("%s:%p",luaL_typename(L,i),lua_topointer(L,i));
 }
 printf("\n");
 return 0;
}

static void rmfifo()
{
	system("rm /tmp/weather_home_in");
	system("rm /tmp/weather_home_out");
}

void mk_fifo()
{
	unlink("/tmp/weather_home_in");
	unlink("/tmp/weather_home_out");
	mkfifo("/tmp/weather_home_in", 0666);
	mkfifo("/tmp/weather_home_out", 0666);
	chmod("/tmp/weather_home_in", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	chmod("/tmp/weather_home_out", S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	atexit(rmfifo);
}

int main(int argc, char *argv[])
{
	if(debug_level == 0) init_daemon();
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_register(L,"print",print);
	mk_fifo();
	if(argc == 2)
	{
		fuart = set_uart(argv[1], 115200, 8, 1, 'N');
		if(-1 != fuart)
		{
			if (luaL_dofile(L,"/usr/share/weather_home/update.lua")!=0) fprintf(stderr,"%s\n",lua_tostring(L,-1));
			luaL_dostring(L, "update_init()");
			if (atexit(OnExit) != 0)
			{
				printf("can't register OnExit\n");
			}
			clean_uart(fuart);
			GAP_DeviceInit();
			GAP_GetParam(0x15);
			GAP_GetParam(0x16);
			GAP_GetParam(0x1A);
			GAP_GetParam(0x19);
			GAP_DeviceDiscoveryRequest(0x03, 0x01, 0x00);
			readThread(-1);
		}
	}
	system("rm /tmp/weather_home.pid");
}

