### Prerequisites

- Raylib

### Clone the repo

```
git clone https://github.com/nearlynithin/procedural_grass.git
```

### Build

```
cd src/
gcc main.c -o game  -lraylib -lm -lpthread -ldl -lrt -lX11
./game

```

### File Structure

```
.
├── assets
│   ├── blade.obj
│   ├── blade.png
│   ├── and some other stuff
├── README.md
├── shaders
│   ├── glsl100
│   │   ├── lighting.fs
│   │   ├── lighting.vs
│   │   ├── these two are all we need
│   └── vignette.fs //this too
└── src
    ├── entities
    ├── helpers
    │   ├── screen.h
    │   └── setShaders.h
    ├── include
    │   └── rlights.h
    ├── main.c //main file
    └── world
        ├── grass.h
        ├── ground.h
        └── skybox.h
```
