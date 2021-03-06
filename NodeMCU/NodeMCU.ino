#include <SerialCommand.h>
SerialCommand sCmd;
/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on NodeMCU.

  Note: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right NodeMCU module
  in the Tools -> Board menu!

  For advanced settings please follow ESP examples :
   - ESP8266_Standalone_Manual_IP.ino
   - ESP8266_Standalone_SmartConfig.ino
   - ESP8266_Standalone_SSL.ino

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "69D2rgmzWlq78jEIXrZvUEhn4Q6KUlug";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Toan Van";
char pass[] = "hoilamchi";
// khai bao
#define temp V10
#define ON_H V0
#define ON_M V1
#define ON_S V2
#define OFF_H V3
#define OFF_M V4
#define OFF_S V5
#define SAVE V6
#define BTN1 V7
#define BTN2 V8
#define terminal V9
#define dev1 0
#define dev2 2
//#define nut_nhan_tay_den 14
//#define nut_nhan_tay_guong 12
WidgetLED st_den(V10);
WidgetLED st_guong(V11);
int return_st_esp , STT_OXI , TIMER_OXI;
unsigned long time_get , time_reset;
unsigned int TEMP_, da_hien_thi , st_btn_den , st_btn_guong , st_dev , run_time , stop_time , on_hours , on_minutes , on_seconds , off_hours , off_minutes
, off_seconds  , BIT_SAVE , time_count , time_on , time_off , RUN, BAT_DEN , BAT_DAU , pushed , da_nhan, da_thuc_thi, da_nhan_btn_den, da_nhan_btn_guong;
BLYNK_WRITE(BTN1) {
  pushed = param.asInt();
  if (pushed == 1 && da_nhan == 0 && BAT_DAU == 0 && run_time != NULL && stop_time != NULL)
  {
    Serial.println("OXI 1");
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "----TR???NG TH??I OXI---");
    Blynk.virtualWrite(terminal, "???? K??CH HO???T CH???Y T??? ?????NG");
    BAT_DAU += 1;
    da_nhan = 1;
  }
  if (pushed == 1 && da_nhan == 0 && BAT_DAU == 1 && run_time != NULL && stop_time != NULL)
  {
    Serial.println("OXI 0");
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "----TR???NG TH??I OXI---");
    Blynk.virtualWrite(terminal, "D???NG K??CH HO???T CH???Y T??? ?????NG");
    BAT_DAU += 1;
    da_nhan = 1;
  }

  if (BAT_DAU >= 2 && da_nhan == 0)
  {

    BAT_DAU = 0;
    da_nhan = 1;
  }
  if (pushed == 0 && da_nhan == 1)
  {
    da_nhan = 0;
  }
  else if ( pushed == 0 && run_time == 0 | stop_time == 0)
  {
    Blynk.virtualWrite(terminal, "* NH???C L???I : Y??U C???U CUNG C???P GI?? TR??? TH???I GIAN ????? B???T ?????U CH???Y");
  }

}
BLYNK_WRITE(ON_H) {
  on_hours = param.asInt();

}
BLYNK_WRITE(ON_M) {
  on_minutes = param.asInt();

}
BLYNK_WRITE(ON_S) {
  on_seconds = param.asInt();

}
BLYNK_WRITE(OFF_H) {
  off_hours = param.asInt();

}
BLYNK_WRITE(OFF_M) {
  off_minutes = param.asInt();

}
BLYNK_WRITE(OFF_S) {
  off_seconds = param.asInt();

}
BLYNK_WRITE(SAVE) {
  BIT_SAVE = param.asInt();
  if (BIT_SAVE == 1)
  {
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "TH???I GIAN C??I ?????T ???? L??U");
    Blynk.virtualWrite(terminal, " * TH???I GIAN CH???Y");
    Blynk.virtualWrite(terminal, "    " + String(on_hours) + " : " + String(on_minutes) + " : " + String(on_seconds));
    Blynk.virtualWrite(terminal, " * TH???I GIAN D???NG");
    Blynk.virtualWrite(terminal, "    " + String(off_hours) + " : " + String(off_minutes) + " : " + String(off_seconds));
    Blynk.virtualWrite(terminal, "----------------------------");
    run_time = (on_hours * 3600) + (on_minutes * 60) + on_seconds;
    stop_time = (off_hours * 3600) + (off_minutes) * 60 + off_seconds;
    Serial.println("RUN_TIME " + String(run_time));
    delay(500);
    Serial.println("STOP_TIME " + String(stop_time));
  }
}
int arduino_err;
unsigned long timer , time_ping;
void setup()
{
  pinMode(D0, OUTPUT);
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8080);
  Serial.begin(9600);
  time_reset = millis();
  time_ping = millis();
  sCmd.addCommand("START_STOP", BTN_START_STOP);
  sCmd.addCommand("on_r", SAVE_ROTARY_ON_TIME);
  sCmd.addCommand("off_r", SAVE_ROTARY_OFF_TIME);
  sCmd.addCommand("TEMP", GET_TEMP);
  sCmd.addCommand("ALLOW_OXI", MONITOR_ST_OXI );
  sCmd.addCommand("TIME_OXI", MONITOR_TIME_OXI );
  sCmd.addCommand("ARDUINO_CALL", RETURN_STATUS );

  //Blynk.begin(auth, ssid, pass);
  // You can also specify server:

  //  pinMode(dev1 , OUTPUT);
  //  pinMode(dev2 , OUTPUT);
  //  pinMode(nut_nhan_tay_den, INPUT_PULLUP);
  //  pinMode(nut_nhan_tay_guong, INPUT_PULLUP);
  Blynk.virtualWrite(terminal, "----------NH?? T???O 3 XIN CH??O !--------");
  Blynk.virtualWrite(terminal, "*L??U ?? 1");
  Blynk.virtualWrite(terminal, "????? ti???n h??nh l??u th???i gian vui l??ng k??o t???t c??? c??c thanh th???i gian theo y??u c???u , sau ???? nh???n 'SAVE' ????? l??u th???i gian ho???t ?????ng c???a OXI");
  Blynk.virtualWrite(terminal, "*L??U ?? 2");
  Blynk.virtualWrite(terminal, "*Th???i gian c??i ?????t b??n d?????i s??? ch???y theo v??ng l???p khi ch???y OXI. NH?? T???O 3 XIN C???M ??N! CH??C B???N TH??NH C??NG (^_^) ! ");
  Blynk.virtualWrite(terminal, " * TH???I GIAN CH???Y");
  Blynk.virtualWrite(terminal, "    " + String(on_hours) + " : " + String(on_minutes) + " : " + String(on_seconds));
  Blynk.virtualWrite(terminal, " * TH???I GIAN D???NG");
  Blynk.virtualWrite(terminal, "    " + String(off_hours) + " : " + String(off_minutes) + " : " + String(off_seconds));
  Blynk.virtualWrite(terminal, "----------------------------");
  time_get = millis();
  st_guong.off();
  time_on = 1;
  time_off = 0;
  da_thuc_thi = 0;
  time_count = 0;
  Serial.println("OXI 0");
  timer = millis();
  digitalWrite(D0, 1);
  while (WiFi.status() != WL_CONNECTED) {
    //Serial.print(".");
    delay(500);
    if (millis() - timer > 60000)
    {
      //bot.sendMessage(CHAT_ID, connected_wifi, "");
      delay(10000);
      ESP.restart();
    }
  }
  //myserial.println("hello");
}

