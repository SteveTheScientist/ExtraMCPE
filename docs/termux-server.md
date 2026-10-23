# Hosting a Dedicated Server on Android using Termux

This guide provides step-by-step instructions for compiling and running a dedicated multiplayer server for this game on an Android phone using **Termux**. No previous terminal experience is required.

---

## 1. What is a Dedicated Server and Phone Requirements?

### What is a Dedicated Server?
A **dedicated server** is a lightweight, command-line application that hosts the game world. It calculates world events, manages mobs and items, and keeps connected players in sync over a network. It runs without rendering 3D graphics or user interface menus ("headless"), making it ideal for hosting multiplayer games in the background.

### What Does Your Phone Need?
* **Storage Space:** At least **1.5 GB to 2.0 GB** of available internal storage space.
  * *Why?* Termux base system (~100 MB), C++ compiler toolchain (`clang`, `cmake`, `make` ~500 MB), source code repository (~100 MB), build output files (~500 MB), and world data.
* **Android Version:** **Android 7.0 (API level 24)** or newer.
* **Battery & Power:** Compiling C++ code and hosting network connections uses CPU power. Keep your phone connected to a charger and place it on a hard surface to dissipate heat.

---

## 2. Installing Termux and Preparing the System

### Step 2.1: Install Termux (from F-Droid or GitHub)
> **CRITICAL WARNING:** Do **NOT** install Termux from the Google Play Store. The Google Play Store version has been deprecated since 2020 and cannot access package repositories.

