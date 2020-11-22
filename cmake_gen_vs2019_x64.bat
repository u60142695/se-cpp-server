if not exist "./cmake_vs2019_x64" (
	mkdir "cmake_vs2019_x64"
)

cd cmake_vs2019_x64

cmake -G "Visual Studio 16 2019" ..

pause