LOGS DIRECTORY
==============

This directory stores traffic logs for the ESP8266 blog server.

Files:
------
- access.log     : Current access log (auto-created)
- access.old     : Rotated backup log (auto-created when access.log exceeds 500KB)

Log Format:
-----------
[TIMESTAMP] IP_ADDRESS - METHOD URL - STATUS_CODE - "USER_AGENT"

Example:
[2025-12-08 14:23:45] 192.168.1.100 - GET /posts/welcome - 200 - "Mozilla/5.0..."

Features:
---------
- Automatic log rotation at 500KB
- Real timestamps (synced with NTP on boot)
- Fallback to relative uptime if NTP fails
- View logs at: http://[IP_ADDRESS]/admin/logs

Notes:
------
- Logs are created automatically on first access
- Old logs are kept as access.old
- You can safely delete logs through the admin panel
- Logs can be disabled by setting ENABLE_TRAFFIC_LOG to false in the sketch
