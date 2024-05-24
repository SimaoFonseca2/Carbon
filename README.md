# Carbon (C++ Compiler)

Carbon is a programming language made as a learning project. I named it carbon since under pressure it makes diamonds but it can also make rubble if you don't know what you're doing. 💥

Hopefully the language can evolve enough for the compiler to be self-hosted but i'm fine with just getting a learning experience out of it. 

## Building

Requires `nasm` and `ld` on a Linux operating system.

```bash
git clone https://github.com/SimaoFonseca2/Carbon
cd Carbon
mkdir build
cmake -S . -B build
cmake --build build
```

Executable will be `carbon` in the `build/` directory.
