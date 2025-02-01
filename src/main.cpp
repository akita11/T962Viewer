#include <Arduino.h>
#include <M5Unified.h>

char buf[1024];
uint8_t p = 0;

float tm01 = 0.0, tm12 = 0.0, tm23 = 0.0, tm = 0.0, 
      temp = 0.0, temp_target = 0.0, temp_th = 0.0,
      dur = 0.0;
uint8_t st = 0, st0 = 0, fRun = 0;
uint8_t s1, s2;
int ctrl = 0;

void DrawFloat(float v, int x, int y, float size, uint16_t color)
{
  char str[80];
  M5.Display.setTextColor(color);
  M5.Display.setTextSize(size);
  if (v == 0.0) strcpy(str, "--"); else sprintf(str, "%.1f", v);
//  sprintf(str, "%.1f", v);
  M5.Display.drawString(str, x, y);

}

void ShowInfo(uint16_t bg_color)
{
  M5.Display.clearDisplay(bg_color);
  M5.Display.setTextSize(2.5); M5.Display.drawString("T=", 0, 8);
  DrawFloat(tm,           90,   8, 2.5, TFT_WHITE);
  M5.Display.setTextSize(2); M5.Display.drawString("Tmp", 0, 65);
  DrawFloat(temp,        110,  60,   3, TFT_WHITE);
  DrawFloat(temp_target,   0, 120, 1.5, TFT_WHITE);
  DrawFloat(temp_th,      110, 120, 1.5, TFT_WHITE);
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

  char str[80];
  M5.Display.setTextSize(1.0);
  sprintf(str, "%d %d %d", s1, s2, st);
  M5.Display.drawString(str, 250, 5);
  sprintf(str, "%d", ctrl);
  M5.Display.drawString(str, 250, 20);
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 22, 21); // PortA
  auto cfg = M5.config(); 
  M5.begin(cfg);
  M5.Display.setFont(&fonts::Font4);
  ShowInfo(TFT_BLACK);
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
         tm01 = 0.0; tm12 = 0.0; tm23 = 0.0;
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
            tm01 = tm; bg_color = TFT_MAGENTA;
          }
          else if (st == 10){
            tm12 = tm; bg_color = TFT_RED;
          }
          else if (st == 11){
            tm12 = tm; bg_color = TFT_PURPLE;
          }
          else if (st == 20 || st == 21){
            tm23 = tm; bg_color = TFT_DARKCYAN;
          }
        }
        st0 = st;
        ShowInfo(bg_color);
      }
      if (buf[0] == '#') fRun = 1;
    }
  }  
}
