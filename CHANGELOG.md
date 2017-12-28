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
