/*
 * logger.h - Traffic Logging System
 * 
 * Functions for logging HTTP requests with NTP timestamps and log rotation
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <SD.h>
#include <time.h>
#include "config.h"

#if ENABLE_TRAFFIC_LOG

void logTraffic(int statusCode) {
  // Get request details
  String clientIP = server.client().remoteIP().toString();
  String method = (server.method() == HTTP_GET) ? "GET" : "POST";
  String uri = server.uri();
  String userAgent = server.header("User-Agent");
  
  // Handle missing or empty User-Agent
  if (userAgent.length() == 0) {
    userAgent = "-";  // Standard log format for missing field
  } else if (userAgent.length() > 60) {
    // Trim if too long
    userAgent = userAgent.substring(0, 57) + "...";
  }
  
  // Create timestamp (real time if available, otherwise relative uptime)
  String timestamp;
  time_t now = time(nullptr);
  
  if (now > 1000000000) {
    // We have real time from NTP
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    timestamp = String(buffer);
  } else {
    // Fall back to relative uptime
    unsigned long uptime = millis() / 1000;
    unsigned long days = uptime / 86400;
    unsigned long hours = (uptime % 86400) / 3600;
    unsigned long mins = (uptime % 3600) / 60;
    unsigned long secs = uptime % 60;
    timestamp = String(days) + "d " + String(hours) + "h " + String(mins) + "m " + String(secs) + "s";
  }
  
  // Build log entry in Apache Combined Log Format
  String logEntry = "[" + timestamp + "] ";
  logEntry += clientIP + " - ";
  logEntry += method + " " + uri + " - ";
  logEntry += String(statusCode) + " - ";
  logEntry += "\"" + userAgent + "\"\n";
  
  // Print to serial for real-time monitoring
  Serial.print("ACCESS: ");
  Serial.print(logEntry);
  
  // Check log file size and rotate if needed
  File logFile = SD.open("/logs/access.log", FILE_READ);
  if (logFile) {
    size_t fileSize = logFile.size();
    logFile.close();
    
    if (fileSize > MAX_LOG_SIZE) {
      // Rotate log: rename current to .old
      SD.remove("/logs/access.old");
      File current = SD.open("/logs/access.log", FILE_READ);
      File old = SD.open("/logs/access.old", FILE_WRITE);
      if (current && old) {
        while (current.available()) {
          old.write(current.read());
        }
        current.close();
        old.close();
      }
      SD.remove("/logs/access.log");
      Serial.println("LOG: Rotated access.log to access.old");
    }
  }
  
  // Append to log file
  logFile = SD.open("/logs/access.log", FILE_WRITE);
  if (logFile) {
    logFile.print(logEntry);
    logFile.close();
  } else {
    Serial.println("ERROR: Could not open log file for writing");
  }
}

#endif // ENABLE_TRAFFIC_LOG

#endif // LOGGER_H
