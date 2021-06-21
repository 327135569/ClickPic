@SET CURRENTDIR="%cd%"
@call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars32.bat"
@cd %CURRENTDIR%
cmake -G Ninja -B build
cmake --build build --target main
@call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
@cd %CURRENTDIR%
cmake -G Ninja -B build_driver -D DRIVER=1
cmake --build build_driver --target click

