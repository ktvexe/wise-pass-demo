/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2016/03/01 by Scott Chang								    */
/* Modified Date: 2017/02/23 by Scott Chang									*/
/* Abstract     : WISE Core Test Application								*/
/* Reference    : None														*/
/****************************************************************************/
#include "network.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "WISECore.h"
#include "util_path.h"
#include "WISEPlatform.h"
#include "liteparse.h"

//Sensor data JSON format, it contain 3 sensor data: data1~3
#define SENSOR_DATA "{\"opTS\":{\"$date\":%lld},\"%s\":{\"%s\":{\"bn\":\"%s\",\"e\":[{\"n\":\"data1\",\"v\":%d},{\"n\":\"data2\",\"v\":%d},{\"n\":\"data3\",\"v\":%d},{\"n\":\"data4\",\"v\":%d},{\"n\":\"data5\",\"v\":%d},{\"n\":\"data6\",\"v\":%d},{\"n\":\"data7\",\"v\":%d},{\"n\":\"data8\",\"v\":%d},{\"n\":\"data9\",\"v\":%d},{\"n\":\"data10\",\"v\":%d},{\"n\":\"data11\",\"v\":%d},{\"n\":\"data12\",\"v\":%d},{\"n\":\"data13\",\"v\":%d},{\"n\":\"data14\",\"v\":%d}]}}}"

/*User can update g_strServerIP, g_iPort, g_strConnID, g_strConnPW and g_strDeviceID to connect to specific broker*/
char g_strServerIP[64] = "wise-msghub.eastasia.cloudapp.azure.com"; // MQTT broker URL or IP
int g_iPort = 1883; // MQTT broker listen port, keep 1883 as default port.
char g_strConnID[256] = "f1b2f600-fb24-4bee-a655-40a5394894d2:d77e6ba9-a635-4459-837e-2ffa0f3093d2"; //broker connection ID
char g_strConnPW[64] = "up6pf454k3r4po8m5h4j37f0gl"; //MQTT broker connection password
char g_strDeviceID[37] = "00000001-0000-0000-0000-305A3A77B120"; //Target device unique ID
char g_strMac[37] = "305A3A770020"; //Network MAC address
char g_strHostName[16] = "WISECoreSample"; //the HostName will show on renote server device list as device name, user can customize the host name.
char g_strProductTag[37] = "RMM"; // for common server the product tag default is "device", but user can change to their own product, such as "RMM", "SCADA"
char g_strTLCertSPW[37] = "05155853"; // SSL/TLS provate key or pre-shared-key

bool g_bReportData = true; // Report data flag, true: send sensor data after connected.
int g_iReportInterval = 60; //Send sensor data every 60 sec.
/* EnSaaS define each message size is 512 byte
 * Small Service: Support 10 devices and 3 million messages per month. For each device the data report frequency(g_iReportInterval) most not smaller then 10 second.
 * Medium Service: Support 100 devices and 50 million messages per month. For each device the data report frequency(g_iReportInterval) most not smaller then 6 second.
 * Large Service: Support 500 devices and 200 million messages per month. For each device the data report frequency(g_iReportInterval) most not smaller then 7 second.
 */
int g_iHeartbeatRate = 60; //Send heartbeat packet every min.
int g_iSensor[14] = {0}; //integer array for randomized sensor data

bool g_bConnected = false;
typedef struct
{
	int cmdID;
	char sessionID[64];
	char handlerName[64];
	bool bHasSessionID;
	char* pkt;
} getset_cmd;

//-------------------------Memory leak check define--------------------------------
#ifdef MEM_LEAK_CHECK
#include <crtdbg.h>
_CrtMemState memStateStart, memStateEnd, memStateDiff;
#endif
//---------------------------------------------------------------------------------
void SubscribeTopic();

#ifdef WIN32
#include "sys/time.h"
//function to get current timestamp 
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
 
    GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm. tm_isdst    = -1;
    clock = mktime(&tm);
    tv->tv_sec = (long)clock;
    tv->tv_usec = wtm.wMilliseconds * 1000;
 
    return (0);
}
#endif

//Callback function to get current timestamp.
long long get_timetick(void* userdata)
{
	long long tick = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tick = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
	return tick;
}

