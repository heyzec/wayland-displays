{
  name = "My tests haha";

  nodes = {
    machine1 = { pkgs, ... }: {
      programs.hyprland.enable = false;
    };
    machine2 = { pkgs, ... }: { };
  };

  interactive.nodes.machine1 = import ./debug-host-module.nix;

  testScript = ''
    machine1.wait_for_unit("network-online.target")
    machine2.wait_for_unit("network-online.target")

    machine1.succeed("ping -c 1 machine2")
    machine2.succeed("ping -c 1 machine1")
  '';
}
