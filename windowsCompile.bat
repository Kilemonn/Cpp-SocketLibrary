@echo off

:: If you are having compilation errors which does not generate an ouput file:
:: Go to MinGW dir and rename mingw32 to mingw32old

g++ -Wall UnitTests/test.cpp Socket/Socket.cpp ServerSocket/ServerSocket.cpp -std=c++11 -lws2_32 -Llibs/ -lbthprops -liphlpapi -lBluetoothApis -pthread -o WindowsTest

::g++ -Wall UnitTests/TestTCP.cpp Socket/Socket.cpp ServerSocket/ServerSocket.cpp -std=c++11 -lws2_32 -Llibs/ -lbthprops -liphlpapi -lBluetoothApis -pthread -o WindowsTestTCP

::g++ -Wall UnitTests/TestUDP.cpp Socket/Socket.cpp ServerSocket/ServerSocket.cpp -std=c++11 -lws2_32 -Llibs/ -lbthprops -liphlpapi -lBluetoothApis -pthread -o WindowsTestUDP

pause
