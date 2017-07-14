# EI-Connect
IoT/EI-PaaS Connection libraries for IoT/EI-PaaS client.

## DESCRIPTION
----

**EI-Connect** – the connection libraries to communicate between device and IoT/EI-PaaS Server.
Advantech provides a software development libraries to communicate and exchange information between target device and IoT/EI-PaaS Server, called EI-Connect.
The EI-Connect provides a rich set of user-friendly, intelligent and integrated interfaces, which speeds development, enhances security and makes agent application easier and simpler to communicate with IoT/EI-PaaS Server.
The framework has several benefits:

* Standardization - The communication protocol is based on the MQTT protocol to communicate and exchange data with IoT/EI-PaaS Server. The IoT sensor data report format is following the IPSO Spec. in JSON format.
* Portability - Whole framework is written in C language and follow the ANSI C Standard, that C compilers are available for most systems and are often the first compiler provided for a new system.


## OVERVIEW
----

 The EI-Connect includes:
 
 Library:
  * libWISECore.so
  	- Core library of EI-Connect to handshake with IoT/EI-PaaS Server.
 
  * libWISEConnector.so
  	- Manage the MQTT connection.
	 
Application:     
  * Sample\WISECoreSample:
  	- Lightweight agent sample application for both Windows and Linux.

  * Sample\MosquittoSample:
  	- Sample application, implement based on EI-PaaS handshake protocol with MQTT library,for both Windows and Linux.
     
Decuments:
	 
## PATENTS
----

## COMPILATION
----
 * Windows - Visual Studio 2008 
   1. Open "EI-connect.sln"
   2. In Solution Explore, Set "WISECoreSample" project as Startup Project
   3. Rebuild all solution by click build->Rebuild Solution
   4. The executable binary file will be generated in Release folder. 
   
 * Linux - CentOS
   1. Execute pre-install-centos.sh as a super user 
   2. Execute build-eiconnect.sh
   3. The executable binary file will be generated in Release folder. 
   
 * Linux - Ubuntu
   1. Execute pre-install-ubuntu.sh as a super user 
   2. Execute build-eiconnect.sh
   3. The executable binary file will be generated in Release folder. 
  
 * To compile this package under other Unix systems, user need install or pre-compile the following libraries:
   - openssl-1.0.1h
   - mosquitto-1.3.2
   - autoconf
   - automake
   - make
   - libx86 
   
## Hardware requirements
----

* CPU architecture
  - x86
  - ARM
 
## OS
----

 * Windows
   - XP, 7, 8, 10

 * Linux
   - Ubuntu ( 14.04.2 X64)
   - CentOS (6.5 X86)
   - Yocto
 
## PROBLEMS
----

## SUPPORT
----

 * [Advantech IoT Developer Forum](http://iotforum.advantech.com/)
 * [WIKI Documents](http://ess-wiki.advantech.com.tw/view/WISE-PaaS_2.0)
 
## License
----

Copyright (c) Advantech Corporation. All rights reserved.

