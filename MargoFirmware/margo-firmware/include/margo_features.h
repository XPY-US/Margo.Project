#pragma once

#include <Arduino.h>

enum class AppId : uint8_t {
  Status,
  WifiScan,
  Infrared,
  Settings,
  About,
};

struct MargoApp {
  AppId id;
  const char *name;
  const char *hint;
};

constexpr MargoApp MARGO_APPS[] = {
    {AppId::Status, "Status", "Battery and memory"},
    {AppId::WifiScan, "Wi-Fi Scan", "Passive nearby APs"},
    {AppId::Infrared, "Infrared", "IR lab controls"},
    {AppId::Settings, "Settings", "Display and power"},
    {AppId::About, "About", "Build information"},
};

constexpr size_t MARGO_APP_COUNT = sizeof(MARGO_APPS) / sizeof(MARGO_APPS[0]);
