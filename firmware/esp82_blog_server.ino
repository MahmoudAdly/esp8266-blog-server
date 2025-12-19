/*
 * ESP8266 SD Card Blog Server - Modular Version
 * 
 * A complete self-hosted blog server running on ESP8266 with SD card storage.
 * See README.md for full documentation, setup instructions, and features.
 * 
 * Hardware: Wemos Lolin D1 Pro Mini V2.0.0 + Micro SD Card Shield
 * WiFi Status LED: D4/GPIO2 (slow blink=connecting, solid=connected, fast blink=reconnecting)
 * 
 * Quick Setup:
 *   1. Update WiFi credentials in config.h
 *   2. Format SD card as FAT32
 *   3. Create folder structure: /config, /posts, /static, /templates, /logs
 *   4. Add routes.txt and redirects.txt to /config
 *   5. Upload sketch and insert SD card
 *   6. Access at http://[IP_ADDRESS] (see Serial Monitor for IP)
 * 
 * Admin Panel: http://[IP_ADDRESS]/admin (default password: admin123)
 * Traffic Logs: http://[IP_ADDRESS]/admin/logs
 * 
 * File Structure:
 *   - esp82_blog_server_modular.ino - Main entry point (this file)
 *   - config.h                      - Configuration and global variables
 *   - initializer.h                 - System initialization
 *   - logger.h                      - Traffic logging
 *   - parser.h                      - Template and content parsing
 *   - server.h                      - Web server route handlers
 *   - admin.h                       - Admin panel functionality
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SD.h>
#include <SPI.h>
#include <time.h>

// Include all module headers
#include "config.h"
#include "logger.h"
#include "parser.h"
#include "initializer.h"
#include "server.h"
#include "admin.h"

// ============================================================================
// GLOBAL VARIABLE DEFINITIONS
// ============================================================================

ESP8266WebServer server(80);

PostMapping* postMappings = nullptr;
int postMappingsCount = 0;

Redirection* redirections = nullptr;
int redirectionsCount = 0;

String logoBase64 = "";

// ============================================================================
// ROUTE SETUP (from server.h)
// ============================================================================

void setupRoutes() {
  server.collectHeaders("User-Agent");
  
  server.on("/", HTTP_GET, handleLandingPage);
  server.on("/page", HTTP_GET, handlePaginatedPage);
  server.on("/archive", HTTP_GET, handleArchive);
  server.on("/style.css", HTTP_GET, handleCSS);
  
  #if ENABLE_ADMIN_PANEL
  server.on("/admin", HTTP_GET, handleAdminPanel);
  server.on("/admin/files", HTTP_GET, handleAdminFiles);
  server.on("/admin/edit", HTTP_GET, handleAdminEdit);
  server.on("/admin/save", HTTP_POST, handleAdminSave);
  server.on("/admin/upload", HTTP_POST, []() {
    server.send(200);
  }, handleAdminUpload);
  server.on("/admin/delete", HTTP_POST, handleAdminDelete);
  server.on("/admin/reload", HTTP_POST, handleAdminReload);
  server.on("/admin/logs", HTTP_GET, handleAdminLogs);
  #endif
  
  server.onNotFound(handleRequest);
}

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\nESP8266 Blog Server Starting...");
  Serial.println("Version: Modular 2.0");
  
  // Initialize WiFi status LED
  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(WIFI_LED_PIN, HIGH); // LED off initially (active LOW)
  
  // Initialize SD card
  if (!initSDCard()) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  
  // Load configurations
  loadPostMappings();
  loadRedirections();
  loadLogo();
  
  // Connect to WiFi
  connectWiFi();
  
  // Sync time with NTP server
  syncTime();
  
  // Setup web server routes
  setupRoutes();
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Access blog at: http://");
  Serial.println(WiFi.localIP());
  Serial.println("Note: Admin panel works best with files <4KB. Large files should be edited via SD card.");
  
  Serial.println("\n=== Initialization Complete ===");
  Serial.printf("Posts loaded: %d\n", postMappingsCount);
  Serial.printf("Redirects loaded: %d\n", redirectionsCount);
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("================================\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Check WiFi connection status
  checkWiFiStatus();
  
  // Handle web requests
  server.handleClient();
}
