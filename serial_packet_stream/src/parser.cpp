#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>

// Function to calculate checksum (XOR of all bytes)
uint8_t calculateChecksum(const std::vector<uint8_t>& data) {
    uint8_t checksum = 0;
    for (const auto& byte : data) {
        checksum ^= byte;
    }
    return checksum;
}

// Function to convert 4 bytes to float (Intel format)
float bytesToFloat(const std::vector<uint8_t>& bytes, int start) {
    union {
        float f;
        uint8_t b[4];
    } u;

    // Ensure the correct byte order for Intel format (little endian)
    u.b[0] = bytes[start];
    u.b[1] = bytes[start + 1];
    u.b[2] = bytes[start + 2];
    u.b[3] = bytes[start + 3];

    return u.f;
}

// Function to read serial data
void readSerial(boost::asio::serial_port& serial) {
    std::vector<uint8_t> buffer(15);  // 14 bytes + 1 for checksum
    boost::asio::read(serial, boost::asio::buffer(buffer, 15));

    if (buffer[0] == 0x59 && buffer[1] == 0x35) {
        // Parse the three floats
        float val1 = bytesToFloat(buffer, 2);
        float val2 = bytesToFloat(buffer, 6);
        float val3 = bytesToFloat(buffer, 10);

        // Calculate and verify the checksum
        uint8_t receivedChecksum = buffer[14];
        uint8_t calculatedChecksum = calculateChecksum(std::vector<uint8_t>(buffer.begin(), buffer.end() - 1));

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

    try {
        boost::asio::io_service io;
        boost::asio::serial_port serial(io, port);

        // Set baud rate and other serial options
        serial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));

        std::cout << "Listening on port " << port << " at " << baudRate << " baud." << std::endl;

        while (true) {
            readSerial(serial);
        }
    } catch (boost::system::system_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
