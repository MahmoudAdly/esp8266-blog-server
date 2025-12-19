/*
 * parser.h - Template and Content Parsing
 * 
 * Functions for loading templates, parsing markdown, and extracting previews
 */

#ifndef PARSER_H
#define PARSER_H

#include <Arduino.h>
#include <SD.h>
#include "config.h"

// ============================================================================
// TEMPLATE LOADING
// ============================================================================

String loadTemplate(String templateName) {
  File templateFile = SD.open("/templates/" + templateName, FILE_READ);
  if (!templateFile) {
    Serial.println("Template not found: " + templateName);
    return "";
  }
  
  String content = "";
  while (templateFile.available()) {
    content += (char)templateFile.read();
  }
  templateFile.close();
  
  return content;
}

String loadPartial(String partialName) {
  File partialFile = SD.open("/templates/" + partialName, FILE_READ);
  if (!partialFile) {
    return "";
  }
  
  String content = "";
  while (partialFile.available()) {
    content += (char)partialFile.read();
  }
  partialFile.close();
  
  return content;
}

// ============================================================================
// TEMPLATE VARIABLE REPLACEMENT
// ============================================================================

String replaceTemplateVars(String html, String title, String content) {
  html.replace("{{TITLE}}", title);
  html.replace("{{CONTENT}}", content);
  html.replace("{{HEADER}}", loadPartial("header.html"));
  html.replace("{{FOOTER}}", loadPartial("footer.html"));
  html.replace("{{YEAR}}", String(2026));
  return html;
}

// ============================================================================
// POST PREVIEW EXTRACTION
// ============================================================================

String getPostPreview(String filename) {
  File postFile = SD.open("/posts/" + filename, FILE_READ);
  if (!postFile) return "Preview not available.";
  
  String preview = "";
  bool foundFirstParagraph = false;
  
  while (postFile.available()) {
    String line = postFile.readStringUntil('\n');
    line.trim();
    
    // Skip empty lines, headers, and images
    if (line.length() == 0 || line.startsWith("#") || line.startsWith("![")) {
      continue;
    }
    
    preview = line;
    foundFirstParagraph = true;
    break;
  }
  
  postFile.close();
  
  if (!foundFirstParagraph) return "Preview not available.";
  
  // Truncate to ~200 characters
  if (preview.length() > 200) {
    preview = preview.substring(0, 200);
    int lastSpace = preview.lastIndexOf(' ');
    if (lastSpace > 150) {
      preview = preview.substring(0, lastSpace);
    }
    preview += "...";
  }
  
  return preview;
}

// ============================================================================
// HTML ESCAPING
// ============================================================================

String escapeHtml(String text) {
  text.replace("&", "&amp;");
  text.replace("<", "&lt;");
  text.replace(">", "&gt;");
  text.replace("\"", "&quot;");
  text.replace("'", "&#39;");
  return text;
}

// ============================================================================
// CONTENT TYPE DETECTION
// ============================================================================

String getContentType(String filename) {
  if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".txt")) {
    return "text/plain";
  }
  return "application/octet-stream";
}

#endif // PARSER_H
