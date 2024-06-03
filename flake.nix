{
  description = "wayland-displays";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";
  };

  outputs = inputs@{ self, flake-parts, ... }: flake-parts.lib.mkFlake { inherit inputs self; } {
    systems = [ "x86_64-linux" ];
    perSystem = { config, pkgs, ... }: {

      devShells.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          gtk3
          pkg-config
        ];
      };

      packages.cli = pkgs.stdenv.mkDerivation {
        name = "cli";
        src = self;
        buildInputs = with pkgs; [
          libgcc
        ];
        buildPhase = "g++ -o cli ./cli.cpp";
        installPhase = "mkdir -p $out/bin; install -t $out/bin cli";
      };

      packages.gui = pkgs.stdenv.mkDerivation {
        name = "gui";
        src = self;
        buildInputs = with pkgs; [
          libgcc
          gtk3
          pkg-config
        ];
        buildPhase = "g++ -o gui ./gui.cpp `pkg-config --cflags --libs gtk+-3.0`";
        installPhase = "mkdir -p $out/bin; install -t $out/bin gui";
      };
    };
  };
}
