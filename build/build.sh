ls | grep -v toy | grep -v "build.sh" | xargs rm -rf
cmake ..
make
