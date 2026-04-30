#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>

#include "margo_features.h"

namespace {

constexpr uint16_t kBg = 0x0841;
constexpr uint16_t kPanel = 0x10A2;
constexpr uint16_t kAccent = 0x07FF;
constexpr uint16_t kText = 0xFFFF;
constexpr uint16_t kMuted = 0xA514;
constexpr uint16_t kWarn = 0xFBE0;

enum class Screen : uint8_t {
  Launcher,
  App,
};

Screen currentScreen = Screen::Launcher;
size_t selectedApp = 0;
uint8_t brightness = 140;
bool ext5vEnabled = false;
int lastWifiCount = -1;
uint32_t lastRender = 0;

void drawHeader(const char *title) {
  M5.Display.fillScreen(kBg);
  M5.Display.fillRoundRect(0, 0, M5.Display.width(), 28, 4, kPanel);
  M5.Display.setTextColor(kAccent, kPanel);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(8, 6);
  M5.Display.print("MARGO");
  M5.Display.setTextColor(kText, kPanel);
  M5.Display.setCursor(64, 6);
  M5.Display.print(title);
}

void drawFooter(const char *left, const char *right) {
  const int16_t y = M5.Display.height() - 18;
  M5.Display.fillRect(0, y, M5.Display.width(), 18, kPanel);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kMuted, kPanel);
  M5.Display.setCursor(6, y + 4);
  M5.Display.print(left);
  M5.Display.setCursor(M5.Display.width() - 82, y + 4);
  M5.Display.print(right);
}

void drawLauncher() {
  drawHeader("Launcher");
  M5.Display.setTextSize(1);

  for (size_t i = 0; i < MARGO_APP_COUNT; ++i) {
    const int16_t y = 36 + static_cast<int16_t>(i) * 28;
    const bool active = i == selectedApp;
    M5.Display.fillRoundRect(6, y, M5.Display.width() - 12, 24, 4, active ? kAccent : kPanel);
    M5.Display.setTextColor(active ? kBg : kText, active ? kAccent : kPanel);
    M5.Display.setCursor(14, y + 4);
    M5.Display.print(MARGO_APPS[i].name);
    M5.Display.setTextColor(active ? kBg : kMuted, active ? kAccent : kPanel);
    M5.Display.setCursor(96, y + 4);
    M5.Display.print(MARGO_APPS[i].hint);
  }

  drawFooter("A open", "B next");
}

String encryptionName(wifi_auth_mode_t mode) {
  switch (mode) {
  case WIFI_AUTH_OPEN:
    return "open";
  case WIFI_AUTH_WEP:
    return "WEP";
  case WIFI_AUTH_WPA_PSK:
    return "WPA";
  case WIFI_AUTH_WPA2_PSK:
    return "WPA2";
  case WIFI_AUTH_WPA_WPA2_PSK:
    return "WPA/WPA2";
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return "WPA2-E";
  case WIFI_AUTH_WPA3_PSK:
    return "WPA3";
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return "WPA2/3";
  default:
    return "?";
  }
}

void drawStatus() {
  drawHeader("Status");
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kText, kBg);
  M5.Display.setCursor(10, 38);
  M5.Display.printf("Battery: %d%%", M5.Power.getBatteryLevel());
  M5.Display.setCursor(10, 56);
  M5.Display.printf("USB: %s", M5.Power.isCharging() ? "charging" : "not charging");
  M5.Display.setCursor(10, 74);
  M5.Display.printf("Uptime: %lu sec", millis() / 1000UL);
  M5.Display.setCursor(10, 92);
  M5.Display.printf("Heap: %u free", ESP.getFreeHeap());
  M5.Display.setCursor(10, 110);
  M5.Display.printf("PSRAM: %u free", ESP.getFreePsram());
  drawFooter("Hold A home", "B refresh");
}

void drawWifiScan() {
  drawHeader("Wi-Fi Scan");
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kMuted, kBg);
  M5.Display.setCursor(10, 36);
  M5.Display.print("Passive scan only. No joins.");

  if (lastWifiCount < 0) {
    M5.Display.setTextColor(kWarn, kBg);
    M5.Display.setCursor(10, 58);
    M5.Display.print("Press A to scan.");
  } else {
    M5.Display.setTextColor(kText, kBg);
    M5.Display.setCursor(10, 54);
    M5.Display.printf("%d networks", lastWifiCount);
    const int maxRows = min(lastWifiCount, 5);
    for (int i = 0; i < maxRows; ++i) {
      const int16_t y = 74 + i * 18;
      String ssid = WiFi.SSID(i);
      if (ssid.length() > 13) {
        ssid = ssid.substring(0, 12) + ".";
      }
      M5.Display.setCursor(10, y);
      M5.Display.printf("%-13s %4d", ssid.c_str(), WiFi.RSSI(i));
      M5.Display.setCursor(168, y);
      M5.Display.print(encryptionName(WiFi.encryptionType(i)));
    }
  }

  drawFooter("A scan", "B home");
}

