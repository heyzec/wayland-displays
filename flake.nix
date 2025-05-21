{
  description = "Yet another wayland displays configurator";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };
  outputs = inputs @ {flake-parts, ...}:
    flake-parts.lib.mkFlake {inherit inputs;} {
      systems = [
        "x86_64-linux"
      ];
      perSystem = {
        config,
        self',
        inputs',
        pkgs,
        lib,
        system,
        ...
      }: let
        wayland-displays = pkgs.callPackage ./package.nix {};
        wayland-displays-debug = wayland-displays.overrideAttrs {
          # See https://nixos.wiki/wiki/Debug_Symbols
          dontStrip = true;
          separateDebugInfo = true;
        };
      in {
        packages.default = wayland-displays;

        packages.debug = pkgs.writeShellApplication {
          name = "debug";
          text = ''
            trap "" SIGUSR1
            ${pkgs.gdb}/bin/gdb \
              --quiet \
              --ex "handle SIGUSR1 nostop pass" \
              --ex "run" \
              --args ${wayland-displays-debug}/bin/wayland-displays "$@"
          '';
        };

        devShells.default = pkgs.mkShell {
          packages = with pkgs;
            [
              # For clangd
              clang-tools_17
              # Create .ui files
              glade
              # Testing sockets
              socat
            ]
            ++ config.packages.default.nativeBuildInputs
            ++ config.packages.default.buildInputs;
        };
      };
    };
}
