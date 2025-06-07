# boulderdash_cpp 
A simple C++ implementation of Boulderdash/Emerald Mine style games.

## Include to Your Project: CMake

### FetchContent
```shell
include(FetchContent)
# ...
FetchContent_Declare(boulderdash
    GIT_REPOSITORY https://github.com/tuero/boulderdash_cpp.git
    GIT_TAG master
)

# make available
FetchContent_MakeAvailable(boulderdash)
link_libraries(boulderdash)
```

### Git Submodules
```shell
# assumes project is cloned into external/boulderdash_cpp
add_subdirectory(external/boulderdash_cpp)
link_libraries(boulderdash)
```

## Installing Python Bindings
```shell
git clone https://github.com/tuero/boulderdash_cpp.git
pip install ./boulderdash_cpp
```

If you get a `GLIBCXX_3.4.XX not found` error at runtime, 
then you most likely have an older `libstdc++` in your virtual environment `lib/` 
which is taking presidence over your system version.
Either correct your `$PATH`, or update your virtual environment `libstdc++`.

For example, if using anaconda
```shell
conda install conda-forge::libstdcxx-ng
```

## Level Format
Levels are expected to be formatted as `|` delimited strings, where the first 2 entries are the rows/columns of the level,
the third entry is the number of gems requires to open the exit,
then the following `rows * cols` entries are the element ID (see `HiddenCellType` in `definitions.h`).

## Notice
The image tile assets under `/tiles/` are taken from [Rocks'n'Diamonds](https://www.artsoft.org/). 
A copy of the license for those materials can be found alongside the assets.
