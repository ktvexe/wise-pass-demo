/****************************************************************************/
/* Copyright(C) : Advantech Technologies, Inc.								*/
/* Create Date  : 2018/09/27 by Fred Chang								    */
/* Modified Date: 2018/09/27 by Fred Chang									*/
/* Abstract     : External Translator for Mosquitto Carrier API definition	*/
/* Reference    : None														*/
/****************************************************************************/

#ifndef _EXTERNEL_TRANSLATOR_H_
#define _EXTERNEL_TRANSLATOR_H_

void ET_AssignSolution(char *soln);

//Before publish
char *ET_PreTopicTranslate(const char* topic, const char *ref, char *buffer, int *len);
char *ET_PreMessageTranslate(const char* message, const char *ref, char *buffer, int *len);

//After recieve
char *ET_PostTopicTranslate(const char* topic, const char *ref, char *buffer, int *len);
char *ET_PostMessageTranslate(const char* message, const char *ref, char *buffer, int *len);

#endif