//function to parse recoved command
bool value_parse(const char* strCmd, const char* strTag, char* value, int length)
{
	char *contents = NULL; 
	char key[128] = {0};
	int len;
	bool bFound = false;
	
	if(!strCmd) return false;
	if(!strTag) return false;
	
	sprintf(key,"\"%s\":", strTag);

	contents = strstr(strCmd, key);
	if(contents>0)
	{
		int result = 2;
		char data[128] = {0};
		while(2== result){
			result = sscanf(contents, "\"%127[^\"]\":\"%127[^\"]\",%n", key, data, &len);
			if(result!=2)
				result = sscanf(contents, "\"%127[^\"]\":%127[^,],%n", key, data, &len);
			if(result!=2)
				break;
			if(!strcmp(key, strTag))
			{
				printf("%s=%s\n", key, data);
				memset(value, 0, length);
				strncpy(value, data, length);
				bFound = true;
				break;
			}
			contents += len;
			while(contents[0] ==' ')
				contents++;			
		}
	}
	return bFound;
}

//function to send snesor data capability in EI-PaaS handshake protocol
void sendCapability(long long curTime)
{
	char strBuffer[1024] = {0};
	char strTopic[256] = {0};
	char temp[512] = {0};
	sprintf(temp, SENSOR_DATA, curTime, "MySensor", "SensorGroup", "SensorGroup",
		g_iSensor[0], g_iSensor[1], g_iSensor[2], g_iSensor[3], g_iSensor[4],
		g_iSensor[5], g_iSensor[6], g_iSensor[7], g_iSensor[8], g_iSensor[9],
		g_iSensor[10], g_iSensor[11], g_iSensor[12], g_iSensor[13]);
	sprintf(strTopic, DEF_AGENTACT_TOPIC, g_strProductTag, g_strDeviceID);
	sprintf(strBuffer, DEF_ACTION_RESPONSE_JSON, g_strDeviceID, 2052, "general", temp, curTime); 
	core_publish(strTopic, strBuffer, strlen(strBuffer), 0, 0);
	printf("Capability send:\n [%s],\n %s\n", strTopic, strBuffer);
}

//function to send snesor data in EI-PaaS handshake protocol
void sendReportData(long long curTime)
{
	char strBuffer[1024] = {0};
	char strTopic[256] = {0};
	char temp[512] = {0};
	sprintf(temp, SENSOR_DATA, curTime, "MySensor", "SensorGroup", "SensorGroup",
		g_iSensor[0], g_iSensor[1], g_iSensor[2], g_iSensor[3], g_iSensor[4],
		g_iSensor[5], g_iSensor[6], g_iSensor[7], g_iSensor[8], g_iSensor[9],
		g_iSensor[10], g_iSensor[11], g_iSensor[12], g_iSensor[13]);
	sprintf(strTopic, DEF_AGENTREPORT_TOPIC, g_strDeviceID);
	sprintf(strBuffer, DEF_AUTOREPORT_JSON, g_strDeviceID, temp, curTime); //device ID
	core_publish(strTopic, strBuffer, strlen(strBuffer), 0, 0);
	printf("Data send:\n [%s],\n %s\n", strTopic, strBuffer);
}

//function to send response message in EI-PaaS handshake protocol
void sendResponse(int cmdID, char* handerlName, char* data, long long curTime)
{
	char strBuffer[1024] = {0};
	char strTopic[256] = {0};
#ifdef _WISEPAAS_02_DEF_H_
	sprintf(strTopic, DEF_AGENTACT_TOPIC, g_strProductTag, g_strDeviceID);
#else
	sprintf(strTopic, DEF_AGENTACT_TOPIC, g_strDeviceID);
#endif
	sprintf(strBuffer, DEF_ACTION_RESPONSE_JSON, g_strDeviceID, cmdID, handerlName, data, curTime); //device ID
	core_publish(strTopic, strBuffer, strlen(strBuffer), 0, 0);
	printf("Response send:\n [%s],\n %s\n", strTopic, strBuffer);
}

// Connect thread body
void* threadconnect(void* args)
{
	char strRecvTopic[256] = {0};
	long long nextHeartbeat = 0;
	long long nextReport = 0;
	core_device_register();

	printf("CB_Connected \n");
	SubscribeTopic();

	while(g_bConnected)
	{
		long long curTime = get_timetick(NULL);
		if(nextHeartbeat < curTime)
		{
			if(nextHeartbeat == 0)
				nextHeartbeat = curTime;
			nextHeartbeat += (g_iHeartbeatRate * 1000);
			core_heartbeat_send();
		}
		if(g_bReportData)
		{
			if(nextReport < curTime)
			{
				if(nextReport == 0)
					nextReport = curTime;
				nextReport += (g_iReportInterval * 1000);
				//send report data
				sendReportData(curTime);
			}
		}
		else
		{
			nextReport = 0;
		}
		usleep(100000);
	}

	pthread_exit(0);
	return NULL;
}

