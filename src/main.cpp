#include <Arduino.h>
#include <M5Unified.h>

char buf[1024];
uint8_t p = 0;

// Screen Design
//T
//|   graph        : (Temp, Temp_target, Temp_th)
//+--------------t
// Time (ElaspTime/Dur) <--- 200
// 10   50   

float tms = 0.0, tm = 0.0, 
      temp = 0.0, temp_target = 0.0, temp_th = 0.0,
      dur = 0.0;
uint8_t st = 0, st0 = 0, fRun = 0;
uint8_t s1, s2;
int ctrl = 0;

uint16_t temp_conv(float temp)
{
  // 20deg - 300deg -> 200 - 0
  int y = (300 - temp) * 200 / 280;
  if (y < 0) y = 0;
  if (y > 200) y = 200;
  return(y);
}

uint16_t time_conv(uint16_t t)
{
  // 0 - 360 -> 0 - 320
  int x = t * 320 / 360;
  if (x < 0) x = 0;
  if (x > 320) x = 320;
  return(x);
}

void ShowInfo(uint16_t bg_color)
{
  M5.Display.fillRect(0, 201, 320, 39, bg_color);
  M5.Display.setTextColor(TFT_WHITE, bg_color);
  M5.Display.setCursor(10, 201); M5.Display.printf("%.0fs", tm);
  if (dur != 0 & tms != 0)
    M5.Display.printf(" (%.0f / %.0f)", tm - tms, dur);

/*
  M5.Display.setTextSize(1.5); M5.Display.drawString("Dur", 20, 160);
  DrawFloat(dur,         120, 160, 1.5, TFT_WHITE);
  DrawFloat(tm01,         20, 200, 1.5, TFT_WHITE);
//  float t1 = 0.0, t2 = 0.0;
//  if (tm12 != 0) t1 = tm12 - tm01;
//  if (tm23 != 0) t2 = tm23 - tm12;
//  DrawFloat(t1, 130, 200, 1.5, TFT_WHITE);
//  DrawFloat(t2, 240, 200, 1.5, TFT_WHITE);
  DrawFloat(tm12, 130, 200, 1.5, TFT_WHITE);
  DrawFloat(tm23, 240, 200, 1.5, TFT_WHITE);
*/
  uint16_t x = (int)tm;
  M5.Display.drawPixel(time_conv(x), temp_conv(temp), TFT_WHITE);
  M5.Display.drawPixel(time_conv(x), temp_conv(temp_target), TFT_RED);
  M5.Display.drawPixel(time_conv(x), temp_conv(temp_th), TFT_BLUE);

/*
  char str[80];
  M5.Display.setTextSize(1.0);
  sprintf(str, "%d %d %d", s1, s2, st);
  M5.Display.drawString(str, 250, 5);
  sprintf(str, "%d", ctrl);
  M5.Display.drawString(str, 250, 20);
*/
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 22, 21); // PortA
  auto cfg = M5.config(); 
  M5.begin(cfg);
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.drawFastHLine(0, 200, 320, TFT_WHITE);
  M5.Display.drawFastHLine(0, temp_conv(20), 320, TFT_DARKGRAY);
  M5.Display.setFont(&fonts::Font4);
  for (uint16_t tmp = 50; tmp < 300; tmp+=50){
    M5.Display.drawFastHLine(0, temp_conv(tmp), 320, TFT_DARKGRAY);
    if (tmp % 100 == 0){
      M5.Display.setCursor(0, temp_conv(tmp)- 13);
      M5.Display.printf("%d", tmp);

    }
  }
  for (uint16_t t = 0; t < 360; t+=60)
    M5.Display.drawFastVLine(time_conv(t), 0, 200, TFT_DARKGRAY);
  M5.Display.setFont(&fonts::Font4);
  M5.Display.setTextSize(1.5, 1.8);


  M5.Display.drawFastVLine(0, 0, 200, TFT_WHITE);
}  

// st fr  st'
// 0  0  rise 1
// 0  1  keep 1
// 1  0  rise 2
// 1  1  keep 2
// 2  0  cool 2
// 2  1  cool 2'

uint16_t bg_color = 0x0000;

void loop() {
  /*
  // for debug
  M5.update();
  if (M5.BtnA.wasClicked()){
    tm += 1.0;
    temp = 20 + tm;
    temp_th = temp + 10;
    temp_target = temp + 20;
    printf("%.1f %.1f %.1f\n", tm, temp, temp_th, temp_target);
    if (tm < 10) bg_color = TFT_BLACK;
    else if (tm < 20) bg_color = TFT_ORANGE;
    else if (tm < 30) bg_color = TFT_MAGENTA;
    else if (tm < 40) bg_color = TFT_RED;
    else if (tm < 50) bg_color = TFT_PURPLE;
    else bg_color = TFT_DARKCYAN;
    if (tm > 20) dur = 10;
    if (tm == 20) tms = tm;
    ShowInfo(bg_color);
  }
*/
  while(Serial1.available()){
    char c = Serial1.read();
    buf[p++] = c;
    if (c == '\n'){
      buf[p] = '\0';
      p = 0;
      char fd[80];
      strcpy(fd, strtok(buf, " \t"));
      if (strcmp(fd, "Reflow") == 0){
         fRun = 0;
         tm = 0.0; temp_target = 0.0; temp_th = 0.0;
         tms = 0.0;
         ShowInfo(TFT_BLACK);
      }
      if (fRun == 1){
        tm = atof(fd);
        temp = atof(strtok(NULL, " ¥t"));
        temp_target = atof(strtok(NULL, " ¥t"));
        temp_th = atof(strtok(NULL, " ¥t"));
        dur = atof(strtok(NULL, " ¥t"));
        s1 = atoi(strtok(NULL, " ¥t"));
        s2 = atoi(strtok(NULL, " ¥t"));
        st = s1 * 10 + s2;
        strtok(NULL, " ¥t");
        strtok(NULL, " ¥t");
        ctrl = atoi(strtok(NULL, " ¥t"));
        if (st != st0){
          if (st == 0) bg_color = TFT_ORANGE;
          else if (st == 1){
            tms = tm; bg_color = TFT_MAGENTA;
          }
          else if (st == 10){
            tms = 0; bg_color = TFT_RED;
          }
          else if (st == 11){
            tms = tm; bg_color = TFT_PURPLE;
          }
          else if (st == 20 || st == 21){
            tms = 0; bg_color = TFT_DARKCYAN;
          }
        }
        st0 = st;
        ShowInfo(bg_color);
      }
      if (buf[0] == '#') fRun = 1;
    }
  }  
}