void runWifiScan() {
  drawHeader("Wi-Fi Scan");
  M5.Display.setTextColor(kWarn, kBg);
  M5.Display.setCursor(10, 54);
  M5.Display.print("Scanning...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(100);
  lastWifiCount = WiFi.scanNetworks(false, true);
  drawWifiScan();
}

void drawInfrared() {
  drawHeader("Infrared");
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kText, kBg);
  M5.Display.setCursor(10, 40);
  M5.Display.print("StickS3 IR power rail:");
  M5.Display.setCursor(10, 60);
  M5.Display.setTextColor(ext5vEnabled ? kAccent : kWarn, kBg);
  M5.Display.print(ext5vEnabled ? "EXT 5V output enabled" : "EXT 5V output disabled");
  M5.Display.setTextColor(kMuted, kBg);
  M5.Display.setCursor(10, 88);
  M5.Display.print("Transmitting is not enabled");
  M5.Display.setCursor(10, 104);
  M5.Display.print("in this starter build.");
  drawFooter("A toggle 5V", "B home");
}

void drawSettings() {
  drawHeader("Settings");
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kText, kBg);
  M5.Display.setCursor(10, 42);
  M5.Display.printf("Brightness: %u", brightness);
  M5.Display.setCursor(10, 62);
  M5.Display.printf("EXT 5V: %s", ext5vEnabled ? "on" : "off");
  M5.Display.setTextColor(kMuted, kBg);
  M5.Display.setCursor(10, 92);
  M5.Display.print("A brightness +");
  M5.Display.setCursor(10, 108);
  M5.Display.print("B toggles EXT 5V");
  drawFooter("Hold A home", "B toggle");
}

void drawAbout() {
  drawHeader("About");
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(kText, kBg);
  M5.Display.setCursor(10, 42);
  M5.Display.print("Margo Firmware");
  M5.Display.setCursor(10, 62);
  M5.Display.print("Target: M5Stack StickS3");
  M5.Display.setCursor(10, 82);
  M5.Display.print("Build: clean starter");
  M5.Display.setTextColor(kMuted, kBg);
  M5.Display.setCursor(10, 110);
  M5.Display.print("Add modules in margo_features.h");
  drawFooter("Hold A home", "B home");
}

void drawCurrentApp() {
  switch (MARGO_APPS[selectedApp].id) {
  case AppId::Status:
    drawStatus();
    break;
  case AppId::WifiScan:
    drawWifiScan();
    break;
  case AppId::Infrared:
    drawInfrared();
    break;
  case AppId::Settings:
    drawSettings();
    break;
  case AppId::About:
    drawAbout();
    break;
  }
}

void goHome() {
  currentScreen = Screen::Launcher;
  drawLauncher();
}

void openSelectedApp() {
  currentScreen = Screen::App;
  drawCurrentApp();
}

void toggleExt5v() {
  ext5vEnabled = !ext5vEnabled;
  M5.Power.setExtOutput(ext5vEnabled);
}

void handleLauncherButtons() {
  if (M5.BtnB.wasPressed()) {
    selectedApp = (selectedApp + 1) % MARGO_APP_COUNT;
    drawLauncher();
  }
  if (M5.BtnA.wasPressed()) {
    openSelectedApp();
  }
}

void handleAppButtons() {
  if (M5.BtnA.pressedFor(900)) {
    goHome();
    return;
  }

  const AppId app = MARGO_APPS[selectedApp].id;
  if (M5.BtnA.wasPressed()) {
    if (app == AppId::WifiScan) {
      runWifiScan();
    } else if (app == AppId::Infrared) {
      toggleExt5v();
      drawInfrared();
    } else if (app == AppId::Settings) {
      brightness = brightness >= 240 ? 60 : brightness + 30;
      M5.Display.setBrightness(brightness);
      drawSettings();
    } else {
      drawCurrentApp();
    }
  }

  if (M5.BtnB.wasPressed()) {
    if (app == AppId::Settings) {
      toggleExt5v();
      drawSettings();
    } else {
      goHome();
    }
  }

  if (M5.BtnC.wasPressed()) {
    goHome();
  }
}

} // namespace

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  cfg.output_power = false;
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setBrightness(brightness);
  WiFi.mode(WIFI_OFF);
  Serial.println("Margo Firmware booted");
  drawLauncher();
}

void loop() {
  M5.update();

  if (currentScreen == Screen::Launcher) {
    handleLauncherButtons();
  } else {
    handleAppButtons();
  }

  if (currentScreen == Screen::App && MARGO_APPS[selectedApp].id == AppId::Status &&
      millis() - lastRender > 2000) {
    lastRender = millis();
    drawStatus();
  }

  delay(15);
}