// Connected event callback function
void on_connect_cb(void* userdata)
{
	pthread_t conn = 0;
	g_bConnected = true;
	if(pthread_create(&conn, NULL, threadconnect, NULL)==0)
		pthread_detach(conn);
}

// Lostconnect event callback function
void on_lostconnect_cb(void* userdata)
{
	printf("CB_Lostconnect %s\n", core_error_string_get());
	g_bConnected = false;
	/*WISEConnector will reconnect automatically*/
}

// Disconnect event callback function
void on_disconnect_cb(void* userdata)
{
	printf("CB_Disconnect \n");
	g_bConnected = false;
}

// Sensor Get thread body to handle get command
void* threadget(void* args)
{
	char strBuffer[2048] = "{\"sensorInfoList\":{\"e\":[";
	char tmp[512] = {0};
	bool bFirst = true;
	char* p = strBuffer;
	getset_cmd* cmd = (getset_cmd*)args;
	if(cmd == NULL)
		goto GET_EXIT;
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data1"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data1\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[0]);
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data2"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data2\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[1]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data3"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data3\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[2]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data4"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data4\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[3]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data5"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data5\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[4]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data6"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data6\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[5]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data7"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data7\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[6]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data8"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data8\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[7]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data9"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data9\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[8]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data10"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data10\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[9]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data11"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data11\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[10]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data12"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data12\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[11]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data13"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data13\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[12]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data14"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data14\",\"v\":%d,\"StatusCode\": 200}", g_iSensor[13]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(cmd->bHasSessionID)
		sprintf(tmp,"]},\"sessionID\":\"%s\"}", cmd->sessionID);
	else
		strcpy(tmp,"]}}");
	p = strcat(p, tmp);
	sendResponse(cmd->cmdID, cmd->handlerName, p, get_timetick(NULL));
GET_EXIT:
	if(cmd)
	{
		if(cmd->pkt)
			free(cmd->pkt);
		free(cmd);
	}
	pthread_exit(0);
	return NULL;
}

// Data Set thread body to handle set command
void* threadset(void* args)
{
	char strBuffer[2048] = "{\"sensorInfoList\":{\"e\":[";
	char tmp[512] = {0};
	bool bFirst = true;
	char* p = strBuffer;
	char* target = NULL;
	getset_cmd* cmd = (getset_cmd*)args;
	if(cmd == NULL)
		goto SET_EXIT;
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data1")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[0] = atoi(tmp);
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data1\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data2")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[1] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data2\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data3")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[2] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data3\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data4")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[3] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data4\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data5")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[4] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data5\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data6")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[5] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data6\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data7")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[6] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data7\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data8")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[7] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data8\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data9")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[8] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data9\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data10")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[9] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data10\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data11")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[10] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data11\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data12")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[11] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data12\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data13")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[12] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data13\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if((target = strstr(cmd->pkt, "MySensor/SensorGroup/data14")) > 0)
	{
		memset(tmp, 0, sizeof(tmp));
		value_parse(target, "v", tmp, sizeof(tmp));
		g_iSensor[13] = atoi(tmp);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, "{\"n\": \"MySensor/SensorGroup/data14\",\"sv\":\"Success\",\"StatusCode\":200}");
		bFirst = false;
	}
	if(cmd->bHasSessionID)
		sprintf(tmp,"]},\"sessionID\":\"%s\"}", cmd->sessionID);
	else
		strcpy(tmp,"]}}");
	p = strcat(p, tmp);
	sendResponse(cmd->cmdID, cmd->handlerName, p, get_timetick(NULL));
SET_EXIT:
	if(cmd)
	{
		if(cmd->pkt)
			free(cmd->pkt);
		free(cmd);
	}
	pthread_exit(0);
	return NULL;
}

