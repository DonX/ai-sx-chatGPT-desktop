# ai-SX ChatGPT Desktop

**ai-SX ChatGPT Desktop** is a native Linux desktop application built with **Qt5** that provides a clean, persistent interface for interacting with the official **OpenAI ChatGPT API**.

The application installs and behaves like a proper Linux desktop program: it integrates with the application menu, uses system icons, stores data locally, and updates through the system package manager.

---

## Screenshot

![ai-SX ChatGPT Desktop running on openSUSE](docs/screenshot.png)

---

![ChatGPT Desktop](resources/icons/boh-chat.png)

---
## Project Status

⚠️ **Early Release (v0.1.1 / internal)**

This project is under active development.  
Core functionality, desktop integration, and RPM packaging are now working and verified. Interfaces and workflows may still evolve.

---

## Features

- Native Qt5 desktop application (no browser dependency)
- Persistent conversation history
- Multiple chat threads
- SQLite-based local storage
- Secure per-user API key storage
- Dark, distraction-free interface
- Proper desktop integration (menu entry, icon, Wayland support)

---

## Supported Platform

Currently **tested and packaged** for:

- **openSUSE Leap 16.0 (x86_64)**

Distribution is handled via **Open Build Service (OBS)** and installs using `zypper`.

No claims are made at this stage about support for other distributions.

---

## Installation (openSUSE Leap 16.0)

Add the repository:

```bash
sudo zypper ar -f https://download.opensuse.org/repositories/home:/L_Don_X/16.0/ ai-sx

Refresh repositories and accept the signing key:

sudo zypper refresh


Install the application:

sudo zypper install ai-sx-chatgpt-desktop

Updating

Updates are handled normally through the package manager:

sudo zypper refresh
sudo zypper update ai-sx-chatgpt-desktop

Launching the Application

Launch from the desktop application menu
or

Run from a terminal:

chatgpt-desktop

Configuration
API Key Setup

Launch the application

Open Settings

Enter your OpenAI API key
(create one at https://platform.openai.com/api-keys
)

Select your preferred model

Save and start chatting

Local Storage

Configuration:

~/.config/ai-sx/ChatGPT Desktop.conf


Chat history database:

~/.local/share/ai-sx/chat_history.db


All data remains local to the user’s machine.

Relationship to AXEM-SX and Golda.Global

ai-SX ChatGPT Desktop is a user-facing application developed within the broader AXEM-SX technology chair.

AXEM-SX operates under Golda.Global, alongside related initiatives such as GoudDi.
This application represents a concrete desktop tool within that ecosystem, without attempting to replace or redefine the underlying operating system.

License

MIT License
See the LICENSE file for details.

Support

Issues: https://github.com/DonX/ai-sx-chatGPT-desktop/issues

OpenAI API: https://platform.openai.com/docs

Note: This application requires an OpenAI API key. API usage is billed by OpenAI according to their pricing. This project is not affiliated with or endorsed by OpenAI.
