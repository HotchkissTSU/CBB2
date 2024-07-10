g++ -c src/main.cpp -o obj/main.o -I"include" -L"static"

g++ obj/*.o -o bin/cbb2.exe -I"include" -L"static" -ljsoncpp