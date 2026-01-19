if [ -d "build" ]; then
    rm -rf build
fi
cmake -D CMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_STANDARD=17 -S . -B build/ 

cmake --build build 