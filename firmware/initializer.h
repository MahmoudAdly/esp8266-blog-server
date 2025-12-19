/*
 * initializer.h - System Initialization Functions
 * 
 * Functions for initializing SD card, WiFi, NTP sync, and loading configurations
 */

#ifndef INITIALIZER_H
#define INITIALIZER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SD.h>
#include <time.h>
#include "config.h"

// Forward declarations
void syncTime();

// ============================================================================
// SD CARD INITIALIZATION
// ============================================================================

bool initSDCard() {
  Serial.println("Initializing SD card...");
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card Mount Failed");
    return false;
  }
  
  Serial.println("SD Card initialized successfully");
  return true;
}

// ============================================================================
// WIFI CONNECTION
// ============================================================================

void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    // Blink LED while connecting
    digitalWrite(WIFI_LED_PIN, !digitalRead(WIFI_LED_PIN));
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // LED solid on when connected
    digitalWrite(WIFI_LED_PIN, LOW); // Active LOW
  } else {
    Serial.println("\nWiFi connection failed!");
    // LED off when failed
    digitalWrite(WIFI_LED_PIN, HIGH);
  }
}

void checkWiFiStatus() {
  static unsigned long lastCheck = 0;
  static unsigned long lastBlink = 0;
  static bool ledState = false;
  
  // Check WiFi status every 5 seconds
  if (millis() - lastCheck > 5000) {
    lastCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi connection lost! Attempting to reconnect...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      
      // Wait up to 10 seconds for reconnection
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi reconnected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(WIFI_LED_PIN, LOW); // LED on
        
        // Re-sync time after reconnection
        syncTime();
      }
    } else {
      // Connected - LED solid on
      digitalWrite(WIFI_LED_PIN, LOW);
    }
  }
  
  // Quick blink when disconnected (every 200ms)
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastBlink > 200) {
      lastBlink = millis();
      ledState = !ledState;
      digitalWrite(WIFI_LED_PIN, ledState ? LOW : HIGH);
    }
  }
}

// ============================================================================
// TIME SYNCHRONIZATION
// ============================================================================

void syncTime() {
  Serial.println("Syncing time with NTP server...");
  
  // Configure NTP client
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Wait for time to be set (max 10 seconds)
  int attempts = 0;
  time_t now = time(nullptr);
  while (now < 1000000000 && attempts < 20) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    attempts++;
  }
  
  if (now > 1000000000) {
    Serial.println("\nTime synced successfully!");
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.println(asctime(&timeinfo));
  } else {
    Serial.println("\nTime sync failed - will use relative timestamps");
  }
}

// ============================================================================
// CONFIGURATION LOADING
// ============================================================================

void loadPostMappings() {
  Serial.println("Loading post mappings...");
  
  File configFile = SD.open("/config/routes.txt", FILE_READ);
  if (!configFile) {
    Serial.println("Failed to open /config/routes.txt");
    return;
  }
  
  // Count lines
  int lineCount = 0;
  while (configFile.available()) {
    String line = configFile.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && !line.startsWith("#")) {
      lineCount++;
    }
  }
  
  // Allocate array
  postMappings = new PostMapping[lineCount];
  postMappingsCount = 0;
  
  // Parse lines
  configFile.seek(0);
  while (configFile.available() && postMappingsCount < lineCount) {
    String line = configFile.readStringUntil('\n');
    line.trim();
    
    if (line.length() > 0 && !line.startsWith("#")) {
      int pipe1 = line.indexOf('|');
      int pipe2 = line.indexOf('|', pipe1 + 1);
      
      if (pipe1 > 0 && pipe2 > pipe1) {
        postMappings[postMappingsCount].urlPath = line.substring(0, pipe1);
        postMappings[postMappingsCount].fileName = line.substring(pipe1 + 1, pipe2);
        postMappings[postMappingsCount].title = line.substring(pipe2 + 1);
        postMappingsCount++;
      }
    }
  }
  
  configFile.close();
  Serial.printf("Loaded %d post mappings\n", postMappingsCount);
}

void loadRedirections() {
  Serial.println("Loading redirections...");
  
  File redirectFile = SD.open("/config/redirects.txt", FILE_READ);
  if (!redirectFile) {
    Serial.println("No /config/redirects.txt found");
    return;
  }
  
  // Count lines
  int lineCount = 0;
  while (redirectFile.available()) {
    String line = redirectFile.readStringUntil('\n');
    line.trim();
    if (line.length() > 0 && !line.startsWith("#")) {
      lineCount++;
    }
  }
  
  // Allocate array
  redirections = new Redirection[lineCount];
  redirectionsCount = 0;
  
  // Parse lines
  redirectFile.seek(0);
  while (redirectFile.available() && redirectionsCount < lineCount) {
    String line = redirectFile.readStringUntil('\n');
    line.trim();
    
    if (line.length() > 0 && !line.startsWith("#")) {
      int pipePos = line.indexOf('|');
      if (pipePos > 0) {
        redirections[redirectionsCount].fromPath = line.substring(0, pipePos);
        redirections[redirectionsCount].toPath = line.substring(pipePos + 1);
        redirectionsCount++;
      }
    }
  }
  
  redirectFile.close();
  Serial.printf("Loaded %d redirections\n", redirectionsCount);
}

void loadLogo() {
  Serial.println("Loading logo...");
  
  File logoFile = SD.open("/static/logo.png", FILE_READ);
  if (!logoFile) {
    Serial.println("No /static/logo.png found");
    return;
  }
  
  logoFile.close();
  Serial.println("Logo loaded");
}

void reloadConfigurations() {
  // Free existing allocations
  if (postMappings != nullptr) {
    delete[] postMappings;
    postMappings = nullptr;
  }
  if (redirections != nullptr) {
    delete[] redirections;
    redirections = nullptr;
  }
  
  // Reload
  loadPostMappings();
  loadRedirections();
}

#endif // INITIALIZER_H
