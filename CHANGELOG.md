# 1.0.15: Maintenance Release

## Feature

** [Changed]

* Platform

  > Fix memory-leak in network.c
  > Fix overwrite in util_os.c
  > Fix malloc size in basequeue
  > Update version to 1.0.11

# 1.0.14: Maintenance Release

## Feature

** [Changed]

* EI-Connect Library

  > Replace strtok with strtok_r for thread safe.
  > Add MAX_SESSION_LEN define to avoid sessionID overflow

* Platform

  > Modify GetSysLogonUserName to support Ubuntu 16.04
  > Update version to 1.0.10

# 1.0.13: Maintenance Release

## Feature

** [Changed]

* Platform

  > In wrapper, add strtoll define for DataSync
  > Update version to 1.0.9

# 1.0.12: Maintenance Release

## Feature

## [Added]

* WISECore Sample]

  > Add RMM3X pre-definitation to build client for WISE-PaaS 1.0 handshake protocol.

** [Changed]

* Platform

  > In netowrk, send WOL packet to all interfaces to local broadcast address
  > Update version to 1.0.8

# 1.0.11: Maintenance Release

## Feature

## [Added]

* EI-Connect Library

  > Add RMM3X pre-definitation to build client for WISE-PaaS 1.0 handshake protocol.

** [Changed]

* Platform

  > Fix xterm command in util_process_as_user_launch
  > Update version to 1.0.7

# 1.0.10: Maintenance Release

## Feature

## [Added]

* EI-Connect Library

  > Add android platform support.

** [Changed]

* MosquittoCarrierEx

  > Set mosquitto_tls_insecure_set to true to disable server check.

* WISECoreEx

  > Modify core_ex_initialize to pass cached mac string into MQTTConnector

* Platform/util_process

  > Modify GetSysLogonUserName for Ubuntu 16.04
  > Updat version to 1.0.6

* WISECoreSample 
  
  > Modify Get/Set response format.

## Bugs Fixed

* [#None]
  * [Install Shell]：Add libssl and libxtst into pre-install script.

# 1.0.9: Maintenance Release

## Feature

## Bugs Fixed

* [#None]
  * [WISECoreSample]：Fix capability message format.

# 1.0.8: Maintenance Release

## Feature
### [Modified]
  * MosquittoCarrier

  > Set reconnect interval to 10 sec.

# 1.0.7: Maintenance Release

## Feature
### [Modified]
  * 3rd Party library/Mosquitto

  > rename libmosquitto to mosquitto

  * MosquittoCarrier

  > re-link libmosquitto to mosquitto

  * MosquittoSample

  > re-link libmosquitto to mosquitto

# 1.0.6: Maintenance Release

## Feature
### [Modified]

  * MosquittoCarrier

  > Modify reconnect behavior
  > Modify PSK for insecure mode

  * WISECoreSample

  > Add comment for EnSaaS Service limitation and modify sample data format to meet the limitation.

  * MosquittoSample

  > Add comment for EnSaaS Service limitation and modify sample data format to meet the limitation.

# 1.0.5: Maintenance Release

## Feature
### [Modified]

  * WISECarrier

  > Set tls in insecure mode.

  * WISECoreSample

  > Modify WISECoreSample.

# 1.0.4: Maintenance Release

## Feature

## Bugs Fixed

* [#None]
  * [Platform]：Add wait result while execute util_process_as_user_launch  

# 1.0.3: Maintenance Release

## Feature
* ### [Removed]

  * WISECoreJNI

  >1. Move WISECoreJNI to EI-Connect-JNI project.

  * WISECore APIs

  >1. Remove address_get api.

# 1.0.2: Maintenance Release

## Feature
* ### [Added]
  * wisepaas pre-define

  >1.  Add common cmd enumeration.

* ### [Removed]
  * WISECore APIs, WISEConnector APIs and WISECarrier APIs

  >1. Remove get local IP api.
  >2. Remove TenantID from API and Topic.

* ### [Changed]
  * WISECoreSample

  >1. check received commcmd with common cmd enumeration.

## Bugs Fixed
* [#None]

        * [Sample Code]：Change heartbeat sleep time from 1 min. to 1sec in WISECoreSample.

# 1.0.1: Maintenance Release
 
## Feature
* ### [Changed]

    * Rebuild EI-Connect project
    
        >1\. Split Platform, Include, Library into subtree
        
## Bugs Fixed
* [#None]

        * [WISECore]：Fix Lostconnect error message

# 1.0.0: Maintenance Release
 
## Feature
* ### [Added]

    * EI-Connect First build
