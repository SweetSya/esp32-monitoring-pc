#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>

#include "D:\Programming\Arduino\monitoring_pc_device\src\icons.h"

TFT_eSPI tft = TFT_eSPI(); // TFT instance

#define COLOR_DARK 0x0000  // Black
#define COLOR_WHITE 0xFFFF // White
#define BACKLIGHT_PIN 15   // D15 = GPIO15

int offsetTop = 0;
int offsetLeft = 0;

int SCREEN_HORIZONTAL_MAX = 480;
int SCREEN_VERTICAL_MAX = 320;
int TEXT_VERTICAL_SHIFTING = 0;

String username = "Sya19_PC";
String mainboard = "";
String cpuName = "";
String gpuName = "";

String line = "";
String state = "sleep";
bool initiate = false;

const char *ssid = "GaJelas";
const char *password = "yotidaktaukoktanyasaya";

float cpuUsage = 0, gpuUsage = 0, cpuTemp = 0, gpuTemp = 0;
int cpuMaxTemp = 0, cpuMinTemp = 0, gpuMaxTemp = 0, gpuMinTemp = 0;

void drawIcon(int x, int y, const unsigned char *icon, int w = 32, int h = 32, uint16_t color = TFT_BLACK)
{
  tft.drawBitmap(x, y, icon, w, h, color);
}

void drawInfoBox(int x, int y, float col, int height, String title, String label, int usage, int temp, int minT, int maxT, uint16_t color, bool initiate_rendering)
{
  float BAR_W = (SCREEN_HORIZONTAL_MAX * col) - 110;
  float BAR_H = 24;
  float BOX_W = SCREEN_HORIZONTAL_MAX * col;
  float BOX_H = height;

  if (initiate_rendering)
  {
    tft.drawRoundRect(x, y, BOX_W, BOX_H, 5, COLOR_WHITE);
    tft.fillRoundRect(x, y + 30, BOX_W, BOX_H - 30, 5, COLOR_WHITE);
    tft.drawRoundRect(x, y, BOX_W, 30, 5, COLOR_WHITE);
    tft.fillRect(x, y, 50, 30, COLOR_WHITE);
  }

  y += 7;

  tft.setTextColor(COLOR_DARK);
  tft.setTextSize(2);
  tft.setCursor(x + 7, y + TEXT_VERTICAL_SHIFTING);
  tft.print(title);

  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(x + 62, y + TEXT_VERTICAL_SHIFTING);
  tft.print(label);

  y += 30;
  x += 10;

  tft.setTextColor(COLOR_DARK);
  tft.setCursor(x + 5, y + 10);
  drawIcon(x, y, epd_bitmap_icon_usage);
  x += 40;
  y += 4;

  int barWidth = map(usage, 0, 100, 0, BAR_W);
  tft.fillRect(x, y, BAR_W, BAR_H, COLOR_WHITE);
  tft.drawRoundRect(x, y, BAR_W, BAR_H, 5, COLOR_DARK);
  tft.fillRoundRect(x + 1, y + 1, barWidth - 2, BAR_H - 2, 5, color);

  tft.setCursor(x + BAR_W + 8, y + 5 + TEXT_VERTICAL_SHIFTING);
  tft.fillRect(x + BAR_W + 8, y + 5, 50, 25, COLOR_WHITE);
  tft.printf("%d%%", usage);

  x -= 40;
  y += 35;

  tft.setCursor(x, y + TEXT_VERTICAL_SHIFTING);
  drawIcon(x, y, epd_bitmap_icon_temp);
  x += 40;
  y += 4;

  int tempBar = map(temp, 20, 110, 0, BAR_W);
  tft.fillRect(x, y, BAR_W, BAR_H, COLOR_WHITE);
  tft.drawRoundRect(x, y, BAR_W, BAR_H, 5, COLOR_DARK);

  if (temp <= 50)
    tft.fillRoundRect(x + 1, y + 1, tempBar - 2, BAR_H - 2, 5, TFT_CYAN);
  else if (temp <= 85)
    tft.fillRoundRect(x + 1, y + 1, tempBar - 2, BAR_H - 2, 5, TFT_GREENYELLOW);
  else
    tft.fillRoundRect(x + 1, y + 1, tempBar - 2, BAR_H - 2, 5, TFT_RED);

  tft.setCursor(x + BAR_W + 8, y + 5 + TEXT_VERTICAL_SHIFTING);
  tft.fillRect(x + BAR_W + 8, y + 5, 50, 25, COLOR_WHITE);
  if (temp < 20)
    tft.print("20C");
  else if (temp < 110)
    tft.printf("%dC", temp);
  else
    tft.print("110C");

  x -= 40;
  y += 40;

  tft.fillRect(x, y, BOX_W - 20, 20, COLOR_WHITE);
  tft.setTextSize(2);
  tft.setCursor(x, y + TEXT_VERTICAL_SHIFTING);
  tft.printf("MIN:%dC", minT);
  tft.setCursor(x + 120, y + TEXT_VERTICAL_SHIFTING);
  tft.printf("MAX:%dC", maxT);
}

