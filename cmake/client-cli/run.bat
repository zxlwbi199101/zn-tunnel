cmake -B build -D Boost_ROOT=C:\Users\ZhaoXinlei\cpplibs\boost_1_73_0 -D BOOST_NO_SYSTEM_PATHS=ON

& 'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe' .\build\client-cli.sln

.\Debug\client.exe