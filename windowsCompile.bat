@echo off

:: If you are having compilation errors which does not generate an ouput file:
:: Go to MinGW dir and rename mingw32 to mingw32old

g++ -Wall UnitTests/test.cpp Socket/Socket.cpp ServerSocket/ServerSocket.cpp -std=c++11 -lws2_32 -Llibs/ -lbthprops -liphlpapi -lBluetoothApis -o WindowsTest

pause
