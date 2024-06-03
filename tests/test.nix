{ pkgs, ... }:
{
  name = "Test on Hyprland";

  nodes = {
    machine1 = { pkgs, ... }: {
      virtualisation.qemu.options = [
        "-nographic"
      ];
    };
    machine2 = { pkgs, ... }: {
      virtualisation.qemu.options = [
        # For a high level outline of available options, see:
        # https://wiki.gentoo.org/wiki/QEMU/Options

        # For a comprehensive list of video devices, see:
        # https://www.kraxel.org/blog/2019/09/display-devices-in-qemu/
        # https://www.kraxel.org/blog/2021/05/virtio-gpu-qemu-graphics-update/

        # Works
        "-device virtio-vga-gl,max_outputs=2"
        "-display gtk,gl=on,show-cursor=off"

        # Works but is slow
        # "-device virtio-vga"
        # "-display gtk,gl=off,show-cursor=off"

        # Incantations for creating a second monitor
        # https://github.com/qemu/qemu/blob/master/docs/multiseat.txt
        "-device pci-bridge,addr=12.0,chassis_nr=2,id=head.2"
        "-device secondary-vga,bus=head.2,addr=02.0,id=video.2"
        "-device nec-usb-xhci,bus=head.2,addr=0f.0,id=usb.2"
        "-device usb-kbd,bus=usb.2.0,port=1,display=video.2"
        "-device usb-tablet,bus=usb.2.0,port=2,display=video.2"
      ];
      virtualisation.writableStore = true;

      users.users.test = {
        isNormalUser = true;
        password = "test";
      };

      virtualisation = {
        memorySize = 8192;
        cores = 4;
      };

      services.displayManager = {
        sddm.enable = true;
        sddm.wayland.enable = true;
        autoLogin.enable = true;
        autoLogin.user = "test";
      };

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
      nix.settings.experimental-features = [ "nix-command" "flakes" ];

      systemd.services."hello" = {
        script = ''
          mkdir -p /home/test/.config/hypr/
          echo "exec = kitty" > /home/test/.config/hypr/hyprland.conf
          echo "exec = hyprctl monitors -j | jq -r '.[].name' | xargs -I{} hyprctl keyword monitor {},1920x1080@60,0x0,1" >> /home/test/.config/hypr/hyprland.conf
        '';
        wantedBy = [ "network-online.target" ];
      };

    };

  };

  interactive.nodes.machine1 = import ./debug-host-module.nix;

  testScript = /* python */ ''
    start_all()
    machine1.wait_for_unit("network-online.target")
    machine2.wait_for_unit("graphical.target")

    machine1.succeed("ping -c 1 machine2")
    machine2.succeed("ping -c 1 machine1")
  '';
}
