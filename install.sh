rm -rf ./build ./src/dicts/ ./lib
mkdir build lib
cd include

rootcling -f ../lib/track_info.cc -c track_info.h+ Linkdef.h

cd ..

cp -R ./lib/ ./src/dicts/
rm -rf ./lib/*.cc
rm -rf ./src/dicts/*.pcm

cd build
cmake ../
make
cp libExtractModule.so ../lib/
cd ../