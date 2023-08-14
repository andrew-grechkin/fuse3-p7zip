# Contributing

## Introduction

Build system is based on CMake scripts, make and gcc/clang compilers.

All source code files are formatted with clang-format based on .clang-format configuration in the project

Include files of p7zip library are provided by git submodule `3rdparty/p7zip`

## Setup

* Install modern c++ compiler with -std=c++20 support
* Install cmake
* Install fuse3 (devel package)
* Install p7zip (devel package)

## Build

### Init 3rdparty submodule

```
$ make init
```

### Debug version

```
$ make build
```

## Tidy

```
$ make tidy
```
