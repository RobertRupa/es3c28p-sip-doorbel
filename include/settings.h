#pragma once
#include <Preferences.h>
#include <Arduino.h>
struct DeviceSettings {
 String ssid,password;
 String sipHost,sipUser,sipPassword,sipDomain,sipDestination;
 uint16_t sipPort=5060;
 String accessHost,accessToken,accessStatusPath;
 String adminPassword;
};
inline DeviceSettings settings;
inline void loadSettings(){
 Preferences p;p.begin("doorbell",true);
 settings.ssid=p.getString("ssid","");
 settings.password=p.getString("wifi","");
 settings.sipHost=p.getString("siphost","");
 settings.sipPort=p.getUShort("sipport",5060);
 settings.sipUser=p.getString("sipuser","");
 settings.sipPassword=p.getString("sippass","");
 settings.sipDomain=p.getString("sipdomain","");
 settings.sipDestination=p.getString("sipdst","");
 settings.accessHost=p.getString("achost","");
 settings.accessToken=p.getString("actoken","");
 settings.accessStatusPath=p.getString("acpath","");
 settings.adminPassword=p.getString("admin","");
 p.end();
}
