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
          ] ++ config.packages.default.nativeBuildInputs ++ [
            # Hack for LSPs to find headers
            # Find using nix-locate <missing-header> | grep -v ^\(
            (runCommand "cDependencies" { src = ./.; } ''
              mkdir -p $out/include
              cp -r ${gtk3.dev}/include/gtk-3.0/* $out/include
              cp -r ${cairo.dev}/include/cairo/* $out/include

              cp -r ${glib.dev}/include/glib-2.0/* $out/include
              cp -r ${glib.out}/lib/glib-2.0/include/* $out/include

              cp -r ${pango.dev}/include/pango-1.0/* $out/include
              cp -r ${harfbuzz.dev}/include/harfbuzz/* $out/include
              cp -r ${gdk-pixbuf.dev}/include/gdk-pixbuf-2.0/* $out/include
              cp -r ${at-spi2-atk.dev}/include/atk-1.0/* $out/include

              # Hackier hack: Include generated headers from build process
              cp ${config.packages.default.out}/include/* $out/include
            '')
          ];
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
