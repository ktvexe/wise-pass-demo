ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include $(RMM3X)
lib_LTLIBRARIES = libMosquittoCarrier.la
libMosquittoCarrier_la_SOURCES = ExternalTranslator.c MosquittoCarrierEx.c MosquittoCarrier.c topic.c topic.h
libMosquittoCarrier_la_LDFLAGS = -L$(top_srcdir)/Platform -lWISEPlatform -lWISEUtil -lmosquitto -lpthread -ldl -release $(PACKAGE_VERSION_EI)
include_HEADERS = WiseCarrier_MQTT.h WiseCarrierEx_MQTT.h ExternalTranslator.h

