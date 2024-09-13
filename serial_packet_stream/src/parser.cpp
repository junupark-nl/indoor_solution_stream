#include <iostream>
#include <iomanip>
#include <string>

#include "util.h"

void readSerial(boost::asio::serial_port& serial) {
    std::vector<uint8_t> buffer(15);  // 14 bytes + 1 for checksum
    boost::asio::read(serial, boost::asio::buffer(buffer, 15));

    if (buffer[0] == 0x59 && buffer[1] == 0x35) {
        // Parse the three floats
        float val1 = util::bytesToFloat(buffer, 2);
        float val2 = util::bytesToFloat(buffer, 6);
        float val3 = util::bytesToFloat(buffer, 10);

        // Calculate and verify the checksum
        uint8_t receivedChecksum = buffer[14];
        uint8_t calculatedChecksum = util::calculateChecksum(std::vector<uint8_t>(buffer.begin(), buffer.end() - 1));

        if (receivedChecksum == calculatedChecksum) {
            std::cout << "Data: " << val1 << ", " << val2 << ", " << val3 << std::endl;
        } else {
            std::cerr << "Checksum error!" << std::endl;
        }
    } else {
        std::cerr << "Invalid preamble!" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <COM port> <baud rate>" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    unsigned int baudRate = std::stoi(argv[2]);
    boost::asio::io_service io;
    boost::asio::serial_port serial(io);

    try {
        util::setupSerialPort(io, serial, port, baudRate);
    } catch (boost::system::system_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Listening on port " << port << " at " << baudRate << " baud." << std::endl;

    while (true) {
        try {
            readSerial(serial);
        } catch (boost::system::system_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
