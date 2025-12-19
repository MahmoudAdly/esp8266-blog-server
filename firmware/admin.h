/*
 * admin.h - Admin Panel Handlers
 * 
 * Functions for authentication and admin panel operations
 */

#ifndef ADMIN_H
#define ADMIN_H

#if ENABLE_ADMIN_PANEL

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <SD.h>
#include "config.h"
#include "parser.h"
#include "initializer.h"

// ============================================================================
// AUTHENTICATION
// ============================================================================

String decodeBase64(String input) {
  const char* base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  String output = "";
  int val = 0;
  int bits = -8;
  
  for (unsigned int i = 0; i < input.length(); i++) {
    char c = input[i];
    if (c == '=') break;
    
    const char* p = strchr(base64Chars, c);
    if (!p) continue;
    
    val = (val << 6) + (p - base64Chars);
    bits += 6;
    
    if (bits >= 0) {
      output += char((val >> bits) & 0xFF);
      bits -= 8;
    }
  }
  
  return output;
}

bool checkAuth() {
  if (!server.hasHeader("Authorization")) {
    Serial.println("No Authorization header found");
    return false;
  }
  
  String authHeader = server.header("Authorization");
  Serial.println("Authorization header: " + authHeader);
  
  // Check if it's Basic auth
  if (!authHeader.startsWith("Basic ")) {
    Serial.println("Not Basic auth");
    return false;
  }
  
  // Extract base64 credentials (after "Basic ")
  String base64Creds = authHeader.substring(6);
  Serial.println("Base64 credentials received");
  
  // Decode base64
  String decoded = decodeBase64(base64Creds);
  Serial.println("Decoded: " + decoded);
  
  // Expected format: "admin:password"
  String expected = "admin:" + String(adminPassword);
  
  if (decoded == expected) {
    Serial.println("Auth successful!");
    return true;
  }
  
  Serial.println("Auth failed - credentials mismatch");
  return false;
}

void requestAuth() {
  server.sendHeader("WWW-Authenticate", "Basic realm=\"Admin Panel\"");
  server.send(401, "text/html", "<h1>401 Unauthorized</h1><p>Authentication required.</p>");
}

// ============================================================================
// ADMIN PANEL HANDLERS
// ============================================================================

void handleAdminPanel() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  String html = loadTemplate("admin.html");
  server.send(200, "text/html", html);
}

void handleAdminFiles() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  String dir = server.arg("dir");
  if (dir == "") dir = "/posts";
  
  File root = SD.open(dir);
  if (!root || !root.isDirectory()) {
    server.send(404, "text/html", "<h1>Error: Cannot open directory</h1>");
    return;
  }
  
  String fileListHtml = "";
  int fileCount = 0;
  
  Serial.println("=== Listing files in directory: " + dir + " ===");
  root.rewindDirectory();
  
  while (true) {
    File file = root.openNextFile();
    if (!file) break;
    
    if (file.isDirectory()) {
      file.close();
      continue;
    }
    
    String fileName = String(file.name());
    int lastSlash = fileName.lastIndexOf('/');
    if (lastSlash != -1) {
      fileName = fileName.substring(lastSlash + 1);
    }
    
    if (fileName.startsWith(".")) {
      file.close();
      continue;
    }
    
    String fullPath = dir;
    if (!fullPath.endsWith("/")) fullPath += "/";
    fullPath += fileName;
    
    size_t fileSize = file.size();
    
    fileListHtml += "<li class='file-item'>";
    fileListHtml += "<span class='file-name'>" + fileName + " (" + String(fileSize) + " bytes)</span>";
    fileListHtml += "<div class='actions'>";
    
    if (fileName.endsWith(".md") || fileName.endsWith(".txt") || 
        fileName.endsWith(".css") || fileName.endsWith(".html")) {
      fileListHtml += "<a href='/admin/edit?file=" + fullPath + "' class='btn'>✏️ Edit</a>";
    }
    
    fileListHtml += "<form method='POST' action='/admin/delete' style='display:inline;margin:0'>";
    fileListHtml += "<input type='hidden' name='file' value='" + fullPath + "'>";
    fileListHtml += "<button type='submit' class='btn btn-danger' onclick='return confirm(\"Delete " + fileName + "?\")'>🗑️ Delete</button>";
    fileListHtml += "</form>";
    fileListHtml += "</div></li>";
    
    fileCount++;
    file.close();
  }
  
  if (fileCount == 0) {
    fileListHtml = "<li class='file-item'><em>No files found in this directory</em></li>";
  }
  
  String html = loadTemplate("admin-files.html");
  html.replace("{{DIRECTORY}}", dir);
  html.replace("{{FILE_LIST}}", fileListHtml);
  
  server.send(200, "text/html", html);
}

void handleAdminEdit() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  String filePath = server.arg("file");
  
  File file = SD.open(filePath, FILE_READ);
  if (!file) {
    server.send(404, "text/html", "<h1>File not found</h1>");
    return;
  }
  
  String content = "";
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();
  
  String escapedContent = escapeHtml(content);
  
  String html = loadTemplate("admin-edit.html");
  html.replace("{{FILE_PATH}}", filePath);
  html.replace("{{CONTENT}}", escapedContent);
  
  String sizeWarning = String(content.length()) + " characters";
  if (content.length() > 4000) {
    sizeWarning += " ⚠️ WARNING: Files >4KB may fail to save due to ESP8266 memory limits!";
  }
  html.replace("{{CONTENT_LENGTH}}", sizeWarning);
  
  server.send(200, "text/html", html);
}

