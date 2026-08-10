{pkgs, ...}: {
  users.users.alice = {
    isNormalUser = true;
    password = "alice";
    extraGroups = [
      "wheel"
    ];
  };
  services.getty.autologinUser = "alice";

  # # Need to switch to a different GPU driver than the default one (-vga std) so that Sway can launch:
  # virtualisation.qemu.options = ["-vga none -device virtio-gpu-pci"];

  # Default of 1GB not enough
  # virtualisation.memorySize = 4096;
  virtualisation.diskSize = 2048;

  # Automatically configure and start Sway when logging in on tty1:
  programs.bash.loginShellInit = ''
    if [ "$(tty)" = "/dev/tty1" ]; then
      set -e

      mkdir -p /home/alice/.config/hypr/
      echo "exec = kitty -- tmux" > /home/alice/.config/hypr/hyprland.conf

      export XDG_RUNTIME_DIR=/run/user/1000/
      Hyprland
    fi
  '';

  programs.hyprland = {
    enable = true;
  };

  environment.systemPackages = with pkgs; [
    kitty # For bare hyprland to be usable
    way-displays
    wdisplays
    jq
    neovim
    grim
    remmina
    wayvnc
    # wayland-displays
    grim
    tmux
    socat
  ];
  environment.etc."wayland-displays.yml".text = ''
    PROFILES:
      vm:
        DISPLAYS:
          - '!^Virtual-1$'
          - '!^HEADLESS-\d$'

        ORDER:
          - '!^Virtual-1$'
          - '!^HEADLESS-\d$'

        ARRANGE: ROW
        ALIGN: BOTTOM
  '';

  services.qemuGuest.enable = true;
}
