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

#define SENSOR_DATA "{\"opTS\":{\"$date\":%lld},\"%s\":{\"%s\":{\"bn\":\"%s\",\"e\":[{\"n\":\"data1\",\"v\":%d},{\"n\":\"data2\",\"v\":%d},{\"n\":\"data3\",\"v\":%d}]}}}"

char g_strServerIP[64] = "wise-msghub.eastasia.cloudapp.azure.com";
int g_iPort = 1883;
char g_strConnID[256] = "0e95b665-3748-46ce-80c5-bdd423d7a8a5:631476df-31a5-4b66-a4c6-bd85228b9d27";
char g_strConnPW[64] = "f3a2342t4oejbefc78cgu080ia";
char g_strDeviceID[37] = "00000001-0000-0000-0000-305A3A770020";

char g_strTenantID[37] = "general";
char g_strHostName[16] = "WISECoreSample";
char g_strProductTag[37] = "device";
char g_strTLCertSPW[37] = "05155853";

bool g_bReportData = true;
int g_iReportInterval = 60; //sec.
int g_iHeartbeatRate = 60; //min.
int g_iSensor[3] = {0};

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

long long get_timetick(void* userdata)
{
	long long tick = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tick = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
	return tick;
}

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

void sendReportData(long long curTime)
{
	char strBuffer[1024] = {0};
	char strTopic[256] = {0};
	char temp[512] = {0};
	sprintf(temp, SENSOR_DATA, curTime, "MySensor", "SensorGroup", "SensorGroup", g_iSensor[0], g_iSensor[1], g_iSensor[2]);
#ifdef _WISEPAAS_02_DEF_H_
	sprintf(strTopic, DEF_AGENTREPORT_TOPIC, g_strTenantID, g_strDeviceID);
#else
	sprintf(strTopic, DEF_AGENTREPORT_TOPIC, g_strDeviceID);
#endif
	sprintf(strBuffer, DEF_AUTOREPORT_JSON, g_strDeviceID, temp, curTime); //device ID
	core_publish(strTopic, strBuffer, strlen(strBuffer), 0, 0);
}

void sendResponse(int cmdID, char* handerlName, char* data, long long curTime)
{
	char strBuffer[1024] = {0};
	char strTopic[256] = {0};
#ifdef _WISEPAAS_02_DEF_H_
	sprintf(strTopic, DEF_AGENTACT_TOPIC, g_strTenantID, g_strDeviceID);
#else
	sprintf(strTopic, DEF_AGENTACT_TOPIC, g_strDeviceID);
#endif
	sprintf(strBuffer, DEF_ACTION_RESPONSE_JSON, g_strDeviceID, cmdID, handerlName, data, curTime); //device ID
	core_publish(strTopic, strBuffer, strlen(strBuffer), 0, 0);
}

