/*
 * config.h - Configuration and Global Variables
 * 
 * Contains all configuration constants, WiFi credentials, and global variables
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <ESP8266WebServer.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi credentials
const char* ssid = "my_network_name";
const char* password = "my_network_password";

// Admin panel
#define ENABLE_ADMIN_PANEL true
const char* adminPassword = "admin123";  // ⚠️ Change this!

// Hardware pins
const int SD_CS_PIN = D8;
const int WIFI_LED_PIN = LED_BUILTIN;

// Traffic logging
#define ENABLE_TRAFFIC_LOG true  // Set to false to disable logging
const int MAX_LOG_SIZE = 500000;  // 500KB max log size before rotation

// NTP time sync settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;        // GMT offset in seconds (0 = UTC)
const int daylightOffset_sec = 0;    // Daylight saving time offset
// To set your timezone: 
//   PST (UTC-8): gmtOffset_sec = -8 * 3600;
//   EST (UTC-5): gmtOffset_sec = -5 * 3600;
//   CET (UTC+1): gmtOffset_sec = 1 * 3600;

// Pagination
const int POSTS_PER_PAGE = 20;

// ============================================================================
// DATA STRUCTURES
// ============================================================================

struct PostMapping {
  String urlPath;
  String fileName;
  String title;
};

struct Redirection {
  String fromPath;
  String toPath;
};

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

extern ESP8266WebServer server;

extern PostMapping* postMappings;
extern int postMappingsCount;

extern Redirection* redirections;
extern int redirectionsCount;

extern String logoBase64;

#endif // CONFIG_H
