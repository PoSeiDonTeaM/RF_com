#include <iostream>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <fstream>

const int update_ms = 300;

int main() {
    std::cout << "Hello, Worlds" << std::endl;

    std::string host, username, password, database, port;
    std::ifstream infile;
    infile.open("config");
    if (!infile.is_open()) {
        std::cerr << "Please create a config file with: host username password database serialport" << std::endl;
        return 2;
    }
    infile >> host >> username >> password >> database >> port;
    infile.close();

    sql::Driver *driver;
    sql::Connection *con;
    sql::PreparedStatement *stMagnetic;
    sql::PreparedStatement *stSignal;

    // MySQL initialisation
    driver = get_driver_instance();
    con = driver->connect(host.c_str(), username.c_str(), password.c_str());
    con->setSchema(database.c_str());

    // Prepare insertion statements
    stMagnetic = con->prepareStatement("INSERT INTO mag(value,time) VALUES(?,NOW(2))     ON DUPLICATE KEY UPDATE value = ?");
    stSignal   = con->prepareStatement("INSERT INTO `signal`(value,time) VALUES(?,NOW(2))  ON DUPLICATE KEY UPDATE value = ?");

    try {
        // Serial interface initialisation
        boost::asio::io_service io;
        boost::asio::serial_port serial(io, port);
        serial.set_option(boost::asio::serial_port_base::baud_rate(9600));

        boost::asio::streambuf buf;
        std::istream is(&buf);
        std::istringstream iss;

        std::string line;
        boost::system::error_code ec;

        // Time when the last MySQL data was sent; used to prevent too frequent updates
        std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
        while (true) {
            try {
                // TODO: Move this into another function
                // TODO: Make this asynchronous
                boost::asio::read_until(serial, buf, '\n', ec);

                if (!ec) {
                    float valMagx, valMagy, valMagz, valTemp, valBat, valPressure, valSignal;
                    std::getline(is, line);
                    iss = std::istringstream(line);
                    iss >> valTemp >> valMagx >> valMagy >> valMagz >> valPressure >> valBat >> valSignal;

                    if (std::chrono::steady_clock::now() - last_update > std::chrono::milliseconds(update_ms)) {
                        std::cout
                            << valTemp << '\t'
                            << valMagx << '\t'
                            << valMagy << '\t'
                            << valMagz << '\t'
                            << valPressure << '\t'
                            << valBat << '\t'
                            << valSignal << '\t'
                            << std::endl;

                        float norm = sqrt(pow(valMagx,2) + pow(valMagy,2) + pow(valMagz,2));

                        // Perform the update
                        stMagnetic->setDouble(1, norm);
                        stMagnetic->setDouble(2, norm);
                        stMagnetic->execute();

                        stSignal->setDouble(1, valSignal);
                        stSignal->setDouble(2, valSignal);
                        stSignal->execute();

                        last_update = std::chrono::steady_clock::now();
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(5));

                    if (line == "stop") {
                        break;
                    }
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                }
            } catch (boost::system::system_error &e) {
                std::cerr << "Unable to execute " << e.what();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

    } catch (boost::system::system_error &e) {
        std::cerr << "Unable to open interface " << port << ": " << e.what();
    }

    delete stMagnetic;
    delete stSignal;
    delete con;

    return 0;
}