void* threadconnect(void* args)
{
	char strRecvTopic[256] = {0};
	long long nextHeartbeat = 0;
	long long nextReport = 0;
	core_device_register();

	printf("CB_Connected \n");

	SubscribeTopic();
	while(true)
	{
		long long curTime = get_timetick(NULL);
		if(nextHeartbeat < curTime)
		{
			if(nextHeartbeat == 0)
				nextHeartbeat = curTime;
			nextHeartbeat += (g_iHeartbeatRate * 60000);
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
		usleep(100);
	}

	pthread_exit(0);
	return NULL;
}

void on_connect_cb(void* userdata)
{
	pthread_t conn = 0;
	if(pthread_create(&conn, NULL, threadconnect, NULL)==0)
		pthread_detach(conn);
}

void on_lostconnect_cb(void* userdata)
{
	printf("CB_Lostconnect %s\n", core_error_string_get());
	/*WISEConnector will reconnect automatically*/
}

void on_disconnect_cb(void* userdata)
{
	printf("CB_Disconnect \n");
}

void* threadget(void* args)
{
	char strBuffer[2048] = "{\"sensorIDList\":{\"e\":[";
	char tmp[512] = {0};
	bool bFirst = true;
	char* p = strBuffer;
	getset_cmd* cmd = (getset_cmd*)args;
	if(cmd == NULL)
		goto GET_EXIT;
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data1"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data1\",\"v\":%d}", g_iSensor[0]);
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data2"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data2\",\"v\":%d}", g_iSensor[1]);
		if(!bFirst)
			p = strcat(p, ",");
		p = strcat(p, tmp);
		bFirst = false;
	}
	if(strstr(cmd->pkt, "MySensor/SensorGroup/data3"))
	{
		sprintf(tmp, "{\"n\": \"MySensor/SensorGroup/data3\",\"v\":%d}", g_iSensor[2]);
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

void* threadset(void* args)
{
	char strBuffer[2048] = "{\"sensorIDList\":{\"e\":[";
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

	if(cmdID == 523)
	{
		/*TODO: Get Sensor Data*/
		pthread_t getthread = 0;
		getset_cmd* cmd = calloc(1, sizeof(getset_cmd));
		cmd->bHasSessionID = bHasSessionID;
		cmd->cmdID = cmdID+1;
		strcpy(cmd->handlerName,handlerName);
		strcpy(cmd->sessionID,sessionID);
		cmd->pkt = strdup((char*)pkt);
		if(pthread_create(&getthread, NULL, threadget, cmd)==0)
			pthread_detach(getthread);
	}
	else if(cmdID == 525)
	{
		/*TODO: Set Sensor Data*/
		pthread_t setthread = 0;
		getset_cmd* cmd = calloc(1, sizeof(getset_cmd));
		cmd->bHasSessionID = bHasSessionID;
		cmd->cmdID = cmdID+1;
		strcpy(cmd->handlerName,handlerName);
		strcpy(cmd->sessionID,sessionID);
		cmd->pkt = strdup((char*)pkt);
		if(pthread_create(&setthread, NULL, threadset, cmd)==0)
			pthread_detach(setthread);
	}	
}

void on_rename(const char* name, const int cmdid, const char* sessionid, const char* tenantid, const char* devid, void* userdata)
{
	printf("rename to: %s\n", name);
	strcpy(g_strHostName, name);
	core_action_response(cmdid, sessionid, true, tenantid, devid);
	return;
}

void on_update(const char* loginID, const char* loginPW, const int port, const char* path, const char* md5, const int cmdid, const char* sessionid, const char* tenantid, const char* devid, void* userdata)
{
	printf("Update: %s, %s, %d, %s, %s\n", loginID, loginPW, port, path, md5);

	core_action_response(cmdid, sessionid, true, tenantid, devid);
	return;
}

void on_server_reconnect(const char* tenantid, const char* devid, void* userdata)
{
	if(!strcmp(g_strDeviceID, devid))
		core_device_register();
}

void on_get_capability(const void *pkt, const long pktlength, const char* tenantid, const char* devid, void* userdata)
{
	/*TODO: send whole capability, no need on common server*/
	long long curTime = get_timetick(NULL);
	sendReportData(curTime);
}

void on_start_report(const void *pkt, const long pktlength, const char* tenantid, const char* devid, void* userdata)
{
	/*TODO: start report sensor data*/
	char data[32] = {0};
	if(value_parse((char*)pkt, "autoUploadIntervalSec", data, sizeof(data)))
	{
		g_bReportData = true;
		g_iReportInterval = atoi(data); //sec.
	}	
}

void on_stop_report(const void *pkt, const long pktlength, const char* tenantid, const char* devid, void* userdata)
{
	/*TODO: stop report sensor data*/
	g_bReportData = false;
}

void on_heartbeatrate_query(const char* sessionid,const char* tenantid,const char* devid, void* userdata)
{
	core_heartbeatratequery_response(g_iHeartbeatRate,sessionid, tenantid, devid);
}

void on_heartbeatrate_update(const int heartbeatrate, const char* sessionid, const char* tenantid, const char* devid, void* userdata)
{
	printf("Heartbeat Rate Update: %d, %s, %s\n", heartbeatrate, sessionid, devid);
	g_iHeartbeatRate = heartbeatrate;
	core_action_response(130/*wise_heartbeatrate_update_rep*/, sessionid, true, tenantid, devid);
	return;
}

void SubscribeTopic()
{
	char topic[256] = {0};
	
#ifdef _WISEPAAS_02_DEF_H_
	sprintf(topic, DEF_CALLBACKREQ_TOPIC, g_strTenantID, g_strProductTag, g_strDeviceID);
#else
	sprintf(topic, DEF_CALLBACKREQ_TOPIC, g_strDeviceID);
#endif
	core_subscribe(topic, 0);
}

void* threadaccessdata(void* args)
{
	srand((int) time(0)); //setup random seed.

	while(true)
	{
		//UpdateData("capability.txt", g_pCapability);
		int i=0;
		for(i=0; i<3; i++)
		{
			g_iSensor[i] += (rand() % 3) -1;
		}

		usleep(1000*1000);
	}
	pthread_exit(0);
	return NULL;
}

pthread_t StartAccessData()
{
	pthread_t thread = 0;
	if(pthread_create(&thread, NULL, threadaccessdata, NULL)!=0)
		thread = 0;
	return thread;
}

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
	
	threaddataaccess = StartAccessData();

	if(!core_initialize(g_strTenantID, g_strDeviceID, g_strHostName, "305A3A77B1CC", NULL))
	{
		printf("Unable to initialize AgentCore.\n");
		goto EXIT;
	}
	printf("Agent Initialized\n");

	core_connection_callback_set(on_connect_cb, on_lostconnect_cb, on_disconnect_cb, on_msgrecv);

	core_action_callback_set(on_rename, on_update);

	core_server_reconnect_callback_set(on_server_reconnect);

	core_iot_callback_set(on_get_capability, on_start_report, on_stop_report);

	core_time_tick_callback_set(get_timetick);

	core_heartbeat_callback_set(on_heartbeatrate_query, on_heartbeatrate_update);

	core_tag_set(g_strProductTag);

	core_product_info_set("305A3A77B1CC", NULL, "1.0.1", "IPC", "Sample", "Sample");

	if(SSLMode == 1)
		core_tls_set( "server.crt", NULL, "ca.crt", "ca.key", g_strTLCertSPW);
	else if(SSLMode == 2)
		core_tls_psk_set(g_strTLCertSPW, g_strDeviceID, NULL);

	if(!core_connect(g_strServerIP, g_iPort, g_strConnID, g_strConnPW)){
		printf("Unable to connect to broker. %s\n", core_error_string_get());
		goto EXIT;
	} else {
		printf("Connect to broker: %s\n", g_strServerIP);
	}
	
EXIT:
	printf("Click enter to exit\n");
	fgetc(stdin);

	StopAccessData(threaddataaccess);

	core_disconnect(true);
	printf("Send Client Info: disconnect\n");
	core_uninitialize();
#ifdef MEM_LEAK_CHECK
	_CrtMemCheckpoint( &memStateEnd );
	if ( _CrtMemDifference( &memStateDiff, &memStateStart, &memStateEnd) )
		_CrtMemDumpStatistics( &memStateDiff );
#endif

	return 0;
}

