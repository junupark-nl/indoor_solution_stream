#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#define VERBOSE
#define CHECKSUM

namespace util {

#ifdef CHECKSUM
const int packet_size = 15;  // 14 bytes + 1 for checksum
#else
const int packet_size = 14;
#endif

typedef union {
    float f;
    uint8_t b[4];
} FloatUnion;

std::vector<uint8_t> floatToBytes(float value) {
    FloatUnion u;
    u.f = value;
#ifdef BIG_ENDIAN
    return std::vector<uint8_t>(u.b + 4, u.b);
#else // Little endian, default
    return std::vector<uint8_t>(u.b, u.b + 4);
#endif
}

float bytesToFloat(const std::vector<uint8_t>& bytes, int start) {
    FloatUnion u;

#ifdef BIG_ENDIAN
    u.b[0] = bytes[start + 3];
    u.b[1] = bytes[start + 2];
    u.b[2] = bytes[start + 1];
    u.b[3] = bytes[start];
#else // Little endian, default
    u.b[0] = bytes[start];
    u.b[1] = bytes[start + 1];
    u.b[2] = bytes[start + 2];
    u.b[3] = bytes[start + 3];
#endif

    return u.f;
}

// Function to calculate checksum (XOR of all bytes)
uint8_t calculateChecksum(const std::vector<uint8_t>& data) {
    uint8_t checksum = 0;
    for (const auto& byte : data) {
        checksum ^= byte;
    }
    return checksum;
}

void setupSerialPort(boost::asio::io_service& io, boost::asio::serial_port& serial, const std::string& port, unsigned int baudRate) {
    serial.open(port);
    serial.set_option(boost::asio::serial_port_base::baud_rate(baudRate));
    serial.set_option(boost::asio::serial_port_base::character_size(8));
    serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
}

}  // namespace util