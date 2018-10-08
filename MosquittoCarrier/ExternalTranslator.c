/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2018/09/27 by Fred Chang								    */
/* Modified Date: 2018/09/27 by Fred Chang									*/
/* Abstract     : External Translator for Azure								*/
/* Reference    : None														*/
/****************************************************************************/
#include <stdio.h>
#include "WISEPlatform.h"
#include "ExternalTranslator.h"

enum {
	RMM = 0,
	AZURE_PAAS = 1,
};

static int solution = RMM;


void ET_AssignSolution(char *soln) {
	if (soln == NULL) return;
	if (strncmp(soln, "Azure-PaaS=", strlen("Azure-PaaS=")) == 0) {
		solution = AZURE_PAAS;
	}
}

//Before publish
static void ToAzureTopic(char *orig, char *dest) {
	int len = strlen(orig);
	int i = 0;
	for (i = 0; i < len; i++) {
		if (orig[i] == '/') dest[i] = '[';
		else dest[i] = orig[i];
	}
}

#define AZURE_IOT_HUB_PREFIX "devices/%s/messages/events/["
char *ET_PreTopicTranslate(const char* topic, const char *ref, char *buffer, int *len) {
	if (buffer == NULL && len == NULL) return topic;
	switch (solution) {
		case AZURE_PAAS:
		{
			snprintf(buffer, *len, AZURE_IOT_HUB_PREFIX, ref);
			ToAzureTopic(topic, buffer + strlen(buffer));
			*len = strlen(buffer);
			buffer[*len] = ']';
			return buffer;
		}
		default:
			return topic;
	}
}
char *ET_PreMessageTranslate(const char* message, const char *ref, char *buffer, int *len) {
	if (buffer == NULL && len == NULL) return message;
	switch (solution) {
		case AZURE_PAAS:
		{
			return message;
		}
		default:
			return message;
	}
}

//After recieve
char *ET_PostTopicTranslate(const char* topic, const char *ref, char *buffer, int *len) {
	if (buffer == NULL && len == NULL) return topic;
	switch (solution) {
		case AZURE_PAAS:
		{
			if (NULL != strstr(topic, "messages/devicebound/%24.to=")) {
				char *s = strstr(ref, "\"topic\":\"");
				if (s != NULL) {
					char *e = NULL;
					s += 9;
					e = strchr(s, '\"');
					memcpy(buffer, s, (int)(e - s));
					*len = strlen(buffer);
					return buffer;
				}
			}
		}
		default:
			return topic;
	}
}
char *ET_PostMessageTranslate(const char* message, const char *ref, char *buffer, int *len) {
	if (buffer == NULL && len == NULL) return message;
	switch (solution) {
		case AZURE_PAAS:
		{
			if (NULL != strstr(ref, "messages/devicebound/%24.to=")) {
				char *s = strstr(message, "\"body\":{");
				if (s != NULL) {
					char *e = NULL;
					s += 7;
					e = strstr(s, "\"topic\":\"") - 3;
					*len = (int)(e - s) + 2;
					return s;
				}
			}
		}
		default:
			return message;
	}
}