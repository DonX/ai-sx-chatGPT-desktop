Name:           ai-sx-chatgpt-desktop
Version:        0.1.0
Release:        1%{?dist}
Summary:        Native Linux desktop client for ChatGPT using the OpenAI API

License:        MIT
URL:            https://github.com/DonX/ai-sx-chatGPT-desktop
Source0:        %{name}-%{version}.tar.gz

%if 0%{?suse_version}
BuildRequires:  update-desktop-files
%endif

BuildRequires:  cmake >= 3.16
BuildRequires:  gcc-c++
BuildRequires:  qt5-qtbase-devel
BuildRequires:  qt5-qtnetwork-devel
BuildRequires:  qt5-qtsql-devel
BuildRequires:  desktop-file-utils

Requires:       qt5-qtbase
Requires:       qt5-qtnetwork
Requires:       qt5-qtsql
Requires:       qt5-qtsql-sqlite

%description
ai-SX ChatGPT Desktop is a native Linux desktop application that provides
a modern user interface for interacting with the official OpenAI ChatGPT API.

Features include:
- Full conversation context (persistent chat memory)
- Multi-thread conversation management
- Support for multiple OpenAI models
- SQLite-based local storage
- Secure per-user API key management
- Modern Qt5 interface

%prep
%setup -q

%build
mkdir -p build
cd build
%cmake ..
%make_build

%install
cd build

# Binary
install -D -m 0755 boh_chat_desktop \
  %{buildroot}%{_bindir}/chatgpt-desktop

# Desktop entry
install -D -m 0644 ../chatgpt-desktop.desktop \
  %{buildroot}%{_datadir}/applications/chatgpt-desktop.desktop

# Icon (v0.1: single size)
install -D -m 0644 ../resources/icons/boh-chat.png \
  %{buildroot}%{_datadir}/icons/hicolor/128x128/apps/chatgpt-desktop.png

desktop-file-validate \
  %{buildroot}%{_datadir}/applications/chatgpt-desktop.desktop

%files
%license LICENSE
%doc README.md
%{_bindir}/chatgpt-desktop
%{_datadir}/applications/chatgpt-desktop.desktop
%{_datadir}/icons/hicolor/128x128/apps/chatgpt-desktop.png

%post
/usr/bin/update-desktop-database &> /dev/null || :
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &> /dev/null || :

%postun
/usr/bin/update-desktop-database &> /dev/null || :
/usr/bin/gtk-update-icon-cache %{_datadir}/icons/hicolor &> /dev/null || :

%changelog
* Sat Feb 01 2025 Don Clermont <1355350+DonX@users.noreply.github.com> - 0.1.0-1
- Initial early release (v0.1 / v1-internal)


