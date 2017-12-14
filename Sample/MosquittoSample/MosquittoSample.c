// MosquittoSample.cpp : Defines the entry point for the console application.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "mosquitto.h"
#include "util_path.h"
#include "WISEPlatform.h"
#include "wise/wisepaas_02_def.h"

//-------------------------Memory leak check define--------------------------------
#ifdef MEM_LEAK_CHECK
#include <crtdbg.h>
_CrtMemState memStateStart, memStateEnd, memStateDiff;
#endif
//---------------------------------------------------------------------------------
//Sensor data JSON format, it contain 3 sensor data: data1~3
#define SENSOR_DATA "{\"opTS\":{\"$date\":%lld},\"%s\":{\"%s\":{\"bn\":\"%s\",\"e\":[{\"n\":\"data1\",\"v\":%d},{\"n\":\"data2\",\"v\":%d},{\"n\":\"data3\",\"v\":%d},{\"n\":\"data4\",\"v\":%d},{\"n\":\"data5\",\"v\":%d},{\"n\":\"data6\",\"v\":%d},{\"n\":\"data7\",\"v\":%d},{\"n\":\"data8\",\"v\":%d},{\"n\":\"data9\",\"v\":%d},{\"n\":\"data10\",\"v\":%d},{\"n\":\"data11\",\"v\":%d},{\"n\":\"data12\",\"v\":%d},{\"n\":\"data13\",\"v\":%d},{\"n\":\"data14\",\"v\":%d}]}}}"

struct mosquitto *g_mosq = NULL;

/*User can update g_strServerIP, g_iPort, g_strConnID, g_strConnPW and g_strDeviceID to connect to specific broker*/
char g_strServerIP[64] = "172.22.12.89"; // MQTT broker URL or IP
int g_iPort = 1883; // MQTT broker listen port, keep 1883 as default port.
char g_strConnID[256] = "0e95b665-3748-46ce-80c5-bdd423d7a8a5:631476df-31a5-4b66-a4c6-bd85228b9d27"; //broker connection ID
char g_strConnPW[64] = "f3a2342t4oejbefc78cgu080ia"; //MQTT broker connection password
char g_strDeviceID[37] = "00000001-0000-0000-0000-305A3A770020"; //Target device unique ID
char g_strMac[37] = "305A3A770020"; //Network MAC address
char g_strHostName[16] = "MQTTSample"; //the HostName will show on renote server device list as device name, user can customize the host name.
char g_strProductTag[37] = "device"; // for common server the product tag default is "device", but user can change to their own product, such as "RMM", "SCADA"
char g_strTLCertSPW[37] = "05155853"; // SSL/TLS provate key or pre-shared-key

bool g_bReportData = true; // Report data flag, true: send sensor data after connected.
int g_iReportInterval = 60; //Send sensor data every 60 sec.
/* EnSaaS define each message size is 512 byte
 * Small Service: Support 10 devices and 3 million messages per month. For each device the data report frequency(g_iReportInterval) most not smaller then 10 second.
 * Medium Service: Support 100 devices and 50 million messages per month. For each device the data report frequency(g_iReportInterval) most not smaller then 6 second.
 * Large Service: Support 500 devices and 200 million messages per month. For each device the data report frequency(g_iReportInterval) most not smaller then 7 second.
 */
int g_iSensor[14] = {0}; //integer array for randomized sensor data
bool g_bConnected = false; //global flag for connection status
pthread_t g_reconnthr = 0;

long long GetTimeTick()
{
	long long tick = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	tick = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
	return tick;
}

void GenerateAgentInfo(char* strBuffer, int status)
{
	if(strBuffer == NULL)
		return;

	sprintf(strBuffer, DEF_AGENTINFO_JSON, 
			"", //Parent ID, keep empty if no parent
			g_strHostName, //target device name
			g_strMac, //serial number
			g_strMac, // MAC address
			"1.0.1", // software version
			"IPC", //agent type
			"MyTest", //Hardware product name
			"Advantech", //Hardware manufacture
			"anonymous", //binding account name
			"",//binding account password
			status,//connection status, 0:disconnect, 1: connect
			g_strProductTag, //supported WISE-PaaS products
			g_strDeviceID, //device ID
			GetTimeTick()); //time tick
}

