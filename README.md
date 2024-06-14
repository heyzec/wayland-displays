# wayland-displays

## Impetus
While Wayland offers several tooling solutions for managing display outputs, it often seems that each solution lacks certain features found in others.

wayland-displays aims to address this gap by integrating the best aspects of existing tools. The focus is on:
- Intuitive user experience similar to that of built-in settings on both Windows and MacOS.
- User-friendly GUI for easy configuration.
- Feature-complete CLI tool capable of adjusting any setting.
- Ensuring compatibility across various window managers through comprehensive VM-based testing.

## Comparison

|Project|Support|GUI|Persistence|Profiles|
|---|---|---|---|---|
|[way-displays](https://github.com/alex-courtis/way-displays)|wayland (wl-roots)|❌ [Issue](https://github.com/alex-courtis/way-displays/issues/55)|✅ [yaml](https://github.com/alex-courtis/way-displays/wiki/Configuration)|[Issue](https://github.com/alex-courtis/way-displays/issues/83)|
|[wdisplays](https://github.com/artizirk/wdisplays)|wayland|✅|❌|❌|
|[nwg-displays](https://github.com/nwg-piotr/nwg-displays)|sway, Hyprland|✅|✅|❌|
|[wlay](https://github.com/atx/wlay)|Wayland|✅|✅|❌|
|[kanshi](https://sr.ht/~emersion/kanshi/)|wayland|❌|✅ [custom](https://sr.ht/~emersion/kanshi/#configuration-file)|✅|
|[autorandr](https://github.com/phillipberndt/autorandr)|X11|❌|✅ [ini](https://sr.ht/~emersion/kanshi/)|✅|


### way-displays
[alex-courtis/way-displays](https://github.com/alex-courtis/way-displays) is a CLI-based tool that is the wayland successor to [alex-courtis/xlayoutdisplay](https://github.com/alex-courtis/xlayoutdisplay), which is inspired by emersion/kanshi.
- It works as a single binary client-server model, where the server is a daemon that responds to configuration changes, while the client allows for high flexibility of configuration.
- Unlike the rest of the tools here, it does not directly allow configuration of display positions. Instead, the suser sets `ARRANGE`, `ALIGN`, `ORDER` and `SCALE` parameters, and the tool does the necessary computations. This allows it to make sensible changes when displays are added and removed.


### wdisplays
[artizirk/wdisplays](https://github.com/artizirk/wdisplays) is a fork of the unmaintained [MichaelAquilina/wdisplays](https://github.com/MichaelAquilina/wdisplays), a GTK3 GUI application with code borrowed from emersion/kanshi.
- Uses the `wlr-output-management-unstable-v1 protocol`


### nwg-displays
[nwg-piotr/nwg-displays](https://github.com/nwg-piotr/nwg-displays) is a GUI tool inspired by [artizirk/wdisplays](https://github.com/artizirk/wdisplays) and [atx/wlay](https://github.com/atx/wlay).
- Unlike the rest of the tools here, it only supports sway and hyprland. The tool takes configurations the user and converts it into compositor-specific config files meant for the two compositors to source, hence providing a form of persitence indirectly.



### wlay
[atx/wlay](https://github.com/atx/wlay) is a simple GUI app that allows configuration of the most basic of settings.
- It can generate sway config, kanshi config or wlr-randr script.
- I'd recommend using wdisplays or nwg-displays, unless kanshi support is required


### kanshi
[emersion/kanshi](https://sr.ht/~emersion/kanshi/) is a CLI tool that has inspired many other tools listed here.
- Uses the `wlr-output-management protocol`


### autorandr
[phillipberndt/autorandr](https://github.com/phillipberndt/autorandr) is a Python rewrite of [wertarbyte/autorandr](https://github.com/wertarbyte/autorandr) meant for X11. Under the hood, they both use xrandr. Even though these tools have existed for many years, they support the feature of profiles. It can still be informative to see how displays were managed in the X11 days.

## Build instructions

We use cmake to configure the project.

**Dependencies**:
- GTK3
- Cairo
- Wayland
- `wayland-scanner` (generates headers from wayland xml spec files)

```sh
make
```

**For Nix users**

Just `nix run` will suffice!

