{
  description = "Yet another wayland displays configurator";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = inputs@{ flake-parts, ... }:
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
      ];
      perSystem = { config, self', inputs', pkgs, lib, system, ... }:
        let
          wayland-displays = pkgs.stdenv.mkDerivation
            {
              name = "wayland-displays";

              # Call Makefile targets instead of CMake
              dontUseCmakeConfigure = true;

              # Filtered list of source files
              src = lib.sourceByRegex ./. [
                "Makefile"
                "CMakeLists.txt"
                "^src.*"
                "^lib.*"
                "^cmake.*"
                "^protocols.*"
                "^resources.*"
                "^tests.*"
              ];

              # Needed at compile time
              nativeBuildInputs = with pkgs; [
                # C++ Compiler is already part of stdenv
                cmake
                pkg-config
                gtk3
                cairo
                wayland
                wayland-scanner
                nlohmann_json
                yaml-cpp
                catch2_3
              ];

              buildInputs = [ ];

              makeFlags = [
                "PREFIX=${placeholder "out"}"
              ];

              doCheck = true;
            };

          wayland-displays-debug = (config.packages.default.overrideAttrs {
            # See https://nixos.wiki/wiki/Debug_Symbols
            dontStrip = true;
            separateDebugInfo = true;
          });
        in
        {
          devShells.default = pkgs.mkShell {
            packages = with pkgs;
              [
                # For clangd
                clang-tools_17

                # Create .ui files
                glade

                # Testing sockets
                socat

                # Experiment with existing tools
                way-displays
                wdisplays
                nwg-displays
                kanshi
              ] ++ config.packages.default.nativeBuildInputs;
          };

          packages.default = wayland-displays;
          apps.debug =
            let
              debug = pkgs.writeShellApplication {
                name = "debug";
                text = ''
                  # readelf --debug-dump=line  ${config.packages.default.out}/bin/wayland-displays
                  ${pkgs.gdb}/bin/gdb --args ${wayland-displays-debug}/bin/wayland-displays "$@"
                '';
              };
            in
            {
              type = "app";
              # Using a derivation in here gets replaced
              # with the path to the built output
              program = "${debug}/bin/debug";
            };
        };
    };
}
