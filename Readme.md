# Magma demo application

## Quick start

### Using the default FetchContent approach

Magma will be automatically downloaded and installed in the demo app build tree.
```bash
cmake -S . -B build
cmake --build build
```

### Using the find_package(Magma) approach

Considering Magma is already installed in */opt/Magma/*.
```bash
cmake -S . -B build -MAGMADEMO_FIND_PACKAGE_MAGMA=ON -DMagma_DIR=/opt/Magma/lib/cmake/Magma
cmake --build build
```
