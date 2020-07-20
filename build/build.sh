ls | grep -v toy | grep -v "yue.mp4" | grep -v "build.sh" | xargs rm -rf
cmake ..
make
