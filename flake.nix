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
      perSystem = { config, self', inputs', pkgs, lib, system, ... }: {

        devShells.default = pkgs.mkShell {
          packages = with pkgs; [
            # Create .ui files
            glade

            # Experiment with existing tools
            way-displays
            wdisplays
            nwg-displays
            kanshi
          ] ++ config.packages.default.nativeBuildInputs;
        };

        packages.default = pkgs.stdenv.mkDerivation {
          name = "wayland-displays";

          # Filtered list of source files
          src = lib.sourceByRegex ./. [
            "Makefile"
            "CMakeLists.txt"
            "^src.*"
            "^cmake.*"
            "^protocols.*"
            "^resources.*"
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
          ];

          # Needed at run time
          buildInputs = [ ];

          doCheck = true;
        };
      };
    };
}
