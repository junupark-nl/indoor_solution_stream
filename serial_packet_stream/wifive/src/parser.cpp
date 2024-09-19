#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <string>
#include <chrono>

#include "util.h"

std::string get_current_timestamp_filename(const std::string &relative_base_dir="") {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;

#ifdef _WIN32
    localtime_s(&now_tm, &now_c);
#else
    localtime_r(&now_c, &now_tm);~
#endif

    std::ostringstream date_oss;
    date_oss << std::put_time(&now_tm, "%Y-%m-%d");
    std::ostringstream time_oss;
    time_oss << std::put_time(&now_tm, "%H;%M;%S");

    // Get the current working directory
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path full_path = cwd / relative_base_dir / date_oss.str();

    // Normalize the path (resolves ".." and ".")
    full_path = std::filesystem::weakly_canonical(full_path);
    std::filesystem::create_directories(full_path);
    full_path /= time_oss.str() + ".csv";

    return full_path.string();
}

void readSerial(boost::asio::serial_port& serial, std::ofstream& csvFile) {
    std::vector<uint8_t> buffer(util::packet_size);
    boost::asio::read(serial, boost::asio::buffer(buffer, 1));  // Read one byte at a time

    if (buffer[0] == 0x59) {
        boost::asio::read(serial, boost::asio::buffer(buffer.data() + 1, 1));  // Read the second byte

        if (buffer[1] == 0x35) {
            auto arrivalTime = std::chrono::steady_clock::now();
            boost::asio::read(serial, boost::asio::buffer(buffer.data() + 2, util::packet_size - 2));

            float x = util::bytesToFloat(buffer, 2);
            float y = util::bytesToFloat(buffer, 6);
            float z = util::bytesToFloat(buffer, 10);
#ifdef CHECKSUM
            uint8_t receivedChecksum = buffer[14];
            uint8_t calculatedChecksum = util::calculateChecksum(std::vector<uint8_t>(buffer.begin(), buffer.end() - 1));
            if (receivedChecksum == calculatedChecksum) {
#endif
                auto micros = std::chrono::duration_cast<std::chrono::microseconds>(arrivalTime.time_since_epoch()).count();
                csvFile << micros << "," << x << "," << y << "," << z << std::endl;
#ifdef VERBOSE
                std::cout << "Data: " << x << ", " << y << ", " << z << " | micros: " << micros << " ms" << std::endl;
#endif
#ifdef CHECKSUM
            } else {
                std::cerr << "Checksum error!" << std::endl;
            }
#endif
        }
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
    std::string csv_filename = get_current_timestamp_filename("../../../../logs/serial_packet/wifive");
    std::cout << "Listening on port " << port << " at " << baudRate << " baud." << std::endl;
    std::cout << "CSV filename: " << csv_filename << std::endl;

    std::ofstream csv_file(csv_filename);
    if (!csv_file.is_open()) {
        std::cerr << "Error: Unable to open CSV file for writing." << std::endl;
        return 1;
    }
    csv_file << "ArrivalTimeUs,X,Y,Z" << std::endl;

    while (true) {
        try {
            readSerial(serial, csv_file);
        } catch (boost::system::system_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}
