# 3D Graphics Engine
This is being made with the educational intent of learning about some of
the basic principles of 3D graphics computing. The beauty of discovering the maths and
equations that are behind every big and small 3D visualization software is incredible.
The idea is to build this engine with the minimum number of external dependencies and
still maintain good efficiency and performance.

---
## How its made
**Built with:**
* SDL2  — the only external library used in this project.

---
## Optimizations
The code is optimized to some extent so that performance can be maintained even with
bigger and chunkier models to render. 

**Optimization #1:**
Before completing the projection and scaling calculation being made, the face is
checked to see whether the camera can see it. This helps so that computational
power is not lost on calculating faces that we will not see.

---
## What I Learned
Through this project, I learned how to make and use macros in C to make
the code cleaner and easier to understand. I also built knowledge around
the SDL2 library, as well as 3D graphics computing.

---
## Getting started
### Prerequisites
Having WSL installed (I used ubuntu). If you do not have WSL, you can install
it through the Microsoft app store.

To install Ubuntu, run:
`wsl --install -d Ubuntu` in the WSL command line.

You will need SDL2 installed in WSL also. You can use:
`sudo apt install libsdl2-dev` to install it.


---
### Installation
**How to build**
```
cmake -B build -DCMAKE_C_FLAGS="-fsanitize=address -g -lm"
cmake --build build
```

**How to run**
```
./build/main
```

---
## Contacts
Francisco Faria - francisco.f.10015@gmail.com 

Project Link - https://github.com/Fariaa09/3DGraphicsEngineRenderer