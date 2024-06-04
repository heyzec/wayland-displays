{
  description = "wayland-displays";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs, ... }@inputs: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        (runCommand "cDependencies" {} ''
          mkdir -p $out/include

          cp -r ${gtk3.dev}/include/gtk-3.0/* $out/include
          cp -r ${glib.dev}/include/glib-2.0/* $out/include
        '')
        way-displays
        wdisplays
        nwg-displays
      ];
      shellHook = ''
        NIX_CFLAGS_COMPILE="$(pkg-config --cflags gtk4) $NIX_CFLAGS_COMPILE"
      '';
    };

    packages.${system} = {
      default = pkgs.stdenv.mkDerivation {
        name = "app";
        src = self;
        buildInputs = with pkgs; [
          libgcc
          gtk3
          pkg-config
        ];
        buildPhase = "g++ -o app src/main.cpp `pkg-config --cflags --libs gtk+-3.0`";
        installPhase = "mkdir -p $out/bin; install -t $out/bin app";
      };

      # nix -L build .#test
      test = pkgs.testers.runNixOSTest ./tests/test.nix;
      # checks.${system} = config.packages;
    };

    nixosConfigurations = {
      # The configuration for build-vm (home manager as a module)
      "nixie-vm" = inputs.nixpkgs.lib.nixosSystem {
        modules = [
          ({ nixpkgs.hostPlatform = system; })
          ({ programs.hyprland.enable = true; })
        ];
      };
    };
  };
}
