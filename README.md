Custom module for waybar to show the cpu usage as a histogram.

Based on the [cpu
module](https://github.com/Alexays/Waybar/blob/master/src/modules/cpu.cpp)
from [waybar](https://github.com/Alexays/Waybar).

# Usage
## waybar
In the `config` file, use:

```json
    "custom/cpu-hist": {
        "format": "{}",
        "return-type": "json",
        "exec": "/path/to/cpu-hist 2> /dev/null",
        "interval": 10
    },
```

# Building from source

## dependencies

Install these with your package manager
  - [fmt](https://fmt.dev/latest/index.html)
  - [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

## build
To build and install the binary to `$HOME/bin`.
```json
meson --buildtype=release --prefix=$HOME --unity=on build
ninja -C build
ninja -C build install
```

If you want to install it somewhere else change `--prefix`. And if you
want to package it up before installing use `DESTDIR=pkg ninja -C build
install`. Where `DESTDIR` is relative to the `build` directory.
