#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "settings.h"
// SIP UDP signaling MVP: unauthenticated registrar only.
// Digest authentication, SDP/RTP and inbound calls intentionally NOT claimed working.
class SipClient {
 WiFiUDP udp;
 uint32_t lastRegister=0;
 uint16_t cseq=1;
 bool started=false;
 String callId, branch;
 String makeTag(){return String((uint32_t)esp_random(),HEX);}
 void sendRegister(){
  if(settings.sipHost.isEmpty()||settings.sipUser.isEmpty())return;
  IPAddress target;
  if(!WiFi.hostByName(settings.sipHost.c_str(),target))return;
  branch="z9hG4bK"+makeTag();
  if(callId.isEmpty())callId=makeTag()+"@"+WiFi.localIP().toString();
  String domain=settings.sipDomain.isEmpty()?settings.sipHost:settings.sipDomain;
  String from="sip:"+settings.sipUser+"@"+domain;
  String msg="REGISTER sip:"+domain+" SIP/2.0\r\n";
  msg+="Via: SIP/2.0/UDP "+WiFi.localIP().toString()+":5062;branch="+branch+";rport\r\n";
  msg+="Max-Forwards: 70\r\n";
  msg+="From: <"+from+">;tag=es3c28p\r\nTo: <"+from+">\r\n";
  msg+="Call-ID: "+callId+"\r\nCSeq: "+String(cseq++)+" REGISTER\r\n";
  msg+="Contact: <"+from+">\r\nExpires: 120\r\n";
  msg+="User-Agent: ES3C28P-Doorbell/0.1\r\nContent-Length: 0\r\n\r\n";
  udp.beginPacket(target,settings.sipPort);udp.print(msg);udp.endPacket();
  Serial.println("[SIP] REGISTER sent; digest authentication not implemented");
 }
 public:
 String status="Nie skonfigurowano";
 void begin(){started=udp.begin(5062);lastRegister=millis()-120000UL;}
 void loop(){
  if(!started||WiFi.status()!=WL_CONNECTED)return;
  if(millis()-lastRegister>=120000UL){lastRegister=millis();sendRegister();}
  int n=udp.parsePacket();
  if(n>0){
   char buf[1024];int len=udp.read((uint8_t*)buf,sizeof(buf)-1);
   if(len>0){buf[len]=0;String message(buf);int firstEnd=message.indexOf("\r\n");
    status=message.substring(0,firstEnd>=0?firstEnd:len);
    Serial.printf("[SIP] %s\n",status.c_str());
   }
   while(udp.available())udp.read();
  }
 }
};
inline SipClient sip;
