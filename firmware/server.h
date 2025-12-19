/*
 * server.h - Web Server Route Handlers
 * 
 * Functions for handling HTTP requests, serving pages, and routing
 */

#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <SD.h>
#include "config.h"
#include "parser.h"
#include "logger.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void setupRoutes();
void serve404();
void servePost(String filename, String title);
void serveStaticFile(String path);
void servePaginatedPosts(int page);

// ============================================================================
// REQUEST ROUTING
// ============================================================================

void handleRequest() {
  String uri = server.uri();
  
  // Check redirections
  for (int i = 0; i < redirectionsCount; i++) {
    if (uri == redirections[i].fromPath) {
      server.sendHeader("Location", redirections[i].toPath, true);
      server.send(302, "text/plain", "");
      return;
    }
  }
  
  // Static files
  if (uri.startsWith("/static/")) {
    serveStaticFile(uri);
    return;
  }
  
  // Post mappings
  for (int i = 0; i < postMappingsCount; i++) {
    if (uri == postMappings[i].urlPath) {
      servePost(postMappings[i].fileName, postMappings[i].title);
      return;
    }
  }
  
  // 404
  serve404();
}

// ============================================================================
// PAGE HANDLERS
// ============================================================================

void handleLandingPage() {
  servePaginatedPosts(0);
}

void handlePaginatedPage() {
  int page = 0;
  if (server.hasArg("p")) {
    page = server.arg("p").toInt();
  }
  servePaginatedPosts(page);
}

void servePaginatedPosts(int page) {
  // Count blog posts
  int totalBlogPosts = 0;
  for (int i = 0; i < postMappingsCount; i++) {
    if (postMappings[i].urlPath.startsWith("/posts/")) {
      totalBlogPosts++;
    }
  }
  
  int startIdx = page * POSTS_PER_PAGE;
  int totalPages = (totalBlogPosts + POSTS_PER_PAGE - 1) / POSTS_PER_PAGE;
  
  if (startIdx >= totalBlogPosts || page < 0) {
    serve404();
    return;
  }
  
  // Build posts HTML
  String postsHtml = "";
  int postsDisplayed = 0;
  int postsSkipped = 0;
  
  for (int i = 0; i < postMappingsCount && postsDisplayed < POSTS_PER_PAGE; i++) {
    if (!postMappings[i].urlPath.startsWith("/posts/")) {
      continue;
    }
    
    if (postsSkipped < startIdx) {
      postsSkipped++;
      continue;
    }
    
    postsHtml += "<div class='post-preview'>";
    postsHtml += "<h2><a href='" + postMappings[i].urlPath + "'>" + postMappings[i].title + "</a></h2>";
    postsHtml += "<p>" + getPostPreview(postMappings[i].fileName) + "</p>";
    postsHtml += "</div>";
    
    postsDisplayed++;
  }
  
  // Build pagination HTML
  String paginationHtml = "<div class='pagination'>";
  if (page > 0) {
    paginationHtml += "<a href='/page?p=" + String(page - 1) + "'>« Previous</a> ";
  }
  paginationHtml += "Page " + String(page + 1) + " of " + String(totalPages);
  if (page < totalPages - 1) {
    paginationHtml += " <a href='/page?p=" + String(page + 1) + "'>Next »</a>";
  }
  paginationHtml += "</div>";
  
  // Load and populate template
  String html = loadTemplate("home.html");
  html.replace("{{TITLE}}", "My Blog - Home");
  html.replace("{{POSTS}}", postsHtml);
  html.replace("{{PAGINATION}}", paginationHtml);
  
  #if ENABLE_TRAFFIC_LOG
  logTraffic(200);
  #endif
  
  server.send(200, "text/html", html);
}

void handleArchive() {
  int postCount = 0;
  for (int i = 0; i < postMappingsCount; i++) {
    if (postMappings[i].urlPath.startsWith("/posts/")) {
      postCount++;
    }
  }
  
  String postListHtml = "";
  for (int i = 0; i < postMappingsCount; i++) {
    if (postMappings[i].urlPath.startsWith("/posts/")) {
      postListHtml += "<li><a href='" + postMappings[i].urlPath + "'>" + postMappings[i].title + "</a></li>";
    }
  }
  
  String html = loadTemplate("archive.html");
  html.replace("{{TITLE}}", "Archive - All Posts");
  html.replace("{{POST_COUNT}}", String(postCount));
  html.replace("{{POST_LIST}}", postListHtml);
  
  #if ENABLE_TRAFFIC_LOG
  logTraffic(200);
  #endif
  
  server.send(200, "text/html", html);
}

// ============================================================================
// POST SERVING
// ============================================================================

void servePost(String filename, String title) {
  File postFile = SD.open("/posts/" + filename, FILE_READ);
  if (!postFile) {
    serve404();
    return;
  }
  
  // Read and escape HTML
  String content = "";
  while (postFile.available()) {
    char c = postFile.read();
    if (c == '<') content += "&lt;";
    else if (c == '>') content += "&gt;";
    else if (c == '&') content += "&amp;";
    else content += c;
  }
  postFile.close();
  
  // Load and populate template
  String html = loadTemplate("post.html");
  html.replace("{{TITLE}}", title);
  html.replace("{{POST_TITLE}}", title);
  html.replace("{{CONTENT}}", content);
  
  #if ENABLE_TRAFFIC_LOG
  logTraffic(200);
  #endif
  
  server.send(200, "text/html", html);
}

// ============================================================================
// STATIC FILE SERVING
// ============================================================================

void serveStaticFile(String path) {
  File file = SD.open(path, FILE_READ);
  if (!file) {
    serve404();
    return;
  }
  
  size_t fileSize = file.size();
  if (fileSize > 200000) {
    Serial.println("WARNING: Serving large file (" + String(fileSize) + " bytes): " + path);
  }
  
  String contentType = getContentType(path);
  server.sendHeader("Cache-Control", "max-age=86400");
  
  #if ENABLE_TRAFFIC_LOG
  logTraffic(200);
  #endif
  
  server.streamFile(file, contentType);
  file.close();
}

void handleCSS() {
  File cssFile = SD.open("/static/style.css", FILE_READ);
  if (cssFile) {
    #if ENABLE_TRAFFIC_LOG
    logTraffic(200);
    #endif
    server.streamFile(cssFile, "text/css");
    cssFile.close();
  } else {
    #if ENABLE_TRAFFIC_LOG
    logTraffic(404);
    #endif
    server.send(404, "text/plain", "CSS file not found");
  }
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

void serve404() {
  #if ENABLE_TRAFFIC_LOG
  logTraffic(404);
  #endif
  
  String html = loadTemplate("404.html");
  server.send(404, "text/html", html);
}

#endif // SERVER_H
