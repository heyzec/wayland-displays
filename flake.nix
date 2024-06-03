{
  description = "wayland-displays";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
    flake-parts.inputs.nixpkgs-lib.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, ... }@inputs: let
    system = "x86_64-linux";      # system arch
    pkgs = nixpkgs.legacyPackages.${system};
  in {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          gtk3
          pkg-config
        ];
      };

      packages.${system} = {


      cli = pkgs.stdenv.mkDerivation {
        name = "cli";
        src = self;
        buildInputs = with pkgs; [
          libgcc
        ];
        buildPhase = "g++ -o cli ./cli.cpp";
        installPhase = "mkdir -p $out/bin; install -t $out/bin cli";
      };

      gui = pkgs.stdenv.mkDerivation {
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
