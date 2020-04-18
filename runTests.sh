
valgrind --log-fd=1 --log-file="TCPText.txt" --tool=memcheck --leak-check=full -v ./TestTCP
valgrind --log-file="UDPText.txt" --tool=memcheck --leak-check=full -v ./TestUDP
