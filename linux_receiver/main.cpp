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

const char *port = "/dev/ttyACM0";
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
    sql::PreparedStatement *pot0;
    sql::PreparedStatement *pot1;
    sql::PreparedStatement *potl;
    sql::PreparedStatement *potb;

    // MySQL initialisation
    driver = get_driver_instance();
    con = driver->connect(host.c_str(), username.c_str(), password.c_str());
    con->setSchema(database.c_str());

    // Prepare insertion statements
    pot0 = con->prepareStatement("INSERT INTO pot0(value,time) VALUES(?,CURRENT_TIMESTAMP)  ON DUPLICATE KEY UPDATE value = ?");
    pot1 = con->prepareStatement("INSERT INTO pot1(value,time) VALUES(?,CURRENT_TIMESTAMP)  ON DUPLICATE KEY UPDATE value = ?");
    potl = con->prepareStatement("INSERT INTO level(value,time) VALUES(?,CURRENT_TIMESTAMP) ON DUPLICATE KEY UPDATE value = ?");
//    potb = con->prepareStatement("INSERT INTO battery(value,time) VALUES(?,CURRENT_TIMESTAMP) ON DUPLICATE KEY UPDATE value = ?");

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
                    int potv1 = -2;
                    float potv0 = -1, potvl = 0, potvb = -3;
                    std::getline(is, line);
                    iss = std::istringstream(line);
                    iss >> potv0 >> potv1 >> potvl >> potvb;

                    if (std::chrono::steady_clock::now() - last_update > std::chrono::milliseconds(update_ms)) {
                        std::cout << potv0 << '\t' << potv1 << '\t' << potvl << '\t' << potvb << '\t' << '.' << std::endl;

                        // Perform the update
                        pot0->setDouble(1, potv0);
                        pot0->setDouble(2, potv0);
                        pot0->execute();
                        pot1->setInt(1, potv1);
                        pot1->setInt(2, potv1);
                        pot1->execute();
                        potl->setDouble(1, potvl);
                        potl->setDouble(2, potvl);
                        potl->execute();
    //                    potb->setDouble(1, potvb);
  //                      potb->setDouble(2, potvb);
      //                  potb->execute();

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

    delete pot0;
    delete pot1;
    delete potl;
//    delete potb;
    delete con;

    return 0;
}