//Callback function to handle received command from server.
void on_msgrecv(const char* topic, const void *pkt, const long pktlength, void* userdata)
{
	int cmdID = 0;
	char sessionID[64] = {0};
	char handlerName[64] = {0};
	char tmp[64] = {0};
	bool bHasSessionID = false;
	printf("Packet received:\n [%s],\n %s\n", topic, pkt);
	/* All messages received from subscribed topics will trigger this callback function.
	 * topic: received topic.
	 * pkt: received message in json string.
	 * pktlength: received message length.
	 */
	if(!value_parse((char*)pkt, "handlerName", handlerName, sizeof(handlerName)))
		return;
	
	if(value_parse((char*)pkt, "sessionID", sessionID, sizeof(sessionID)))
		bHasSessionID = true;

	if(value_parse((char*)pkt, "commCmd", tmp, sizeof(tmp)))
	{
		cmdID = atoi(tmp);
	}
	else
		return;

	//command id 523 and 525 is defined in RMM product to get and set sensor data.
	//user cand customize the command id to trigger function.
	if(cmdID == wise_get_sensor_data_req)
	{
		/*TODO: Get Sensor Data*/
		pthread_t getthread = 0;
		getset_cmd* cmd = calloc(1, sizeof(getset_cmd));
		cmd->bHasSessionID = bHasSessionID;
		cmd->cmdID = wise_get_sensor_data_rep;
		strcpy(cmd->handlerName,handlerName);
		strcpy(cmd->sessionID,sessionID);
		cmd->pkt = strdup((char*)pkt);
		if(pthread_create(&getthread, NULL, threadget, cmd)==0)
			pthread_detach(getthread);
	}
	else if(cmdID == wise_set_sensor_data_req)
	{
		/*TODO: Set Sensor Data*/
		pthread_t setthread = 0;
		getset_cmd* cmd = calloc(1, sizeof(getset_cmd));
		cmd->bHasSessionID = bHasSessionID;
		cmd->cmdID = wise_set_sensor_data_rep;
		strcpy(cmd->handlerName,handlerName);
		strcpy(cmd->sessionID,sessionID);
		cmd->pkt = strdup((char*)pkt);
		if(pthread_create(&setthread, NULL, threadset, cmd)==0)
			pthread_detach(setthread);
	}	
}

//Callback function to handle rename command.
void on_rename(const char* name, const int cmdid, const char* sessionid, const char* devid, void* userdata)
{
	printf("rename to: %s\n", name);
	strcpy(g_strHostName, name);
	core_action_response(cmdid, sessionid, true, devid);
	return;
}

//Callback function to handle update command.
// the message brings the file transfer server's IP, Port, ID, Password file path and the md5.
// client can based on these data to download file.
void on_update(const char* loginID, const char* loginPW, const int port, const char* path, const char* md5, const int cmdid, const char* sessionid, const char* devid, void* userdata)
{
	printf("Update: %s, %s, %d, %s, %s\n", loginID, loginPW, port, path, md5);

	core_action_response(cmdid, sessionid, true, devid);
	return;
}

//Callback function to handle reconnect command.
void on_server_reconnect(const char* devid, void* userdata)
{
	if(!strcmp(g_strDeviceID, devid))
		core_device_register();
}

//Callback function to handle IoT get capability command.
//User can describe full capability of this device in IPSO json format.
void on_get_capability(const void *pkt, const long pktlength, const char* devid, void* userdata)
{
	/*TODO: send whole capability, no need on common server*/
	long long curTime = get_timetick(NULL);
	sendCapability(curTime);
}

// Callback function to handle IoT start report command
// parse the command to get the report interval and set to global variable
void on_start_report(const void *pkt, const long pktlength, const char* devid, void* userdata)
{
	/*TODO: start report sensor data*/
	char data[32] = {0};
	if(value_parse((char*)pkt, "autoUploadIntervalSec", data, sizeof(data)))
	{
		g_bReportData = true;
		g_iReportInterval = atoi(data); //sec.
	}	
}

// Callback function to handle IoT stop report command
// using a global flag to skip send data function (MQTT publish)
void on_stop_report(const void *pkt, const long pktlength, const char* devid, void* userdata)
{
	/*TODO: stop report sensor data*/
	g_bReportData = false;
}

// Callback function to handle heartbet rate query
void on_heartbeatrate_query(const char* sessionid, const char* devid, void* userdata)
{
	core_heartbeatratequery_response(g_iHeartbeatRate,sessionid, devid);
}

// Callback function to handle heartbet rate update command
void on_heartbeatrate_update(const int heartbeatrate, const char* sessionid, const char* devid, void* userdata)
{
	printf("Heartbeat Rate Update: %d, %s, %s\n", heartbeatrate, sessionid, devid);
	g_iHeartbeatRate = heartbeatrate;
	core_action_response(130/*wise_heartbeatrate_update_rep*/, sessionid, true, devid);
	return;
}

// subscribe a specific topic to receive the server command message.
void SubscribeTopic()
{
	char topic[256] = {0};
	
#ifdef _WISEPAAS_02_DEF_H_
	sprintf(topic, DEF_CALLBACKREQ_TOPIC, g_strProductTag, g_strDeviceID);
#else
	sprintf(topic, DEF_CALLBACKREQ_TOPIC, g_strDeviceID);
#endif
	core_subscribe(topic, 0);
}

