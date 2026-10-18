#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <lvgl.h>
#include <esp_timer.h>
#include <esp_system.h>
#include <esp_heap_caps.h>
#include "board_config.h"
#include "display.h"
#include "settings.h"
#include "sip_client.h"
#include "access_client.h"
#include "web_config.h"

static LGFX lcd;
static bool touchOk=false,codecOk=false;
static uint8_t *drawBuffer=nullptr;
static lv_obj_t *info=nullptr;
static uint32_t lastScreenUpdate=0;
static uint32_t lastWifiAttempt=0;
static void lvTick(void*){lv_tick_inc(1);}
static bool i2cPing(uint8_t address){Wire.beginTransmission(address);return Wire.endTransmission()==0;}
static bool touchRead(int32_t &x,int32_t &y){
 if(!touchOk)return false;
 Wire.beginTransmission(TOUCH_ADDR);Wire.write(0x02);
 if(Wire.endTransmission(false)!=0||Wire.requestFrom((uint8_t)TOUCH_ADDR,(uint8_t)1)!=1)return false;
 int n=Wire.read()&0x0f;if(n<1||n>2)return false;
 Wire.beginTransmission(TOUCH_ADDR);Wire.write(0x03);
 if(Wire.endTransmission(false)!=0||Wire.requestFrom((uint8_t)TOUCH_ADDR,(uint8_t)4)!=4)return false;
 uint8_t xh=Wire.read(),xl=Wire.read(),yh=Wire.read(),yl=Wire.read();
 x=constrain((int32_t)(LCD_W-1-(((xh&15)<<8)|xl)),0,LCD_W-1);
 y=constrain((int32_t)(LCD_H-1-(((yh&15)<<8)|yl)),0,LCD_H-1);
 return true;
}
static void lcdFlush(lv_display_t *d,const lv_area_t *a,uint8_t *px){
 const uint32_t w=a->x2-a->x1+1,h=a->y2-a->y1+1;
 lcd.startWrite();lcd.setAddrWindow(a->x1,a->y1,w,h);
 lcd.writePixels((lgfx::rgb565_t*)px,w*h);lcd.endWrite();
 lv_display_flush_ready(d);
}
static void touchCallback(lv_indev_t*,lv_indev_data_t *d){
 int32_t x=0,y=0;if(touchRead(x,y)){d->state=LV_INDEV_STATE_PRESSED;d->point.x=x;d->point.y=y;}
 else d->state=LV_INDEV_STATE_RELEASED;
}
static void createUi(){
 lv_obj_t *screen=lv_screen_active();lv_obj_set_style_bg_color(screen,lv_color_hex(0x111b2a),0);
 lv_obj_set_style_text_color(screen,lv_color_hex(0xe2e8f0),0);
 lv_obj_t *title=lv_label_create(screen);lv_label_set_text(title,"ES3C28P INTERCOM");lv_obj_align(title,LV_ALIGN_TOP_MID,0,12);
 info=lv_label_create(screen);lv_obj_set_width(info,215);lv_label_set_long_mode(info,LV_LABEL_LONG_WRAP);
 lv_obj_align(info,LV_ALIGN_TOP_LEFT,13,50);
 lv_obj_t *foot=lv_label_create(screen);lv_obj_set_width(foot,215);
 lv_label_set_text(foot,"SIP signaling MVP\nAudio/RTP: not ready\nH2: reserved GPIO43/44");
 lv_obj_align(foot,LV_ALIGN_BOTTOM_LEFT,13,-12);
}
static void updateUi(){
 if(!info)return;
 String text="Wi-Fi: "+String(WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():"offline");
 text+="\nCodec ES8311: "+String(codecOk?"found":"missing");
 text+="\nTouch FT6336: "+String(touchOk?"found":"missing");
 text+="\n\nSIP:\n"+sip.status;
 text+="\n\nUniFi Access:\n"+accessClient.status;
 lv_label_set_text(info,text.c_str());
}
static void provisionAdminFromSerial(){
 // First boot: provision admin secret locally over USB, never print secret in logs.
 static String line;
 while(Serial.available()){
  char ch=(char)Serial.read();
  if(ch=='\n'||ch=='\r'){
   line.trim();
   if(line.startsWith("ADMIN ")&&line.length()>=14){
    Preferences p;p.begin("doorbell",false);p.putString("admin",line.substring(6));p.end();
    loadSettings();Serial.println("[ADMIN] Password provisioned");
   }else if(line.startsWith("WIFI ")&&line.length()>6){
    int sep=line.indexOf(' ',5);
    if(sep>5){
     String ssid=line.substring(5,sep),pass=line.substring(sep+1);
     if(ssid.length()<=32 && pass.length()<=63){
      Preferences p;p.begin("doorbell",false);p.putString("ssid",ssid);p.putString("wifi",pass);p.end();
      loadSettings();WiFi.disconnect();WiFi.begin(settings.ssid.c_str(),settings.password.c_str());
      Serial.println("[WIFI] Credentials provisioned; connecting");
     }
    }
   }else if(line=="HELP")Serial.println("Commands: ADMIN <password> | WIFI <ssid> <password> (USB local only).");
   line="";
  }else if(line.length()<140)line+=ch;
 }
}
void setup(){
 Serial.begin(115200);
 delay(350);
 Serial.println("\nES3C28P SIP Doorbell v0.1 (signaling prototype)");
 loadSettings();
 Wire.begin(TOUCH_SDA,TOUCH_SCL);
 touchOk=i2cPing(TOUCH_ADDR);codecOk=i2cPing(CODEC_ADDR);
 lcd.init();lcd.setRotation(0);lcd.setBrightness(180);
 lv_init();
 drawBuffer=(uint8_t*)heap_caps_malloc(LCD_W*28*2,MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
 if(!drawBuffer){Serial.println("[FATAL] No DMA buffer");return;}
 lv_display_t *display=lv_display_create(LCD_W,LCD_H);
 lv_display_set_color_format(display,LV_COLOR_FORMAT_RGB565);
 lv_display_set_flush_cb(display,lcdFlush);
 lv_display_set_buffers(display,drawBuffer,nullptr,LCD_W*28*2,LV_DISPLAY_RENDER_MODE_PARTIAL);
 lv_indev_t *input=lv_indev_create();lv_indev_set_type(input,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(input,touchCallback);
 esp_timer_create_args_t args={};args.callback=lvTick;args.name="lv_tick";
 esp_timer_handle_t timer;ESP_ERROR_CHECK(esp_timer_create(&args,&timer));ESP_ERROR_CHECK(esp_timer_start_periodic(timer,1000));
 createUi();updateUi();
 WiFi.mode(WIFI_STA);
 if(settings.ssid.length())WiFi.begin(settings.ssid.c_str(),settings.password.c_str());
 sip.begin();
 webStart();
 Serial.println("[BOOT] WebGUI requires ADMIN password set through USB serial");
}
void loop(){
 provisionAdminFromSerial();
 if(WiFi.status()!=WL_CONNECTED && settings.ssid.length() && millis()-lastWifiAttempt>15000){
  lastWifiAttempt=millis();WiFi.reconnect();
 }
 sip.loop();web.handleClient();
 lv_timer_handler();
 if(millis()-lastScreenUpdate>1000){lastScreenUpdate=millis();updateUi();}
 delay(5);
}