1. Download the Termux APK from **F-Droid** ([f-droid.org/packages/com.termux](https://f-droid.org/packages/com.termux/)) or from **Termux GitHub Releases** ([github.com/termux/termux-app/releases](https://github.com/termux/termux-app/releases)).
2. Open the downloaded `.apk` file on your Android device and install it (allow "Install from unknown sources" if prompted by Android).
3. Tap the **Termux** icon to open the app. You will see a black window with text welcoming you to Termux.

### Step 2.2: Setup Android Storage Access
In the Termux terminal, type the following command and press **Enter**:
```bash
termux-setup-storage
```
* **What you will see:** A system dialog titled *"Allow Termux to access photos, media, and files on your device?"*. Tap **Allow**.

### Step 2.3: Update System Packages
Type the following command to update Termux's software list and packages:
```bash
pkg update && pkg upgrade -y
```
* **What you will see:** Text scrolling rapidly with lines starting with `Get:`, `Unpacking`, and `Setting up`. If Termux asks a question about configuration files (e.g. `[default=N]`), press **Enter** to accept the default option. Once complete, you will return to the `$` prompt.

### Step 2.4: Install Required Development Tools
Install the compiler toolchain, git, cmake, make, and library dependencies:
```bash
pkg install -y git cmake clang make zlib libpng
```
* **What you will see:** Termux will download `clang` (C++ compiler), `cmake` (build configuration tool), `make` (build runner), and required libraries. When completed, you will see `Setting up clang...` followed by the `$` prompt.

---

## 3. Getting the Code and Building the Server Target

### Step 3.1: Clone the Code Repository
Clone the repository into Termux's internal home folder (`~`):
```bash
git clone --depth 1 https://github.com/SteveTheScientist/minecraft-pe-cb.git
```
* **Storage Note:** Always clone into Termux internal home directory (`~` / `/data/data/com.termux/files/home`). Do **NOT** clone into `/sdcard` or shared phone storage (`~/storage/shared`), because Android shared storage uses FAT/exFAT filesystems which lack Linux permissions and symlink support required for building C++ projects.
* **What you will see:**
  ```text
  Cloning into 'minecraft-pe-cb'...
  remote: Enumerating objects: ...
  remote: Compressing objects: 100% ...
  Receiving objects: 100% ...
  ```

### Step 3.2: Navigate to the Source Directory
```bash
cd minecraft-pe-cb
```
* **What you will see:** The prompt directory changes to `~/minecraft-pe-cb$`.

### Step 3.3: Configure CMake for Termux Server Build
Configure the build output directory and disable desktop GUI windowing libraries (`GLFW` Wayland/X11) that are not needed for a headless server:
```bash
cmake -B build -S . -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=OFF
```
* **What you will see:**
  ```text
  -- The C compiler identification is Clang ...
  -- The CXX compiler identification is Clang ...
  -- Building for target architecture: ...
  -- Configuring done
  -- Generating done
  -- Build files have been written to: /data/data/com.termux/files/home/minecraft-pe-cb/build
  ```

### Step 3.4: Build ONLY the Dedicated Server Target
Compile specifically the `MinecraftPE-server` target defined in `CMakeLists.txt`:
```bash
cmake --build build --target MinecraftPE-server
```
* **Compilation Duration:** This step takes approximately **5 to 15 minutes** depending on your phone's processor speed.
* **What you will see:** Progress indicators from `[ 1%]` to `[100%]`:
  ```text
  [ 10%] Building CXX object CMakeFiles/MinecraftPE-server.dir/src/main_dedicated.cpp.o
  [ 50%] Building CXX object CMakeFiles/MinecraftPE-server.dir/src/server/ServerLevel.cpp.o
  ...
  [100%] Linking CXX executable MinecraftPE-server
  [100%] Built target MinecraftPE-server
  ```
* **Finished Binary Location:** The compiled server binary is created at:
  `./build/MinecraftPE-server`

---

## 4. Starting and Configuring the Server

### Step 4.1: Initial Launch & Default Config Generation
Run the server executable for the first time:
```bash
./build/MinecraftPE-server
```
* **What you will see:**
  ```text
  // MCPE Server properties
  port=19132
  level-dir=level
  level-cachepath=.
  level-externalpath=.
  gamemode=survival
  level-name=level
  world-size=16

  Edit a config file first!
  ```
  The server automatically creates `server.properties`, `ops.txt`, and `banned-players.txt` on its first run, then exits back to the terminal prompt. This is expected behavior.

### Step 4.2: Edit Server Options (`server.properties`)
Edit `server.properties` using a text editor (e.g., `nano` or `micro`):
```bash
nano server.properties
```
You can adjust the following exact options in `server.properties`:

| Property Name | Default Value | Description |
| :--- | :--- | :--- |
| `port` | `19132` | The UDP network port players connect to. |
| `level-dir` | `level` | The directory name where world chunk data is saved. |
| `level-name` | `level` | The display name of the world shown in pause menus. |
| `gamemode` | `survival` | Default game mode (`survival` or `creative`). |
| `world-size` | `16` | Chunk cache width (`16` to `32`, corresponding to 256x256 chunks). |
| `level-cachepath` | `.` | Directory path for cache files. |
| `level-externalpath` | `.` | Base directory path for world storage files. |

> **Max Players:** The server code natively supports up to **16** concurrent player connections over RakNet UDP networking.
> **World Location:** World level files are stored at `<level-externalpath>/<level-dir>/` (e.g. `./level/`).

To save changes in `nano`: Press `Ctrl+O`, press `Enter`, then press `Ctrl+X` to exit.

### Step 4.3: Start the Server
Launch the server again:
```bash
./build/MinecraftPE-server
```
* **What you will see:**
  ```text
  Level Name: level
  Listening on port 19132
  Level has been generated in 0.412
  Level has been saved!
  ```
  The server is now live and waiting for player connections!

### Step 4.4: Stopping the Server Safely
To stop the server, press **`Ctrl` + `C`** in Termux.
* **What you will see:**
  ```text
  Signum caught: 2
  Level has been saved!
  Quit correctly
  ```
  The server catches the signal, saves all modified chunks, writes updated `ops.txt` and `banned-players.txt`, and exits cleanly.

---

## 5. Server Commands and Operator Privileges

### Operator Privileges (`ops.txt`)
* Players whose usernames are listed in `ops.txt` (one name per line) are given operator status.
* On server startup, `ops.txt` is loaded into the server's operator list.
* Operators can execute administrative chat commands directly in game.

### Server Chat Commands
Type commands into the in-game chat prompt starting with `/`:

* `/help` `[page]` - List available commands.
* `/op` `<player>` - Grant operator privileges to a player.
* `/deop` `<player>` - Revoke operator privileges.
* `/kick` `<player> [reason]` - Disconnect a player from the server.
* `/ban` `<player>` - Ban a player (saved to `banned-players.txt`).
* `/gamemode` `<0|1|survival|creative> [player]` - Set player game mode.
* `/difficulty` `<0|1|2|3|peaceful|easy|normal|hard>` - Set world difficulty.
* `/gamerule` `<rule> [value]` - View or change world game rules.
* `/time` `<set|add> <value>` - Set or advance world time.
* `/give` `<player> <item_id> [amount] [data]` - Give items to a player.
* `/clear` `[player]` - Clear player inventory.
* `/tp` `<player> <targetPlayer|x y z>` - Teleport a player.
* `/kill` `[player]` - Kill a player character.
* `/spawnpoint` `[player] [x y z]` - Set individual spawn point.
* `/setworldspawn` `[x y z]` - Set default world spawn location.
* `/seed` - Display the world seed.
* `/list` - List all connected players.
* `/say` `<message>` - Broadcast a server message.
* `/me` `<action>` - Broadcast a player action.
* `/msg` `<player> <message>` (also `/tell`, `/w`) - Send private message.

---

## 6. How Friends Can Join Your Server

### Method A: Same Local Wi-Fi Network or Phone Hotspot
If your friends are on the same Wi-Fi network or connected to your phone's Wi-Fi Hotspot:

1. **Find your phone's Local IP Address in Termux:**
   Run:
   ```bash
   ip addr show
   ```
   * **What you will see:** Look for the section labeled `wlan0` (Wi-Fi) or `ap0` / `rndis0` (Hotspot). Locate the line starting with `inet` (e.g., `inet 192.168.1.150/24`). Your local IP is `192.168.1.150`.

2. **Connect on the Game Client:**
   * Open Minecraft Pocket Edition on another phone or device on the same network.
   * Tap **Play** -> **External** tab (or **Servers** -> **Add Server**).
   * **Server Name:** Any name (e.g. `My Termux Server`).
   * **Server Address:** Enter your phone's local IP address (e.g., `192.168.1.150`).
   * **Port:** `19132` (or your configured port).
   * Tap **Play** or **Save** and select the server to join.

---

### Method B: Over the Internet (Free Tunnels)

#### Option 1: Tailscale (Virtual Mesh Network) *(Untested on physical mobile device)*
1. Install **Tailscale** from the Google Play Store / App Store on your host phone and your friends' devices.
2. Log into the same Tailscale account on all devices.
3. Friends connect in MCPE using your host phone's Tailscale IP address (e.g., `100.x.y.z`) and port `19132`.

#### Option 2: Playit.gg (UDP Port Forwarding Agent) *(Untested on physical mobile device)*
1. Download the ARM/Linux playit CLI binary in Termux.
2. Run playit agent to establish a UDP tunnel pointing to local port `19132`.
3. Share the generated domain and mapped port with your friends.

> **SAFETY WARNING:** Never share your personal home router's public IP address on public internet forums. Only invite trusted friends.

---

## 7. Keeping the Server Running in the Background

Android aggressive power management can kill background apps when the screen turns off. To keep the server running:

1. **Acquire Termux Wake Lock:**
   Run the following command in Termux before starting the server:
   ```bash
   termux-wake-lock
   ```
   * **What you will see:** A persistent Android notification appearing in your notification shade: *"Termux - Wake lock held"*.
   * To release the lock when finished, run `termux-wake-unlock`.

2. **Disable Android Battery Optimization for Termux:**
   * Open Android System **Settings** -> **Apps** -> **Termux**.
   * Tap **Battery** or **Battery Saver**.
   * Select **Unrestricted** (or **Don't Optimize**).

---

## 8. Updating the Server and Backing Up Your World

### Updating the Server
When new updates are pushed to the codebase:
1. Stop the server safely (`Ctrl+C`).
2. Pull the latest source changes:
   ```bash
   git pull
   ```
3. Recompile the server target:
   ```bash
   cmake --build build --target MinecraftPE-server
   ```
4. Restart the server:
   ```bash
   ./build/MinecraftPE-server
   ```

### Backing Up Your World
To create a backup archive of your world data:
```bash
tar -czvf world_backup.tar.gz level/
```
To copy the backup to your phone's public Downloads folder:
```bash
cp world_backup.tar.gz ~/storage/downloads/
```

---

## 9. Troubleshooting Matrix

| Problem | Cause | Solution |
| :--- | :--- | :--- |
| `pkg update` fails with repository errors | Installed Termux from Google Play Store | Uninstall Play Store version; install Termux from F-Droid or GitHub Releases. |
| `cmake: command not found` | Packages not installed | Run `pkg install -y cmake clang make git`. |
| Build fails / process `Killed` | Phone ran out of RAM during parallel build | Limit build thread count: run `cmake --build build --target MinecraftPE-server -j2`. |
| `Failed binding socket - 2` | Port `19132` is already in use by another process | Stop other server instances: `pkill -f MinecraftPE-server`, or change `port=` in `server.properties`. |
| `permission denied` when running binary | Binary lost executable permission | Run `chmod +x build/MinecraftPE-server`. |
| Friends cannot connect on Wi-Fi | Devices on different networks or wrong IP | Verify host phone IP with `ip addr show`. Ensure AP isolation is turned off on router. |
| Server stops when screen turns off | Android suspended Termux in background | Run `termux-wake-lock` and set Termux battery setting to "Unrestricted". |
| `No space left on device` | Storage full | Clean build directory (`rm -rf build`) or run `pkg clean`. |

---

## 10. Safety and Hardware Limits

* **Hardware Boundaries:** Mobile phone processors are designed for battery efficiency. Hosting 2–4 players works smoothly on mid-range phones. Hosting 8–16 players requires a higher-end device with good cooling.
* **Privacy:** Your IP address reveals your general location. Share connection details only with friends you trust.
* **Wi-Fi vs Mobile Data:** Hosting over Wi-Fi provides lower latency and avoids Cellular Carrier NAT (CGNAT) blocks.
