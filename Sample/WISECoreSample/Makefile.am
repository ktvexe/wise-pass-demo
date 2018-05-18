AM_CPPFLAGS = -Wall -fPIC -I$(top_srcdir)/Platform -I$(top_srcdir)/Platform/Linux/ -I$(top_srcdir)/Include -I$(top_srcdir)/Lib_EI/WISECore $(RMM3X)
bin_PROGRAMS = WISECoreSample
WISECoreSample_SOURCES = WISECoreSample.c
WISECoreSample_LDFLAGS = -L$(top_srcdir)/Platform -lWISEPlatform -lWISEUtil -L$(top_srcdir)/Lib_EI/WISECore -lWISECore -lpthread -lm -ldl -Wl,-rpath,./ 
#LDADD = $(INTLLIBS)
