#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include "settings.h"
#include "sip_client.h"
#include "access_client.h"
inline WebServer web(80);
inline String escHtml(const String &s){
 String r;for(size_t i=0;i<s.length();++i){char c=s[i];if(c=='&')r+="&amp;";else if(c=='<')r+="&lt;";else if(c=='>')r+="&gt;";else if(c=='"')r+="&quot;";else r+=c;}return r;
}
inline bool authorized(){
 if(settings.adminPassword.isEmpty()){web.send(503,"text/plain","Set admin password over USB serial provisioning before enabling WebGUI");return false;}
 if(web.authenticate("admin",settings.adminPassword.c_str()))return true;
 web.requestAuthentication(BASIC_AUTH,"ES3C28P","Admin login");return false;
}
inline void webStart(){
 web.on("/",HTTP_GET,[](){
  if(!authorized())return;
  String html="<html><meta name='viewport' content='width=device-width'><style>body{font:16px sans-serif;background:#121927;color:#eef;padding:20px;max-width:620px;margin:auto}input{display:block;width:100%;padding:9px;margin:5px 0 16px;box-sizing:border-box}button{padding:12px}</style><h2>ES3C28P SIP Doorbell</h2>";
  html+="<p>SIP: "+escHtml(sip.status)+"</p><p>Access: "+escHtml(accessClient.status)+"</p><form action='/save' method='POST'>";
  auto field=[&](const char*label,const char*key,const String &value,bool secret=false){
   html+="<label>"+String(label)+"</label><input name='"+key+"' value='"+(secret?String(""):escHtml(value))+"' "+(secret?"type='password' autocomplete='new-password'":"")+" maxlength='220'>";
  };
  field("Wi-Fi SSID","ssid",settings.ssid);
  field("Wi-Fi password (empty = unchanged)","wifi","");
  field("SIP server","siphost",settings.sipHost);
  field("SIP port","sipport",String(settings.sipPort));
  field("SIP account","sipuser",settings.sipUser);
  field("SIP password (empty = unchanged)","sippass","",true);
  field("SIP domain","sipdomain",settings.sipDomain);
  field("SIP destination","sipdst",settings.sipDestination);
  field("UniFi Access hostname or IP (without scheme)","achost",settings.accessHost);
  field("UniFi Access API token (empty = unchanged)","actoken","",true);
  field("UniFi status GET path (diagnostic only)","acpath",settings.accessStatusPath);
  html+="<button>Save configuration</button></form><p>Security: HTTP Basic Auth is LAN-only and not encrypted; use isolated management network.</p></html>";
  web.send(200,"text/html; charset=utf-8",html);
 });
 web.on("/save",HTTP_POST,[](){
  if(!authorized())return;
  // Validate and persist only known fields; never echo credentials.
  Preferences p;p.begin("doorbell",false);
  const char*keys[]={"ssid","wifi","siphost","sippass","sipuser","sipdomain","sipdst","achost","actoken","acpath"};
  const char*nvs[]={"ssid","wifi","siphost","sippass","sipuser","sipdomain","sipdst","achost","actoken","acpath"};
  for(size_t i=0;i<10;i++)if(web.hasArg(keys[i])){
   String v=web.arg(keys[i]);if(v.length()<=220 && (v.length()||strcmp(keys[i],"wifi")&&strcmp(keys[i],"sippass")&&strcmp(keys[i],"actoken")))p.putString(nvs[i],v);
  }
  if(web.hasArg("sipport")){int v=web.arg("sipport").toInt();if(v>0&&v<65536)p.putUShort("sipport",(uint16_t)v);}
  p.end();loadSettings();
  web.sendHeader("Location","/");web.send(303,"text/plain","");
 });
 web.on("/test-access",HTTP_POST,[](){if(!authorized())return;bool ok=accessClient.test();web.send(ok?200:503,"text/plain",accessClient.status);});
 web.begin();
}
