@echo off

gcc -std=c11 -Wall -Wextra -Wpedantic -c vector.c -o vector.o || exit /b 1
gcc -std=c11 -Wall -Wextra -Wpedantic -c matrix.c -o matrix.o || exit /b 1

ar rcs libdimred.a vector.o matrix.o || exit /b 1

gcc -std=c11 -Wall -Wextra -Wpedantic main.c -L. -ldimred -lm -o main.exe || exit /b 1

echo Build completed: main.exe

main.exe
pause