void clear_screen()
{
  offsetTop = 30;
  tft.fillRect(offsetLeft, offsetTop, SCREEN_HORIZONTAL_MAX, SCREEN_VERTICAL_MAX, COLOR_DARK);
}

void set_header(String title)
{
  offsetTop = 0;
  offsetLeft = 0;
  tft.fillRect(offsetLeft, offsetTop, SCREEN_HORIZONTAL_MAX, offsetTop + 30, COLOR_WHITE);
  tft.setTextColor(COLOR_DARK);
  tft.setTextSize(2);
  tft.setCursor(offsetLeft + 15, offsetTop + 8 + TEXT_VERTICAL_SHIFTING);
  tft.print(title);
  offsetTop += 35;
}
void intTempTrack(float temp, String sensor)
{
  int maxTemp, minTemp;
  if (sensor == "cpu")
  {
    maxTemp = cpuMaxTemp;
    minTemp = cpuMinTemp;
  }
  if (sensor == "gpu")
  {
    maxTemp = gpuMaxTemp;
    minTemp = gpuMinTemp;
  }
  int t = (int)temp;
  if (t > maxTemp)
  {
    maxTemp = t;
  }
  if (t < minTemp || minTemp == 0)
  {
    minTemp = t;
  }

  if (sensor == "cpu")
  {
    cpuMaxTemp = maxTemp;
    cpuMinTemp = minTemp;
  }
  if (sensor == "gpu")
  {
    gpuMaxTemp = maxTemp;
    gpuMinTemp = minTemp;
  }
}

void parseData(String data)
{
  int start = 0;
  while (true)
  {
    int sep = data.indexOf('|', start);
    String part = (sep == -1) ? data.substring(start) : data.substring(start, sep);

    int eq = part.indexOf('=');
    if (eq != -1)
    {
      String key = part.substring(0, eq);
      String value = part.substring(eq + 1);

      if (key == "MB" && value != "")
        mainboard = value;
      else if (key == "CPU" && value != "")
        cpuName = value;
      else if (key == "CPUT" && value != "" && value.toFloat() > 0)
        cpuTemp = value.toFloat();
      else if (key == "CPUL" && value != "" && value.toFloat() > 0)
        cpuUsage = value.toFloat();
      else if (key == "GPU" && value != "")
        gpuName = value;
      else if (key == "GPUT" && value != "" && value.toFloat() > 0)
        gpuTemp = value.toFloat();
      else if (key == "GPUL" && value != "" && value.toFloat() > 0)
        gpuUsage = value.toFloat();
    }

    if (sep == -1)
      break;
    start = sep + 1;
  }
  intTempTrack(cpuTemp, "cpu");
  intTempTrack(gpuTemp, "gpu");
}

