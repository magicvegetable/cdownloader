FROM alpine:3.19.1

WORKDIR /app

RUN apk add meson pkgconf

RUN apk add gettext desktop-file-utils appstream

RUN apk add glib glib-dev

RUN apk add gtk4.0 gtk4.0-dev libadwaita libadwaita-dev

RUN apk add dbus dbus-x11

RUN apk add build-base

RUN apk add yt-dlp

RUN apk add adwaita-icon-theme font-cantarell

COPY . /app

RUN mkdir build

RUN meson setup build

RUN meson compile -C build

RUN meson install -C build

RUN rm -rf /app

CMD ["cdownloader"]
