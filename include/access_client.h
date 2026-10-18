#pragma once
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "settings.h"
// Read-only diagnostic access. No unverified door-control endpoints.
class AccessClient {
 public:
 String status="Brak konfiguracji";
 bool test(){
  if(settings.accessHost.isEmpty()||settings.accessStatusPath.isEmpty()){
   status="Ustaw host i sciezke GET w WebGUI";return false;
  }
  if(settings.accessStatusPath[0]!='/'||settings.accessStatusPath.indexOf("..")>=0){
   status="Niepoprawna sciezka";return false;
  }
  if(settings.accessHost.indexOf('/')>=0||settings.accessHost.indexOf('@')>=0){
   status="Niepoprawny host";return false;
  }
  // TLS verification must be provisioned before enabling network queries.
  status="Oczekuje konfiguracji zaufanego CA / certyfikatu TLS";
  return false;
 }
};
inline AccessClient accessClient;