// Sensor data access thread body.
// User can implement functions to access dirver or library to get sensor data.
void* threadaccessdata(void* args)
{
	srand((int) time(0)); //setup random seed.

	while(true)
	{
		int i=0;
		for(i=0; i<14; i++)
		{
			g_iSensor[i] += (rand() % 3) -1;
		}

		usleep(1000*1000);
	}
	pthread_exit(0);
	return NULL;
}

// Create a thread to access sensor data with your driver or library.
pthread_t StartAccessData()
{
	pthread_t thread = 0;
	if(pthread_create(&thread, NULL, threadaccessdata, NULL)!=0)
		thread = 0;
	return thread;
}

// Stop data access thread
void StopAccessData(pthread_t thread)
{
	if(thread != 0)
	{
		pthread_cancel(thread);
		pthread_join(thread, NULL);
	}
}

int main(int argc, char *argv[])
{
	pthread_t threaddataaccess = 0;
	int SSLMode = 0;  //0:disable, 1:CA Mode, 2: PSK Mode.
#ifdef MEM_LEAK_CHECK
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
	_CrtMemCheckpoint( &memStateStart);
#endif
	// Create a thread to access sensor data with your driver or library.
	threaddataaccess = StartAccessData();

	// Initialize WISECore and set "305A3A77B1CC" as identity.
	if(!core_initialize(g_strDeviceID, g_strHostName, g_strMac, NULL))
	{
		printf("Unable to initialize AgentCore.\n");
		goto EXIT;
	}
	printf("Agent Initialized\n");

	// Setup WISECore connection event callback function.
	core_connection_callback_set(on_connect_cb, on_lostconnect_cb, on_disconnect_cb, on_msgrecv);

	// Setup WISECore action command callback function.
	core_action_callback_set(on_rename, on_update);

	// Setup WISECore reconnect command callback function.
	core_server_reconnect_callback_set(on_server_reconnect);

	// Setup WISECore iot command callback function.
	core_iot_callback_set(on_get_capability, on_start_report, on_stop_report);

	// Setup WISECore timetick callback function to apply timestamp on reply message.
	core_time_tick_callback_set(get_timetick);

	// Setup WISECore heartbeat command callback function.
	core_heartbeat_callback_set(on_heartbeatrate_query, on_heartbeatrate_update);

	// Setup production tag let server can idebtify the agent type or supported function set.
	core_tag_set(g_strProductTag);

	// Setup basic agent product info. include: SN, parent ID (keep NULL if not any), software version, agent type (keep IPC if you don't know), product name and manufacturer name.
	core_product_info_set(g_strMac, NULL, "1.0.1", "IPC", "Sample", "Sample");

	// Setup WISECore connection SSL/TLS, 
	//   SSLMode=0 disable the SSL/TLS.
	//   SSLMode=1 certificate based SSL/TLS.
	//     If the server you are connecting to requires clients to provide a
	//     certificate, define certfile and keyfile with your client certificate and
	//     private key. If your private key is encrypted, provide a password callback
	//     function or you will have to enter the password at the command line.
	//   SSLMode=2  pre-shared-key based TLS.
	//      If the server you are connecting to provide a pre-shared-key, define the pre-shared-key and an ID with your client.
	//     private key.
	if(SSLMode == 1)
		core_tls_set( "server.crt", NULL, "ca.crt", "ca.key", g_strTLCertSPW);
	else if(SSLMode == 2)
		core_tls_psk_set(g_strTLCertSPW, g_strDeviceID, NULL);

	// connect to remote server(broker)
	if(!core_connect(g_strServerIP, g_iPort, g_strConnID, g_strConnPW)){
		printf("Unable to connect to broker. %s\n", core_error_string_get());
		goto EXIT;
	} else {
		printf("Connect to broker: %s\n", g_strServerIP);
	}
	
EXIT:
	printf("Click enter to exit\n");
	fgetc(stdin);

	// stop data access thread.
	StopAccessData(threaddataaccess);

	// disconnect from remote server
	core_disconnect(true);
	printf("Send Client Info: disconnect\n");

	// release WISECore library
	core_uninitialize();
#ifdef MEM_LEAK_CHECK
	_CrtMemCheckpoint( &memStateEnd );
	if ( _CrtMemDifference( &memStateDiff, &memStateStart, &memStateEnd) )
		_CrtMemDumpStatistics( &memStateDiff );
#endif

	return 0;
}

