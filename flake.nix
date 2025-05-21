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
      }: {
        packages.default = pkgs.callPackage ./package.nix {};

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
            ]
            ++ config.packages.default.nativeBuildInputs;
        };
      };
    };
}
