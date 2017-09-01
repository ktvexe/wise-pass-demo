ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include -I$(top_srcdir)/Lib_EI/MQTTConnector
lib_LTLIBRARIES = libWISECore.la
libWISECore_la_SOURCES = liteparse.c liteparse.h WISECoreEx.c WISECore.c WISECore.h
libWISECore_la_LDFLAGS = -L$(top_srcdir)/Platform -L$(top_srcdir)/Lib_EI/MQTTConnector -lWISEPlatform -lWISEUtil -lMQTTConnector -lpthread -ldl -release $(PACKAGE_VERSION_EI)
include_HEADERS = WISECore.h WISECoreEx.h
