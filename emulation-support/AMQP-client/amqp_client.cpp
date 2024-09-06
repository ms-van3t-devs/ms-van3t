//
// Created by carlos on 05/02/24.
//
#include <proton/message.hpp>
#include <proton/connection_options.hpp>
#include <proton/ssl.hpp>
#include <proton/source_options.hpp>
#include <proton/receiver_options.hpp>
#include <iostream>
#include "amqp_client.h"
#include "C-ITS/ETSImessageHandler.h"
#include <string>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void AMQP_client::set_no_local_filter(proton::source_options &opts) {
    proton::source::filter_map map;
    proton::symbol filter_key("no-local");
    proton::value filter_value;

    // The value is a specific AMQP "described type": list with symbolic descriptor
    proton::codec::encoder enc(filter_value);
    enc << proton::codec::start::described()
        << proton::symbol("apache.org:no-local-filter:list")
        << proton::codec::start::list() << proton::codec::finish()
        << proton::codec::finish();

    // In our case, the map has this one element
    map.put(filter_key, filter_value);
    opts.filters(map);
}

void AMQP_client::on_container_start(proton::container &c) {
    proton::connection_options co;
    co.idle_timeout(proton::duration(10000));
    //c.connect(cr_arg_cl.m_broker_address,co.idle_timeout(proton::duration::FOREVER));
    c.connect(m_url,co);

}

void AMQP_client::on_connection_open(proton::connection &c) {
    proton::source_options so;
    set_no_local_filter(so);
    c.open_receiver(m_topic, proton::receiver_options().source(so));
    c.open_sender(m_topic);

    std::cout << "[AMQP client] AMQP connection opened: " << m_url << " topic: " << m_topic << std::endl;
}

void AMQP_client::on_sender_open(proton::sender &protonsender) {
    m_sender=protonsender;

    // Get the work queue pointer out of the sender
    m_work_queue_ptr=&m_sender.work_queue();

    // Set "m_sender_ready" to true -> now the sender is ready and the application can safely call sendCAM_AMQP()
    m_sender_ready=true;
}

void AMQP_client::on_sendable(proton::sender &sndr) {
    messaging_handler::on_sendable(sndr);
}

void AMQP_client::on_message(proton::delivery &dlvr, proton::message &msg) {
    proton::codec::decoder qpid_decoder(msg.body());
    proton::binary message_bin;
    uint8_t *message_bin_buf;

    // Check if a binary message has been received
    // If no binary data has been received, just ignore the current AMQP message
    if(qpid_decoder.next_type () == proton::BINARY) {
        qpid_decoder >> message_bin;

        message_bin_buf=message_bin.data ();
        std::cout << "[AMQP client] Received a message of type " << msg.properties().get("type") << std::endl;
    } else {
            std::cout << "[AMQP client] Error: received a message in a non-binary AMQP type." << std::endl;
    }

    // Extract the stationID from the AMQP message
    proton::scalar stationID_prop = msg.properties().get("stationID");
    int stationID;
    if(stationID_prop.type() == proton::INT) {
        stationID = proton::get<int>(stationID_prop);}


    ETSImessageHandler handler;
    ETSImessageHandler::etsiDecodedData_t decoded_data;
    if (handler.decodeMessage((uint8_t *)message_bin_buf, message_bin.size(), decoded_data) == 0)
    {
        std::string json_string = decoded_data.json_msg.dump();
        std::cout << "[AMQP client] Message successfully decoded into JSON: " << std::endl << json_string << std::endl;
    }
    else
    {
        std::cout << "[AMQP client] Error decoding the received message" << std::endl;
    }
}

AMQP_client::AMQP_client(const std::string &url,
                         const std::string &topic)
        {
    m_url = url;
    m_topic = topic;
}

void AMQP_client::on_transport_error(proton::transport &transport) {
    std::cout << "[AMQP client] TRANSPORT ERROR" << std::endl;
    messaging_handler::on_transport_error(transport);
}

void AMQP_client::on_transport_close(proton::transport &transport) {
    std::cout << "[AMQP client] TRANSPORT CLOSE" << std::endl;
    messaging_handler::on_transport_close(transport);
}

void AMQP_client::on_connection_close(proton::connection &connection) {
    std::cout << "[AMQP client] CONNECTION CLOSE" << std::endl;
    messaging_handler::on_connection_close(connection);
}


void AMQP_client::on_connection_error(proton::connection &connection) {
    std::cout << "[AMQP client] CONNECTION ERROR" << std::endl;
    messaging_handler::on_connection_error(connection);
}

AMQP_client::AMQP_client() {

    m_url = "127.0.0.1:5672";
    m_topic = "topic://test";
}


