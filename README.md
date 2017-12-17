# bbb.zmq.sub

[zeromq](http://zeromq.org) subscriber for [_Cycling'74 Max_](https://cycling74.com/products/max).

## Notice

if you want to develop this object, you had better see [bbb.max.dev](2bbb/bbb.max.dev).

this repository is subdivided from bbb.max.dev.

and after clone/download bbb.max.dev, you see this **For developer** on this README.

## Object Argument

### **bbb.zmq.sub** [ `host_str` [ `format_char...` ] ]

- `host_str` : destination

- - default: `""` (doesn’t connect to subscriber)

- `format_char...` : see _format_ on Messages

- - default: `[t]`

**ex.**

- **bbb.zmq.sub**
- **bbb.zmq.sub** ``tcp://localhost:9999``
- **bbb.zmq.sub** ``ipc:///tmp/foo [c]``

## Messages

### **connect** `host_str`

connect to publisher is binded to `host_str`.

### **disconnect**

disconnect from publisher.

### **format** `format_char…`

- `c` : char (int8_t)
- `C` : unsigned char (uint8_t)
- `s` : short (int16_t)
- `S` : unsigned short (uint16_t)
- `i` : int (int32_t)
- `I` : unsigned int (uint32_t)
- `l` : long (int64_t)
- `L` : unsigned long (uint64_t)
- `f` : float
- `d` : double
- `t` : null terminated string
- `_` : discard 1byte
- `[X]` : array of type `X`
  - `X` allows complex type doesn't include array.
    - ex. `cc[ccif]` is valid

**ex.**

- format `iCCC`

- - int32_t, uint8_t, uint8_t, uint8_t

- format `ff[iit]`

- - float, float, int32_t, int32_t, string, int32_t, int32_t, string, …, int32_t, int32_t, string

## TODO

- [ ] implement option about converting belong Little Endian ↔ Big Endian (if needed)


- [ ] out connection status from right outlet
- [ ] make enable to choice bind or connect

## Licenses

MIT

### Licenses of dependencies

- [2bbb/bbb.max.dev](https://github.com/2bbb/bbb.max.dev)
  - MIT
- [Cycling74/max-sdk](https://github.com/Cycling74/max-sdk)
  - MIT
- [grrrwaaa/maxcpp](https://github.com/grrrwaaa/maxcpp)
  - MIT
- [zeromq/libzmq](https://github.com/zeromq/libzmq)
  - MPL-2.0
- [zeromq/cppzmq](https://github.com/zeromq/cppzmq)
  - MIT

# For developer

## Dependencies

- [2bbb/bbb.max.dev](https://github.com/2bbb/bbb.max.dev)
  - [Cycling74/max-sdk](https://github.com/Cycling74/max-sdk)
    - `v7.3.3` tag
  - [2bbb/maxcpp](https://github.com/grrrwaaa/maxcpp) (fork of [grrrwaaa/maxcpp](https://github.com/grrrwaaa/maxcpp))
    - `bbb.max.dev` branch
- [zeromq/libzmq](https://github.com/zeromq/libzmq) (recommend to use [**homebrew**](https://brew.sh/))
- [zeromq/cppzmq](https://github.com/zeromq/cppzmq)

## Dev env.

- Touchbar MacBook Pro (15-inch, 2016)
- macOS Sierra v10.12.6
- Xcode v9.1 (9B55)
- Max v7.3.4 (Oct)

### Setup dev env.

```bash
brew install zeromq
```

## Author

* ISHII 2bit
* i[at]2bit.jp

## At last

If you get happy with using this addon, and you're rich, please donation for support continuous development.

Bitcoin: `17AbtW73aydfYH3epP8T3UDmmDCcXSGcaf`