void updateDisplay()
{
  // tft.fillRect(0, 35, SCREEN_HORIZONTAL_MAX, SCREEN_VERTICAL_MAX - 35, COLOR_DARK);
  offsetLeft = 10;
  SCREEN_HORIZONTAL_MAX = 460;
  offsetTop = 40;

  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(offsetLeft + 5, offsetTop);
  tft.print(mainboard);

  offsetTop += 30;
  drawInfoBox(offsetLeft, offsetTop, 0.49, 150, "CPU", cpuName, cpuUsage, cpuTemp, cpuMinTemp, cpuMaxTemp, TFT_GREEN, initiate);
  drawInfoBox(offsetLeft + (SCREEN_HORIZONTAL_MAX / 2), offsetTop, 0.5, 150, "GPU", gpuName, gpuUsage, gpuTemp, gpuMinTemp, gpuMaxTemp, TFT_GREEN, initiate);
  if (initiate)
  {
    initiate = false;
  }
}

void connect_wifi()
{
  set_header("Connecting to WiFi..");
  WiFi.begin(ssid, password);

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    set_header("Trying to connect..");
  }
  set_header("Wifi Connected...");
  delay(1000);
  set_header("Connected :) at " + WiFi.localIP().toString());
}

// Time of waiiting
int waiting_serial = 0;

void setup()
{
  state = "initiating";
  Serial.begin(115200);
  delay(1000);
  // Attach GPIO15 to PWM channel 1
  ledcAttachPin(BACKLIGHT_PIN, 1);
  // Set up PWM on channel 1: 5kHz frequency, 8-bit resolution
  ledcSetup(1, 5000, 8);
  // Set brightness (0 = off, 255 = full brightness)
  ledcWrite(1, 85);

  tft.init();
  tft.setRotation(1); // Portrait
  clear_screen();
  set_header("Preparing device..");
  delay(2000);
  // connect_wifi();
  set_header("Ready to use..");
  state = "ready";
  offsetTop += 35;
}

void loop()
{
  SCREEN_HORIZONTAL_MAX = 480;
  SCREEN_VERTICAL_MAX = 320;
  TEXT_VERTICAL_SHIFTING = 0;

  waiting_serial += 1;
  if (Serial.available())
  {
    if (state != "wake")
    {
      state = "ready";
    }
    // if (state != "sleep")
    // {
    //   state = "sleep";
    //   clear_screen();
    //   String message = "Sleep Mode..";
    //   set_header(message);
    //   drawIcon(SCREEN_HORIZONTAL_MAX / 2 - 130, SCREEN_VERTICAL_MAX / 2 - 80, epd_bitmap_cat_sleep, 256, 168, COLOR_WHITE);
    //   ledcWrite(1, 85);
    // }
    if (state != "wake" && state == "ready")
    {
      initiate = true;
      state = "wake";
      ledcWrite(1, 220);
      clear_screen();
      set_header("Monitoring");
    }
    while (Serial.available() && state == "wake")
    {
      char c = Serial.read();
      if (c == '\n')
      {
        parseData(line);
        if (state == "wake")
        {
          updateDisplay();
        }
        line = "";
      }
      else
      {
        line += c;
      }
    }
    waiting_serial = 0;
  }
  if (waiting_serial > 2)
  {
    int turn_off_after = 5;
    if (waiting_serial > turn_off_after)
    {
      if (state != "offline")
      {
        clear_screen();
        state = "offline";
        // String message = "Offline Mode..";
        // set_header(message);
        ledcWrite(1, 0);
      }
      if (waiting_serial > 9999)
      {
        waiting_serial = 10;
      }
    }
    else
    {
      // clear_screen();
      // String message = "Offline mode in..(" + String(turn_off_after - waiting_serial) + ")";
      // set_header(message);

      if (state != "sleep")
      {
        state = "sleep";
        clear_screen();
        drawIcon(SCREEN_HORIZONTAL_MAX / 2 - 130, SCREEN_VERTICAL_MAX / 2 - 80, epd_bitmap_cat_sleep, 256, 168, COLOR_WHITE);
        ledcWrite(1, 85);
      }
      String message = "Sleep mode in..(" + String(turn_off_after - waiting_serial) + ")";
      set_header(message);
    }
  }
  delay(2000); // Smooth refresh
}
