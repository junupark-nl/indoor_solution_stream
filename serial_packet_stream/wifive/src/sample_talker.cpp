#include <iostream>
#include <cstdlib>  // For std::rand()

#include "util.h"

// Function to send data packet (preamble + 3 floats + checksum)
void sendPacket(boost::asio::serial_port& serial, float val1, float val2, float val3) {
    std::vector<uint8_t> packet;

    // Add preambles
    packet.push_back(0x59);
    packet.push_back(0x35);

    // Convert floats to bytes and add them to the packet
    std::vector<uint8_t> bytesVal1 = util::floatToBytes(val1);
    std::vector<uint8_t> bytesVal2 = util::floatToBytes(val2);
    std::vector<uint8_t> bytesVal3 = util::floatToBytes(val3);

    packet.insert(packet.end(), bytesVal1.begin(), bytesVal1.end());
    packet.insert(packet.end(), bytesVal2.begin(), bytesVal2.end());
    packet.insert(packet.end(), bytesVal3.begin(), bytesVal3.end());

#ifdef CHECKSUM
    // Calculate checksum (excluding the checksum byte itself)
    uint8_t checksum = util::calculateChecksum(packet);
    packet.push_back(checksum);
#endif

    // Send packet through serial port
    boost::asio::write(serial, boost::asio::buffer(packet));
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
    std::cout << "Sending data on port " << port << " at " << baudRate << " baud." << std::endl;

    // Initialize random seed
    std::srand(0);
    while (true) {
        try {
            float val1 = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX / 100.0);  // Random between 0 and 100
            float val2 = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX / 100.0);
            float val3 = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX / 100.0);
            sendPacket(serial, val1, val2, val3);

#ifdef VERBOSE
            std::cout << "Sent: " << val1 << ", " << val2 << ", " << val3 << std::endl;
#endif
            std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Send every 1 second
        } catch (boost::system::system_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
