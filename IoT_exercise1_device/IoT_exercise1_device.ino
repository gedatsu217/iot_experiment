#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include "TimeLib.h"


const char ssid[]="";
const char pass[]="";
const char host[]="";
const int port = 10001;
const char* ntp_server="ntp.nict.jp";
unsigned long last_sync_time=0;
int connect_count=0;
int cul30=0;

#define OLED_RESET 2
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(16, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Connecting to: ");
  display.print(ssid);
  display.display();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    display.print(".");
    display.display();
    connect_count+=1;
    if(connect_count>50){
      display.println();
      display.println("failure");
      display.display();
      delay(2000);
      exit(1);
    }
  }

  display.println();
  display.display();
  display.println("WiFi connected");
  display.println("IP address:");
  display.println(WiFi.localIP());
  display.println("success");
  display.display();
  delay(3000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Sync to ");
  display.println(ntp_server);
  display.display();
  
  if(syncNTPtime()){
    display.println();
    display.println("success");
    
    unsigned long t = now();
    char str_time[30];
    sprintf(str_time, "%04d-%02d-%02dT%02d:%02d:%02d",
                      year(t),month(t), day(t),
                      hour(t), minute(t), second(t));
    display.println(str_time);
    display.display();
    delay(3000);
  }
  else{
    display.println("failure");
    display.display();
    delay(2000);
    exit(1);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long t = now();

  char str_time[30];
  sprintf(str_time, "%04d-%02d-%02dT%02d:%02d:%02d",
                    year(t),month(t), day(t),
                    hour(t), minute(t), second(t));

  display.println(str_time);

  if(t/30!=cul30){
    int stat_DIPS=getDIPSWStatus();
    int lux=getIlluminance();
    boolean stat_MD=getMDStatus();
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.println(stat_DIPS);
    display.println(str_time);
    display.println(lux);
    display.println(stat_MD);
    
    display.display();
    cul30=t/30;

    WiFiClient client;

    if(!client.connect(host,port)){
      display.println("...ERR\r\n");
      display.display();
      client.stop();
      exit(1);
    }

    client.print(stat_DIPS);
    delay(50);
    client.print(str_time);
    delay(50);
    client.print(lux);
    delay(50);
    client.print(stat_MD);
    delay(500);
    char message[10];
    int count=0;
    
    while(client.available()){
      message[count]=client.read();
      count++;
    }
    char *error = "ERROR\r\n";
    char *ok = "OK\r\n";
    
    if(strncmp(message, ok, 2)==0){
      display.println("...OK\r\n");
    }
    else if(strncmp(message, error, 5)==0){
      display.println("...NG\r\n");
    }

    else if(!client.connect(host,port)){
      display.println("...ERR\r\n");
    }

    display.display();
    client.stop();
  }

  if(t/300!=last_sync_time/300){
    syncNTPtime();
    last_sync_time=t;
  }
  delay(500);
}

boolean syncNTPtime(){
  unsigned long unix_time=getNTPtime();
  if(unix_time>0){
    setTime(unix_time);
    return true;
  }
  return false;
}

unsigned long getNTPtime(){
  WiFiUDP udp;
  udp.begin(8888);
  unsigned long unix_time=0UL;
  byte packet[48];
  memset(packet, 0, 48);
  packet[0]=0b11100011;
  packet[1]=0;
  packet[2]=6;
  packet[3]=0xEC;
  packet[12]=49;
  packet[13]=0x4E;
  packet[14]=49;
  packet[15]=52;

  udp.beginPacket(ntp_server, 123);
  udp.write(packet, 48);
  udp.endPacket();

  for(int i=0; i<10; i++){
    delay(500);
    if(udp.parsePacket()){
      udp.read(packet, 48);
      unsigned long highWord = word(packet[40], packet[41]);
      unsigned long lowWord = word(packet[42], packet[43]);
      unsigned long secsSince1900=highWord<<16 | lowWord;
      const unsigned long seventyYears=2208988800UL;
      unix_time=secsSince1900-seventyYears+32400UL;
      break;
    }
  }
  udp.stop();
  return unix_time;
}

int getDIPSWStatus(){
  int stat=0;
  int bit1=digitalRead(12);
  int bit0=digitalRead(13);
  if(bit0==LOW) stat|=0x01;
  if(bit1==LOW) stat|=0x02;
  return stat;
}

int getIlluminance(){
  double inv=analogRead(A0);
  double v=inv/1024.0;
  int ans=(3200*v)/3;
  return ans;
}

boolean getMDStatus(){
  int stat=digitalRead(16);
  if(stat==LOW) return false;
  else return true;
}
