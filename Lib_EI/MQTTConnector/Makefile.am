ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include -I$(top_srcdir)/Lib_EI/MosquittoCarrier
lib_LTLIBRARIES = libMQTTConnector.la
libMQTTConnector_la_SOURCES = MQTTConnectorEx.c MQTTConnector.c MQTTConnector.h
libMQTTConnector_la_LDFLAGS = -L$(top_srcdir)/Platform -L$(top_srcdir)/Lib_EI/MosquittoCarrier -lWISEPlatform -lMosquittoCarrier -ldl -release $(PACKAGE_VERSION_EI)
include_HEADERS = WISEConnector.h WISEConnectorEx.h

