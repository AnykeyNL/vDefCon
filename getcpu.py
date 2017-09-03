# Project vDefCon 
# 
# Pulling Realtime monitoring data directly from an ESXi server

import atexit
from pyVim import connect
from pyVmomi import vmodl
from pyVmomi import vim
import time

hostname="your ip "
username="root"
password="password"
disable_ssl_verification = True
port = 443

interval = 5

def makeconnection():
  global service_instance
  try:
        if disable_ssl_verification:
            service_instance = connect.SmartConnectNoSSL(host=hostname,
                                                         user=username,
                                                         pwd=password,
                                                         port=int(port))
        else:
            service_instance = connect.SmartConnect(host=hostname,
                                                    user=username,
                                                    pwd=password,
                                                    port=int(port))

        atexit.register(connect.Disconnect, service_instance)
        return 1

  except vmodl.MethodFault as error:
        print("Caught vmodl fault : " + error.msg)
        return -1

def getEsxtopCounters():
  try:
        services = service_instance.content.serviceManager.QueryServiceList(serviceName="Esxtop")
        if services:
            for service in services:
                if service.serviceName == "Esxtop":
                   results = service.service.ExecuteSimpleCommand(
                       arguments=["CounterInfo"])
                   return results



  except vmodl.MethodFault as error:
        print("Caught vmodl fault : " + error.msg)
        return -1
  return 0


def getEsxtopData():
  try: 
        services = service_instance.content.serviceManager.QueryServiceList(serviceName="Esxtop")
        if services:
            for service in services:
                if service.serviceName == "Esxtop":
                   results = service.service.ExecuteSimpleCommand(
                       arguments=["FetchStats"])
                   return results

  except vmodl.MethodFault as error:
        print("Caught vmodl fault : " + error.msg)
        return -1
  return 0

def parseCounterInfo(data, component):
  counters = counterinfo.split("\n")
  for counter in counters:
    elements = counter.split("|")
    if (len(elements)> 1):
      if (elements[1] == component):
        counterdetails  = elements

  return counterdetails


def parseInfo(data, component):
  info = []
  items = data.split("\n")
  for item in items:
   elements = item.split("|")
   #print (elements)
   if (len(elements) > 1):
    if elements[1] == component:
      info.append(elements)
  return info

def CalculatePercentage(UsedTime1, UsedTime2, Elapsed1, Elapsed2):
  DeltaUsed = UsedTime2 - UsedTime1
  DeltaElapsed = Elapsed2 - Elapsed1
  perc = float( ((DeltaUsed * 1000) / DeltaElapsed))
  return (perc/10)

  


#---------------------------------------------


if (makeconnection()):
  #counterinfo = getEsxtopCounters()

  prevdata = getEsxtopData()
  CpuCount = len( parseInfo(prevdata, "LCPU"))
  print ("CPU Count: {}". format(CpuCount))


  time.sleep(interval)
  while True:
 
   currentdata = getEsxtopData()

   lcpuinfo1 = []
   UsedTimeInUsec1 = []
   ElapsedTimeInUsec1 = []
   lcpuinfo2 = []
   UsedTimeInUsec2 = []
   ElapsedTimeInUsec2 = []
   p = []
 
   lcpuinfo1 = parseInfo(prevdata, "LCPU")
   lcpuinfo2 = parseInfo(currentdata, "LCPU")


   for c in range(0,CpuCount):
     UsedTimeInUsec1.append(long(lcpuinfo1[c][4]))
     ElapsedTimeInUsec1.append( long(lcpuinfo1[c][7]))

     UsedTimeInUsec2.append( long(lcpuinfo2[c][4]))
     ElapsedTimeInUsec2.append( long(lcpuinfo2[c][7]))

     p.append( CalculatePercentage(UsedTimeInUsec1[c], UsedTimeInUsec2[c], ElapsedTimeInUsec1[c], ElapsedTimeInUsec2[c]))

   txt = "CPU Used: "
   for c in range(0,CpuCount):
     txt = txt + str(p[c]) + " "
   txt = txt + " - Average: " + str(sum(p) / CpuCount) + "%"

   print (txt)

   prevdata = currentdata   
  
   time.sleep(interval)






 






