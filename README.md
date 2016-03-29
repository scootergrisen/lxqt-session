lxqt-session
===

This repository includes user session related tools/daemons for **LXQt**:
- **lxqt-session** - base session process/manager starting window manager and all needed modules
- **lxqt-session-config** - GUI conriguration tool for session (default and autostart applications settings, environment settings, etc.)
- **lxqt-leave** - GUI for session logout, reboot, shutdown, suspend, hibernate and lock screen

(CMake) Build parameters
---
- **UPDATE_TRANSLATIONS** Update source translation translations/\*.ts files (default OFF)
- **WITH_LIBUDEV_MONITOR** Use libudev to monitor spawning of input devices in lxqt-session (default ON)
