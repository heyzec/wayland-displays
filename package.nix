{
  lib,
  pkgs,
}:
pkgs.stdenv.mkDerivation (finalAttrs: {
  name = "wayland-displays";

  # Filtered list of source files
  src = lib.sourceByRegex ./. [
    "meson.build"
    "^src.*"
    "^lib.*"
    "^protocols.*"
    "^resources.*"
    "^tests/unit.*"
  ];

  nativeBuildInputs = with pkgs; [
    meson
    ninja
    pkg-config
    wayland-scanner
  ];

  buildInputs = with pkgs; [
    gtk3
    libepoxy
    wayland
    spdlog
    yaml-cpp
  ];
})
