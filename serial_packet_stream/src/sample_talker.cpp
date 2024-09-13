#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <COM port> <baud rate>" << std::endl;
        return 1;
    }

    // Parse command line arguments
    std::string port = argv[1];
    int baudRate = std::stoi(argv[2]);

    // Create an I/O service
    boost::asio::io_service io;

    // Create a serial port object
    boost::asio::serial_port serial(io, port);
    serial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));

    // Write a message to the serial port
    std::string message = "Hello, world!";
    boost::asio::write(serial, boost::asio::buffer(message));

    // Read a message from the serial port
    std::vector<char> buffer(100);
    size_t bytesRead = serial.read_some(boost::asio::buffer(buffer));

    // Print the received message
    std::cout << "Received: " << std::string(buffer.begin(), buffer.begin() + bytesRead) << std::endl;

    return 0;
}