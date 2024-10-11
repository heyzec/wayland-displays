import json

# https://nixos.org/manual/nixos/stable/#ssec-machine-objects
def main(subtest, machine):
    with subtest("ensure hyprland starts"):
        machine.wait_until_succeeds("ps aux | grep Hyprland")
        machine.sleep(1)
        machine.succeed("ps aux | grep Hyprland")
        machine.screenshot("before")
        machine.wait_for_text("alice@machine", 1)
        machine.screenshot("started")

    # We create two methods to run commands in the VM with wayland variables
    # 1. Sending keys to a tmux session in a kitty teminmal within the hyprland session
    # 2. Using a temporary file to store the environment variables and sourcing it before running the command

    def send_to_term(cmd):
        machine.succeed(f"""sudo -u alice tmux send-keys '{cmd}' Enter""")

    def get_env(vars):
        ENV_FILE = "/tmp/.env"
        send_to_term(f"touch {ENV_FILE}")
        for var in vars:
            send_to_term(f'echo "export {var}=${var}" >> {ENV_FILE}')

        def succeed_with_env(cmd):
            return machine.succeed(f"(source {ENV_FILE} && {cmd})")

        send_to_term("touch /tmp/done")
        machine.wait_for_file("/tmp/done")
        return succeed_with_env

    succeed_with_env = get_env([
        "HYPRLAND_INSTANCE_SIGNATURE",
        "XDG_RUNTIME_DIR",
    ])

    with subtest("ORDER"):
        succeed_with_env("hyprctl output create headless test")  # Create a HEADLESS-X output
        machine.sleep(2)

        send_to_term("XDG_CONFIG_HOME=/etc wayland-displays --server & >/dev/null 2>&1")
        machine.sleep(2)
        send_to_term("XDG_CONFIG_HOME=/etc wayland-displays --gui &")
        machine.sleep(2)

        monitors = json.loads(succeed_with_env("hyprctl monitors -j"))
        monitor_virtual = next((monitor for monitor in monitors if 'Virtual' in monitor['name']), None)
        assert monitor_virtual is not None
        assert (monitor_virtual['x'], monitor_virtual['y']) == (0, 0), f"Monitor not at the right position\n{monitors}"

    # send_to_term("grim /tmp/grim.png")
    # machine.wait_for_file("/tmp/grim.png")
    # machine.copy_from_vm("/tmp/grim.png")
    machine.screenshot("break")

