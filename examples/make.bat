if exist build
  rd /s /q build

cmake -S . -B build
cmake --build build
