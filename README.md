# tlogger

## How to build

Requirements:
- [conan](https://conan.io/) version 2.0 or higher

To build the project run the following commands:
```sh
conan install . --output-folder=build --build=missing
cd build
cmake --build .
```

## How to run

```sh
.\tlogger BTC-USD ETH-USD --output-file ticker.csv
```

See `.\tglogger --help` for details.