void loop()
{
  if (millis() - time_ping > 60000)
  {
    Serial.println("ESP_RESPON 1");
    time_ping = millis();
  }
  if (millis() - time_reset > 60000)
  {
    arduino_err += 1;
    if (arduino_err == 3)
    {
      digitalWrite(D0, 0);
      delay(100);
      digitalWrite(D0, 1);
      delay(100);
      arduino_err = 0;
    }
    time_reset = millis();
  }
  if (return_st_esp == 1)
  {
    Serial.println("ESP_RESPON 1");
    return_st_esp = 0;
  }
  Blynk.run();
  sCmd.readSerial();
  delay(0);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10000);
    ESP.restart();
  }
}

void terminal1(int hien_thi)
{
  if (BAT_DAU == 1 && hien_thi == 1)
  {
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "----TR???NG TH??I OXI---");
    Blynk.virtualWrite(terminal, "???? K??CH HO???T CH???Y T??? ?????NG");

  }
  if (BAT_DAU == 0 && hien_thi == 1)
  {
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "----TR???NG TH??I OXI---");
    Blynk.virtualWrite(terminal, "D???NG K??CH HO???T T??? ?????NG");
    Serial.println("OXI 0");
  }
  if (time_off == 1 && hien_thi == 1 )
  {
    Blynk.virtualWrite(terminal, "D???NG CH???Y ");
  }
  if (time_on == 1 && hien_thi == 1 )
  {
    Blynk.virtualWrite(terminal, "??ANG CH???Y");

  }
}
void RETURN_STATUS()
{
  char *arg ;
  arg = sCmd.next();
  return_st_esp = atoi(arg); // Chuy???n chu???i th??nh s???
  if(return_st_esp==1)
  {
    arduino_err=0;
    }
}
void BTN_START_STOP()
{
  char *arg ;
  arg = sCmd.next();
  int  value = atoi(arg); // Chuy???n chu???i th??nh s???
  BAT_DAU = value;
  if (value == 1)
  {
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "----TR???NG TH??I OXI---");
    Blynk.virtualWrite(terminal, "???? K??CH HO???T CH???Y TR??N T??? ??I???N");
  }
  else
  {
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "----TR???NG TH??I OXI---");
    Blynk.virtualWrite(terminal, "D???NG K??CH HO???T CH???Y TR??N T??? ??I???N ");
  }
  arg = NULL;

}
void SAVE_ROTARY_ON_TIME()
{
  char *arg ;
  arg = sCmd.next();
  on_minutes = atoi(arg); // Chuy???n chu???i th??nh s???
  //on_minutes =value;
  //arg=NULL;
  BIT_SAVE = 1;
  if (BIT_SAVE == 1)
  {
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "TH???I GIAN C??I ?????T ???? L??U TR??N T??? ??I???N");
    Blynk.virtualWrite(terminal, " * TH???I GIAN CH???Y");
    Blynk.virtualWrite(terminal, "    " + String(on_hours) + " : " + String(on_minutes) + " : " + String(on_seconds));
    Blynk.virtualWrite(terminal, " * TH???I GIAN D???NG");
    Blynk.virtualWrite(terminal, "    " + String(off_hours) + " : " + String(off_minutes) + " : " + String(off_seconds));
    Blynk.virtualWrite(terminal, "----------------------------");
    run_time = (on_hours * 3600) + (on_minutes * 60) + on_seconds;
    Serial.println("RUN_TIME " + String(run_time));
    delay(500);
    Serial.println("STOP_TIME " + String(stop_time));
    BIT_SAVE = 0;
  }
}
void SAVE_ROTARY_OFF_TIME()
{
  char *arg ;
  arg = sCmd.next();
  off_minutes = atoi(arg); // Chuy???n chu???i th??nh s???
  //off_minutes=value;
  //arg=NULL;
  BIT_SAVE = 1;
  if (BIT_SAVE == 1)
  {
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "TH???I GIAN C??I ?????T ???? L??U TR??N T??? ??I???N");
    Blynk.virtualWrite(terminal, " * TH???I GIAN CH???Y");
    Blynk.virtualWrite(terminal, "    " + String(on_hours) + " : " + String(on_minutes) + " : " + String(on_seconds));
    Blynk.virtualWrite(terminal, " * TH???I GIAN D???NG");
    Blynk.virtualWrite(terminal, "    " + String(off_hours) + " : " + String(off_minutes) + " : " + String(off_seconds));
    Blynk.virtualWrite(terminal, "----------------------------");
    stop_time = (off_hours * 3600) + (off_minutes) * 60 + off_seconds;

    Serial.println("RUN_TIME " + String(run_time));
    delay(500);
    Serial.println("STOP_TIME " + String(stop_time));
    BIT_SAVE = 0;
  }
}
void GET_TEMP()
{
  char *arg ;
  arg = sCmd.next();
  TEMP_ = atoi(arg);
  Blynk.virtualWrite(temp, TEMP_);
}
void MONITOR_TIME_OXI()
{
  char *arg ;
  arg = sCmd.next();
  TIMER_OXI = atoi(arg);
  if (STT_OXI == 0 && run_time != NULL)
  {
    Blynk.virtualWrite(terminal, "* TH???I GIAN B???T OXI C??N L???I: " + String(TIMER_OXI) + "s");
  }
  if (STT_OXI == 1 && stop_time != NULL)
  {
    Blynk.virtualWrite(terminal, "* TH???I GIAN T???T OXI C??N L???I: " + String(TIMER_OXI) + "s");
  }


}
void MONITOR_ST_OXI()
{
  char *arg ;
  arg = sCmd.next();
  STT_OXI = atoi(arg);
  if (STT_OXI == 0)
  {
    st_guong.on();
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "--TR???NG TH??I OXI--");
    Blynk.virtualWrite(terminal, "???? K??CH HO???T TR??N T??? ??I???N");
    Blynk.virtualWrite(terminal, "??ANG CH???Y");

  }
  else
  {
    st_guong.off();
    Blynk.virtualWrite(terminal, "----------------------------");
    Blynk.virtualWrite(terminal, "--TR???NG TH??I OXI--");
    Blynk.virtualWrite(terminal, "D???NG K??CH HO???T TR??N T??? ??I???N");
    Blynk.virtualWrite(terminal, "D???NG CH???Y");

  }
}
