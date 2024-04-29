FROM registry.gitlab.gnome.org/gnome/gnome-control-center/fedora/40:2024-04-16.0-40

WORKDIR /app

RUN dnf install -y yt-dlp

RUN dnf clean all

COPY . /app

RUN mkdir build

RUN meson setup build

RUN meson compile -C build

RUN meson install -C build

CMD ["cdownloader"]

