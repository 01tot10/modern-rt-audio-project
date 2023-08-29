# Modern Real-Time Audio Project

This repository contains the project work for the course [Modern Real-Time Audio Programming](https://github.com/Neural-DSP/modern-rt-audio-course)

## Configuring and building

To configure the project, run:
```bash
cmake -Bbuild
```

To add `projects/` into the build, run:
```bash
cmake -Bbuild -DBUILD_PROJECTS=ON
```
To build the project, run:
```bash
cmake --build build
```
