{ lib, pkgs, ... }:
{
  name = "Test on Hyprland";

  # Refer to:
  # https://nixos.org/manual/nixos/stable/#sec-writing-nixos-tests
  # https://github.com/NixOS/nixpkgs/blob/master/nixos/tests/sway.nix

  nodes = {
    machine1 = { pkgs, ... }: {
      virtualisation.qemu.options = [
        "-nographic"
      ];
    };
    machine2 = { pkgs, ... }: {
      users.users.alice = {
        isNormalUser = true;
        password = "alice";
        extraGroups = [
          "wheel"
        ];
      };
      services.getty.autologinUser = "alice";

      # Need to switch to a different GPU driver than the default one (-vga std) so that Sway can launch:
      virtualisation.qemu.options = [ "-vga none -device virtio-gpu-pci" ];

      # Automatically configure and start Sway when logging in on tty1:
      programs.bash.loginShellInit = ''
        if [ "$(tty)" = "/dev/tty1" ]; then
          set -e

          mkdir -p /home/alice/.config/hypr/
          echo "exec = kitty" > /home/alice/.config/hypr/hyprland.conf
          # echo "exec = wdisplays > /home/alice/logs" > /home/alice/.config/hypr/hyprland.conf
          # echo "exec = hyprctl monitors -j | jq -r '.[].name' | xargs -I{} hyprctl keyword monitor {},1920x1080@60,0x0,1" >> /home/alice/.config/hypr/hyprland.conf

          export XDG_RUNTIME_DIR=/run/user/1000/
          Hyprland
        fi
      '';

      programs.hyprland = {
        enable = true;
      };

      environment.systemPackages = with pkgs; [
        kitty  # For bare hyprland to be usable
        way-displays
        wdisplays
        jq
        neovim
        grim
        remmina
        wayvnc
      ];
    };

  };

  enableOCR = true;

  interactive.nodes.machine1 = import ./debug-host-module.nix;

  testScript = /* python */ ''
    machine = machine2
    with subtest("ensure hyprland starts"):
        # machine.wait_until_succeeds("ls /tmp/hypr/*")
        machine.wait_until_succeeds("ps aux | grep Hyprland")
        machine.sleep(1)
        machine.succeed("ps aux | grep Hyprland")
        machine.wait_for_text("alice@machine", 1)
        # machine.sleep(3)
        machine.screenshot("started")


    with subtest("wdisplays"):
        machine.execute("sudo -u alice sh -c 'kitty --detach -- wdisplays'")
        # machine.wait_for_file("/home/alice/logs")
        # machine.copy_from_vm("/home/alice/logs", "")
        machine.sleep(1)
        machine.screenshot("wdisplays")
  '';
}
