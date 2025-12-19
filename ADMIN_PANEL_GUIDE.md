# Admin Panel - Quick Reference

## 🎯 Overview

Your ESP8266 blog server now includes a built-in web-based admin panel for managing files without removing the SD card!

## ⚙️ Enable/Disable Feature

At the top of `esp82_blog_server.ino`, find:

```cpp
#define ENABLE_ADMIN_PANEL true  // Set to false to disable
const char* adminPassword = "admin123";  // Change this!
```

**To Disable:** Change `true` to `false` and re-upload the sketch.

## 🔐 Access & Login

1. **URL:** `http://[YOUR_ESP8266_IP]/admin`
2. **Default Password:** `admin123` 
3. **⚠️ IMPORTANT:** Change the password in the code before deploying!

## 📁 Features

### Browse Files
- **Posts:** `/admin/files?dir=/posts` - View all blog posts
- **Config:** `/admin/files?dir=/config` - Edit routes.txt and redirects.txt
- **Static:** `/admin/files?dir=/static` - Manage CSS, images, assets
- **Templates:** `/admin/files?dir=/templates` - Edit HTML templates

### Edit Files
- Click "✏️ Edit" button next to any text file
- Edit markdown posts directly in browser
- Modify CSS styling on-the-fly
- Update configuration files
- Changes saved immediately

### Upload Files
1. Enter path (e.g., `/posts/new-post.md` or `/static/img/photo.jpg`)
2. Choose file from your computer
3. Click "⬆️ Upload File"
4. File appears on SD card instantly

### Delete Files
- Click "🗑️ Delete" button next to any file
- Confirm deletion
- File removed from SD card

### Reload Configuration
- Click "🔄 Reload Configuration" button
- Reloads `routes.txt` and `redirects.txt`
- New posts and redirects appear immediately
- No need to restart ESP8266

## 🔒 Security

### Session Management
- Sessions expire after **1 hour** of inactivity
- Only **one active session** at a time
- Logout by closing browser or waiting for expiry

### Best Practices
✅ **DO:**
- Change default password immediately
- Use only on trusted home network
- Disable admin panel when not needed
- Keep passwords secure

❌ **DON'T:**
- Expose to public internet (no HTTPS/SSL)
- Use default password
- Leave enabled on production
- Share admin password

## 📱 Mobile Friendly

The admin panel works great on:
- Desktop browsers
- Tablets
- Smartphones
- Any device with a modern browser

## 🚀 Quick Workflow

### Adding a New Blog Post:

1. Go to `http://[IP]/admin`
2. Click "Upload New File"
3. Path: `/posts/my-new-post.md`
4. Choose your markdown file
5. Upload
6. Edit `/config/routes.txt` to add URL mapping:
   ```
   /posts/my-new-post|my-new-post.md|My New Post Title
   ```
7. Click "🔄 Reload Configuration"
8. Done! Post is live at `http://[IP]/posts/my-new-post`

### Editing CSS:

1. Go to `http://[IP]/admin`
2. Click "🎨 Static Files"
3. Find `style.css`
4. Click "✏️ Edit"
5. Make changes
6. Click "💾 Save Changes"
7. Refresh your blog to see changes instantly!

### Quick Config Edit:

1. Go to `http://[IP]/admin`
2. Click "⚙️ Config"
3. Edit `routes.txt` or `redirects.txt`
4. Save changes
5. Click "🔄 Reload Configuration"
6. Changes live immediately

## 🛠️ Technical Details

### Memory Usage
- Minimal impact: ~8-10KB additional RAM
- Only compiled when `ENABLE_ADMIN_PANEL` is true
- File operations stream from SD card (no RAM loading)

### File Size Limits
- Text files: Unlimited (streamed)
- Uploads: Handled in chunks (memory-efficient)
- Works great with your small files (markdown, CSS, configs)

### Supported File Types

**Editable in browser:**
- `.md` - Markdown posts
- `.txt` - Config files
- `.css` - Stylesheets
- `.html` - Templates

**Upload only:**
- `.jpg`, `.jpeg`, `.png`, `.gif` - Images
- Any other file type

## 🐛 Troubleshooting

**Can't access /admin:**
- Check that `ENABLE_ADMIN_PANEL` is `true`
- Re-upload sketch after changing setting
- Verify ESP8266 IP address

**Login fails:**
- Check password in code matches what you're typing
- Case-sensitive password
- Clear browser cookies

**Upload fails:**
- Check path starts with `/`
- Ensure directory exists (e.g., `/posts/`, `/static/img/`)
- File size reasonable (<1MB recommended)

**Changes don't appear:**
- Click "🔄 Reload Configuration" after editing config files
- Clear browser cache for CSS changes
- Check Serial Monitor for errors

## 💡 Pro Tips

1. **Backup Before Editing:** Keep local copies of important files before editing
2. **Test Changes:** Preview blog posts before adding to routes.txt
3. **Image Optimization:** Optimize images before uploading (use the Python script)
4. **Mobile Editing:** Edit posts on your phone/tablet for quick updates
5. **Session Management:** Admin panel remembers your login for 1 hour

## 🔄 Disable Admin Panel

When you don't need the admin panel:

1. Open `esp82_blog_server.ino`
2. Change line:
   ```cpp
   #define ENABLE_ADMIN_PANEL false  // Disabled
   ```
3. Upload to ESP8266
4. Admin routes no longer compiled (saves memory)
5. `/admin` URL returns 404

Re-enable anytime by changing back to `true` and re-uploading!

---

**Happy blogging! 🎉**