void handleAdminSave() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  String filePath = server.arg("file");
  String content = server.arg("content");
  
  Serial.println("=== Admin Save Debug ===");
  Serial.println("File: " + filePath);
  Serial.println("Content length received: " + String(content.length()));
  
  if (content.length() == 0) {
    String html = loadTemplate("admin-save-error.html");
    html.replace("{{FILE_PATH}}", filePath);
    server.send(500, "text/html", html);
    return;
  }
  
  if (SD.exists(filePath)) {
    if (!SD.remove(filePath)) {
      server.send(500, "text/html", "<h1>Error: Cannot delete old file</h1>");
      return;
    }
  }
  
  File file = SD.open(filePath, FILE_WRITE);
  if (!file) {
    server.send(500, "text/html", "<h1>Error: Cannot open file for writing</h1>");
    return;
  }
  
  size_t bytesWritten = file.print(content);
  file.close();
  
  String html = loadTemplate("admin-success.html");
  html.replace("{{REDIRECT_URL}}", "/admin");
  html.replace("{{ICON}}", "✅");
  html.replace("{{MESSAGE}}", "File Saved Successfully!");
  html.replace("{{DETAILS}}", "<p>" + filePath + "</p><p>" + String(bytesWritten) + " bytes written</p>");
  
  server.send(200, "text/html", html);
}

void handleAdminUpload() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  HTTPUpload& upload = server.upload();
  static File uploadFile;
  
  if (upload.status == UPLOAD_FILE_START) {
    String path = server.arg("path");
    uploadFile = SD.open(path, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      
      String html = loadTemplate("admin-success.html");
      html.replace("{{REDIRECT_URL}}", "/admin");
      html.replace("{{ICON}}", "✅");
      html.replace("{{MESSAGE}}", "File Uploaded Successfully!");
      html.replace("{{DETAILS}}", "<p>" + String(upload.totalSize) + " bytes</p>");
      
      server.send(200, "text/html", html);
    } else {
      server.send(500, "text/html", "<h1>Upload Failed</h1>");
    }
  }
}

void handleAdminDelete() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  String filePath = server.arg("file");
  
  if (SD.remove(filePath)) {
    String html = loadTemplate("admin-success.html");
    html.replace("{{REDIRECT_URL}}", "/admin");
    html.replace("{{ICON}}", "✅");
    html.replace("{{MESSAGE}}", "File Deleted Successfully!");
    html.replace("{{DETAILS}}", "<p>" + filePath + "</p>");
    
    server.send(200, "text/html", html);
  } else {
    server.send(500, "text/html", "<h1>Delete Failed</h1>");
  }
}

void handleAdminReload() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  reloadConfigurations();
  
  String html = loadTemplate("admin-success.html");
  html.replace("{{REDIRECT_URL}}", "/admin");
  html.replace("{{ICON}}", "🔄");
  html.replace("{{MESSAGE}}", "Configuration Reloaded!");
  html.replace("{{DETAILS}}", "<p>Posts: " + String(postMappingsCount) + "</p><p>Redirects: " + String(redirectionsCount) + "</p>");
  
  server.send(200, "text/html", html);
}

void handleAdminLogs() {
  if (!checkAuth()) {
    requestAuth();
    return;
  }
  
  File logFile = SD.open("/logs/access.log", FILE_READ);
  
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1.0'>";
  html += "<title>Access Logs</title>";
  html += "<style>body{font-family:monospace;margin:0;padding:20px;background:#1e1e1e;color:#d4d4d4}";
  html += ".header{background:#333;color:white;padding:20px;margin:-20px -20px 20px}";
  html += ".container{max-width:1200px;margin:0 auto;background:#2d2d2d;padding:30px;border-radius:8px}";
  html += ".log-entry{padding:8px;border-bottom:1px solid #444;font-size:14px;line-height:1.6}";
  html += ".log-entry:hover{background:#333}";
  html += ".btn{background:#0066cc;color:white;padding:10px 20px;text-decoration:none;border-radius:4px;display:inline-block;margin:10px 5px 0 0}";
  html += ".info{color:#888;margin-bottom:20px}</style></head><body>";
  html += "<div class='header'><h1>📊 Access Logs</h1></div>";
  html += "<div class='container'>";
  html += "<a href='/admin' class='btn'>← Back to Admin</a>";
  html += "<a href='/admin/files?dir=/logs' class='btn'>📁 Manage Logs</a>";
  
  if (logFile) {
    size_t fileSize = logFile.size();
    int lineCount = 0;
    
    while (logFile.available()) {
      logFile.readStringUntil('\n');
      lineCount++;
    }
    logFile.close();
    
    html += "<div class='info'><br>Log file size: " + String(fileSize) + " bytes | ";
    html += "Total requests: " + String(lineCount) + "</div>";
    
    logFile = SD.open("/logs/access.log", FILE_READ);
    html += "<div style='max-height:600px;overflow-y:auto'>";
    
    int displayCount = 0;
    int skipLines = (lineCount > 100) ? lineCount - 100 : 0;
    int currentLine = 0;
    
    while (logFile.available()) {
      String line = logFile.readStringUntil('\n');
      if (currentLine >= skipLines) {
        html += "<div class='log-entry'>" + line + "</div>";
        displayCount++;
      }
      currentLine++;
    }
    html += "</div>";
    logFile.close();
    
    if (skipLines > 0) {
      html += "<div class='info'><br>Showing last " + String(displayCount) + " entries (of " + String(lineCount) + " total)</div>";
    }
  } else {
    html += "<p style='color:#888;margin-top:30px'>No log file found. Logs will appear here once traffic starts.</p>";
  }
  
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

#endif // ENABLE_ADMIN_PANEL

#endif // ADMIN_H
