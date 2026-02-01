# ChatGPT Desktop

A native Linux desktop client for ChatGPT using the official OpenAI API.

![ChatGPT Desktop](resources/icons/boh-chat.png)

⚠️ Early release (v0.1 / v1-internal)
This project is under active development. Interfaces, packaging, and distribution are stabilizing.

## Features

✅ **Official OpenAI API Integration** - Secure, stable, and compliant  
✅ **Full Conversation Context** - AI remembers your chat history  
✅ **Multi-Thread Management** - Organize conversations with create/rename/delete  
✅ **6 OpenAI Models** - From GPT-3.5-turbo to GPT-4o and o1  
✅ **Persistent Storage** - SQLite database for chat history  
✅ **Modern Qt5 UI** - Clean, responsive interface  
✅ **Secure API Key Storage** - Keys stored in user config, never in code  
✅ **Context Menu** - Right-click for quick actions  
✅ **Visual Feedback** - Status messages and loading states  

## Supported Models

- **gpt-4o** - Latest flagship model (128K context)
- **gpt-4o-mini** - Faster & cheaper (128K context) ⭐ Default
- **o1** - Best for reasoning, math, and coding
- **o1-mini** - Faster reasoning model
- **gpt-4-turbo** - Previous generation (128K context)
- **gpt-3.5-turbo** - Fast & economical (16K context)

## Requirements

### Runtime Dependencies
- Qt5 (>= 5.12)
  - Qt5Core
  - Qt5Widgets
  - Qt5Network
  - Qt5Sql
- SQLite3
- OpenSSL (for HTTPS)

### Build Dependencies
- CMake (>= 3.16)
- C++17 compatible compiler (GCC 7+, Clang 5+)
- Qt5 development packages

## Installation

### openSUSE Leap / Tumbleweed

#### Install from RPM (Coming Soon)
```bash
sudo zypper install chatgpt-desktop
```

#### Build from Source
```bash
# Install dependencies
sudo zypper install cmake gcc-c++ qt5-qtbase-devel qt5-qtnetwork-devel

# Clone repository
git clone https://github.com/YOUR_USERNAME/ai-sx-chatGPT-desktop.git
cd ai-sx-chatGPT-desktop

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run
./boh_chat_desktop
```

### Other Linux Distributions

#### Debian/Ubuntu
```bash
sudo apt install cmake g++ qtbase5-dev qtnetwork5-dev
```

#### Fedora
```bash
sudo dnf install cmake gcc-c++ qt5-qtbase-devel qt5-qtnetwork-devel
```

#### Arch Linux
```bash
sudo pacman -S cmake gcc qt5-base qt5-networkauth
```

## Configuration

### First-Time Setup

1. Launch the application
2. Go to **Settings** → **Preferences**
3. Enter your **OpenAI API Key** (get one at [platform.openai.com](https://platform.openai.com/api-keys))
4. Select your preferred model (default: gpt-4o-mini)
5. Click **OK**
6. Start chatting!

### API Key Storage

Your API key is stored securely in:
```
~/.config/ai-sx/ChatGPT Desktop.conf
```

This file is **never** committed to git and remains private on your system.

### Database Location

Chat history is stored in:
```
~/.local/share/ai-sx/chat_history.db
```

## Usage

### Creating Conversations
- Click **"+ New Chat"** to start a new conversation thread

### Managing Threads
- **Rename**: Double-click on thread name OR right-click → Rename
- **Delete**: Right-click on thread → Delete
- **Switch**: Click on any thread to load its conversation

### Sending Messages
- Type your message in the input box
- Press **Enter** to send (or click **Send ➤**)
- Use **Shift+Enter** for multi-line messages

### Viewing Timestamps
- Hover over any thread to see "Last updated" time

## Building RPM Package

```bash
# Install RPM build tools
sudo zypper install rpm-build rpmdevtools

# Build RPM
rpmbuild -ba chatgpt-desktop.spec
```

The RPM will be created in `~/rpmbuild/RPMS/x86_64/`

## Development

### Project Structure
```
ai-sx-chatGPT-desktop/
├── bohmainwindow.cpp/h/ui  # Main window
├── openai_client.cpp/h     # OpenAI API integration
├── chat_store.cpp/h        # SQLite database layer
├── settingsdialog.cpp/h/ui # Settings dialog
├── main.cpp                # Application entry point
├── resources/              # Icons and resources
├── CMakeLists.txt          # Build configuration
└── chatgpt-desktop.desktop # Linux desktop integration
```

## Troubleshooting

### Icon Not Showing
The icon may not display in some window managers (especially Wayland). To fix:

```bash
# Install desktop file
mkdir -p ~/.local/share/applications
cp chatgpt-desktop.desktop ~/.local/share/applications/
update-desktop-database ~/.local/share/applications/
```

### API Key Not Saved
Make sure the settings directory exists:
```bash
mkdir -p ~/.config/ai-sx
```

### Database Errors
Reset the database:
```bash
rm ~/.local/share/ai-sx/chat_history.db
# Restart the application
```

## License

[Specify your license here - e.g., MIT, GPL-3.0, etc.]

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Support

- **Issues**: [GitHub Issues](https://github.com/YOUR_USERNAME/ai-sx-chatGPT-desktop/issues)
- **OpenAI API**: [OpenAI Documentation](https://platform.openai.com/docs)

## Acknowledgments

- Built with Qt5 framework
- Powered by OpenAI API
- Designed for openSUSE and Linux desktop environments

---

**Note**: This application requires an OpenAI API key. API usage is billed by OpenAI according to their pricing. This is NOT affiliated with or endorsed by OpenAI.

