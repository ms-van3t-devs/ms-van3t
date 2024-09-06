//
// Created by Carlos Risma
//
#include <signal.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <cstring>
#include <nlohmann/json.hpp>
#include <sys/wait.h>
#include <sys/socket.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include "C-ITS/utils.h"
#include "C-ITS/Seq.hpp"
#include "C-ITS/Getter.hpp"
#include "C-ITS/Setter.hpp"
#include "C-ITS/Encoding.hpp"
#include "C-ITS/SetOf.hpp"
#include "C-ITS/SequenceOf.hpp"
#include "C-ITS/BitString.hpp"
#include <queue>
#include <functional>
#include <chrono>
#include <fcntl.h>

// TCLAP headers
#include "tclap/CmdLine.h"
#include "C-ITS/ETSImessageHandler.h"
#include <proton/connection.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/tracker.hpp>
#include <proton/connection_options.hpp>
#include <netinet/tcp.h>
#include "amqp_client.h"



int main (int argc, char *argv[]) {
    std::string destIp = "172.17.0.2";
    std::string broker_address;
    std::string queue_name;

    // Parse the command line options with the TCLAP library
    try {
        TCLAP::CmdLine cmd("AMQP client for C-ITS", ' ', "0.1.0");

        // Arguments: short option, long option, description, is it mandatory?, default value, type indication (just a string to help the user)
        TCLAP::ValueArg<std::string> destIpArg("D", "dest-ip", "IP for the UDP communication", false, "172.21.19.57",
                                               "string");
        cmd.add(destIpArg);

        TCLAP::ValueArg<std::string> urlArg("U","url","Broker URL (with port)",false,"172.21.19.57:5672","string");
        cmd.add(urlArg);

        TCLAP::ValueArg<std::string> queueArg("Q","queue","Broker queue or topic",false,"topic://MWCdemo","string");
        cmd.add(queueArg);


        cmd.parse(argc, argv);


        destIp = destIpArg.getValue();
        broker_address = urlArg.getValue();
        queue_name = queueArg.getValue();

    } catch (TCLAP::ArgException &tclape) {
        std::cerr << "TCLAP error: " << tclape.error() << " for argument " << tclape.argId() << std::endl;
    }


    AMQP_client amqp_client(broker_address,
                            queue_name);

    proton::container container(amqp_client);
    try{
        container.run();
    } catch (const std::exception &e) {
        std::cerr << "Error in AMQP client: " << e.what() << std::endl;
    }


    return 0;
}