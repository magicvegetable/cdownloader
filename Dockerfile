FROM fedora:latest

WORKDIR /app

RUN dnf install -y meson pkgconf-pkg-config

RUN dnf install -y gettext desktop-file-utils appstream

RUN dnf install -y glib2 glib2-devel

RUN dnf install -y gtk4 gtk4-devel libadwaita libadwaita-devel

RUN dnf install -y dbus dbus-x11

# RUN dnf install -y xorg-x11-xauth xhost

RUN dnf install -y gcc

RUN dnf clean all

COPY . /app

RUN mkdir build

RUN meson setup build

RUN meson compile -C build

RUN meson install -C build

CMD ["cdownloader"]