// Sensor data access thread body.
// User can implement functions to access dirver or library to get sensor data.
void* threadaccessdata(void* args)
{
	while(true)
	{
		/*TODO: Get Data*/
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

// Connect thread body
void* threadconnect(void* args)
{
	char strTopic[256] = {0};
	char strBuffer[2048] = {0};
	struct mosquitto *mosq = NULL;
	long long curTime = 0;
	long long nextHeartbeat = 0;
	long long nextReport = 0;
	if(args == NULL)
	{
		pthread_exit(0);
		return NULL;
	}
	mosq = (struct mosquitto *) args;
	/*Send Connect Info*/
	sprintf(strTopic, DEF_INFOACK_TOPIC, g_strDeviceID);
	GenerateAgentInfo(strBuffer, 1);
	mosquitto_publish(mosq, NULL, strTopic, strlen(strBuffer), strBuffer, 0, false);

	/*Subscribe Command Topic*/
	sprintf(strTopic, DEF_CALLBACKREQ_TOPIC, g_strProductTag, g_strDeviceID);
	mosquitto_subscribe(mosq, NULL, strTopic, 0);

	printf("CB_Connected \n");

	while(g_bConnected)
	{
		curTime = GetTimeTick();
		if(nextHeartbeat == 0)
			nextHeartbeat = curTime;
		if(nextReport == 0)
			nextReport = curTime;
		/*Send heart beat*/
		if(nextHeartbeat<= curTime)
		{
			sprintf(strTopic, DEF_HEARTBEAT_TOPIC, g_strDeviceID);
			sprintf(strBuffer, DEF_HEARTBEAT_MESSAGE_JSON, g_strDeviceID); //device ID
			mosquitto_publish(mosq, NULL, strTopic, strlen(strBuffer), strBuffer, 0, false);
			printf("Heartbeat Send Topic:%s, Payload:%s \n", strTopic, strBuffer);
			nextHeartbeat += 60000; //60 sec.
		}

		/*Send report data*/
		if(g_bReportData)
		{
			if(nextReport<= curTime)
			{
				char temp[1024] = {0};
				sprintf(temp, SENSOR_DATA, curTime, "MySensor", "SensorGroup", "SensorGroup",
					g_iSensor[0], g_iSensor[1], g_iSensor[2], g_iSensor[3], g_iSensor[4],
					g_iSensor[5], g_iSensor[6], g_iSensor[7], g_iSensor[8], g_iSensor[9],
					g_iSensor[10], g_iSensor[11], g_iSensor[12], g_iSensor[13]);
				sprintf(strTopic, DEF_AGENTREPORT_TOPIC, g_strDeviceID);
				sprintf(strBuffer, DEF_AUTOREPORT_JSON, g_strDeviceID, temp, curTime); //device ID
				mosquitto_publish(mosq, NULL, strTopic, strlen(strBuffer), strBuffer, 0, false);
				printf("Data Send Topic:%s, Payload:%s \n", strTopic, strBuffer);
				nextReport += g_iReportInterval*1000; //sec.
			}
		}

		usleep(500*1000);
	}

	pthread_exit(0);
	return NULL;
}

// Reconnect thread body
void* thread_reconnect(void* args)
{
	struct mosquitto *mosq = NULL;
	int sleep_time = 30;
	if(args == NULL)
	{
		pthread_exit(0);
		return 0;
	}
	mosq = (struct mosquitto*)args;
	sleep_time += (10 - (rand() % 20));

	sleep(sleep_time);

	if(!g_bConnected)
		mosquitto_reconnect_async(mosq);

	pthread_exit(0);
	return 0;
}

// Connected event callback function
void on_connect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	if(g_reconnthr)
	{
		pthread_cancel(g_reconnthr);
		pthread_join(g_reconnthr, NULL);
		g_reconnthr = 0;
	}
	if(rc == 0)
	{
		pthread_t conn = 0;
		printf("Connected to broker: %s\n", g_strServerIP);
		g_bConnected = true;
		if(pthread_create(&conn, NULL, threadconnect, mosq)==0)
			pthread_detach(conn);
	}
	else
	{
		printf("Lostconnect from broker: %s\n", g_strServerIP);
		g_bConnected = false;
		if(pthread_create(&g_reconnthr, NULL, thread_reconnect, mosq)!=0)
			g_reconnthr = 0;				
	}
}

// Disconnect event callback function
void on_disconnect_callback(struct mosquitto *mosq, void *userdata, int rc)
{
	if(g_reconnthr)
	{
		pthread_cancel(g_reconnthr);
		pthread_join(g_reconnthr, NULL);
		g_reconnthr = 0;
	}
	g_bConnected = false;
	if(rc == 0)
	{
		printf("Disconnect from broker: %s\n", g_strServerIP);
	}
	else
	{
		printf("lostconnect from broker: %s\n", g_strServerIP);
		if(pthread_create(&g_reconnthr, NULL, thread_reconnect, mosq)!=0)
			g_reconnthr = 0;	
	}
}

//Callback function to handle received command from server.
void on_message_recv_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{	
	printf("Received topic: %s\n message: %s\n", msg->topic, (char*)msg->payload);

	//commCmd 523 and 525 is defined to get and set sensor data.
	//user cand customize the command id to trigger function.

	if(strstr((char*)msg->payload, "\"commCmd\":523")!=0)
	{
		/*TODO: Get Sensor Data*/
		printf("Get Sensor Data\n");
	}
	else if(strstr((char*)msg->payload, "\"commCmd\":525")!=0)
	{
		/*TODO: Set Sensor Data*/
		printf(" Set Sensor Data\n");
	}
}

// password check callback function to handle SSL/TLS private password 
int on_password_check(char *buf, int size, int rwflag, void *userdata)
{
	int length = 0;

	if(!buf)
		return 0;

	length = strlen(g_strTLCertSPW);

	memset(buf, 0, size);
	if(length+1 >= size)
	{
		strncpy(buf,g_strTLCertSPW,size);
		return size;
	}
	else
	{
		strncpy(buf, g_strTLCertSPW, length+1);
		return length;
	}
}

int main(int argc, char *argv[])
{
	pthread_t threaddataaccess = 0;
	int SSLMode = 0;  //0:disable, 1:CA Mode, 2: PSK Mode.
	struct mosquitto *mosq = NULL;
	char strWillTopic[260] = {0};
	char strWillPayload[1024] = {0};
	int iResult = MOSQ_ERR_SUCCESS;
#ifdef MEM_LEAK_CHECK
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
	_CrtMemCheckpoint( &memStateStart);
#endif
	srand((int) time(0)); //setup random seed.

	// Create a thread to access sensor data with your driver or library.
	threaddataaccess = StartAccessData();

	// initialize mosquitto library
	mosquitto_lib_init();

	// Create a modquitto handle
	mosq = mosquitto_new(g_strDeviceID, true, NULL);
	if (!mosq)
	{
		printf("Unable to create new mosquitto handle.\n");
		goto EXIT;
	}
	printf("Mosquitto initialized\n");

	/*setup callback function*/
	mosquitto_connect_callback_set(mosq, on_connect_callback);
	mosquitto_disconnect_callback_set(mosq, on_disconnect_callback);
	mosquitto_message_callback_set(mosq, on_message_recv_callback);
	//mosquitto_publish_callback_set(mosq, MQTT_publish_callback);


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
	{
		mosquitto_tls_insecure_set(mosq, true);
		mosquitto_tls_set(mosq, "server.crt", NULL, "ca.crt", "ca.key", on_password_check);
	}
	else if(SSLMode == 2)
	{
		mosquitto_tls_psk_set(mosq, g_strTLCertSPW, g_strDeviceID, NULL);
	}

	// Setup MQTT connection ID and Password
	if( strlen(g_strConnID)>0 && strlen(g_strConnPW)>0)
		mosquitto_username_pw_set(mosq, g_strConnID, g_strConnPW);

	/*reset will message*/
	mosquitto_will_clear(mosq);

	//Create MQTT Will message format and topic and register to broker
	sprintf(strWillTopic, DEF_WILLMSG_TOPIC, g_strDeviceID);
	GenerateAgentInfo(strWillPayload, 0);
	mosquitto_will_set(mosq, strWillTopic ,strlen(strWillPayload) ,strWillPayload, 0, false);
		
	/*start mosquitto thread to handle messages*/
	mosquitto_loop_start(mosq);
	/*mosquitto connect to specific server*/
	iResult = mosquitto_connect(mosq, g_strServerIP, g_iPort, 120);
	if(iResult != MOSQ_ERR_SUCCESS){
		printf("Unable to connect to broker. %s\n", mosquitto_strerror(iResult));
		goto EXIT;
	} else {
		printf("Connect to broker: %s\n", g_strServerIP);
	}
	
EXIT:
	printf("Click enter to exit\n");
	fgetc(stdin);

	// stop data access thread.
	StopAccessData(threaddataaccess);

	if(mosq)
	{
		char strTopic[260] = {0};
		/*Send Disconnect Info*/
		sprintf(strTopic, DEF_INFOACK_TOPIC, g_strDeviceID);
		mosquitto_publish(mosq, NULL, strTopic, strlen(strWillPayload), strWillPayload, 0, false);

		/*flush message queue*/
		mosquitto_loop(mosq, 0, 1);	
		/*send mqtt disconnect message for broker*/
		mosquitto_disconnect(mosq);
		/*make sure the disconnect message is published*/
		mosquitto_loop(mosq, 0, 1);	
		/*stop mosquitto thread*/
		mosquitto_loop_stop(mosq, false); //disable force for linux.
		printf("Send Client Info: disconnect\n");
		
		/*reset callback function*/
		mosquitto_connect_callback_set(mosq, NULL);
		mosquitto_disconnect_callback_set(mosq, NULL);
		mosquitto_message_callback_set(mosq, NULL);
		mosquitto_publish_callback_set(mosq, NULL);

		/*release mosquiotto handle*/
		mosquitto_destroy(mosq);
		mosq = NULL;
	}
	/*release mosquitto library*/
	mosquitto_lib_cleanup();
#ifdef MEM_LEAK_CHECK
	_CrtMemCheckpoint( &memStateEnd );
	if ( _CrtMemDifference( &memStateDiff, &memStateStart, &memStateEnd) )
		_CrtMemDumpStatistics( &memStateDiff );
#endif

	return 0;
}
