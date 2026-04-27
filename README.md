# Pong

A small Pong implementation written in C with SDL2.

## Requirements

Linux with:

- C compiler (`gcc` or `clang`)
- `make`
- SDL2 development files

On Arch Linux:

```sh
sudo pacman -S base-devel sdl2
```

On Debian or Ubuntu:

```sh
sudo apt install build-essential libsdl2-dev
```

## Build

```sh
make
```

## Run

```sh
./pong
```

## Controls

- Left paddle: `W` / `S`
- Right paddle: `Up` / `Down`
- Restart after a win: `Space`
- Quit: `Esc` or close the window

## Clean

```sh
make clean
```
