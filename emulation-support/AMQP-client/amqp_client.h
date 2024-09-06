//
// Created by carlos on 05/02/24.
//

#ifndef ZMQ_PROXY_AMQP_CLIENT_H
#define ZMQ_PROXY_AMQP_CLIENT_H

#include <proton/messaging_handler.hpp>
#include <proton/container.hpp>
#include <proton/work_queue.hpp>
#include <atomic>
#include <mutex>

class AMQP_client : public proton::messaging_handler {
    std::string m_url;                           // URL of the AMQP broker
    std::string m_topic;                         // Topic
    proton::work_queue *m_work_queue_ptr;        // Pointer to a work queue for "injecting" CAMs from an external thread
    proton::sender m_sender;
    std::atomic<bool> m_sender_ready;

    // SSL configuration
    std::string cert_file;  // Path to SSL certificate file
    std::string key_file;   // Path to SSL private key file
    std::string ca_file;    // Path to CA certificate file
    bool verify_peer;       // Whether to verify the peer's certificate



    // Qpid Proton event callbacks
    void on_container_start(proton::container& c) override;
    void on_connection_open(proton::connection& c) override;
    void on_sender_open(proton::sender& protonsender) override;
    void on_sendable (proton::sender& sndr) override;
    void on_message(proton::delivery &dlvr, proton::message &msg) override;
    void on_transport_error(proton::transport &transport) override;
    void on_transport_close(proton::transport &transport) override;
    void on_connection_close(proton::connection &connection) override;
    void on_connection_error(proton::connection &connection) override;

public:
    // Empty constructor
    // You must call set_args just after the usage of an empty constructor, otherwise the behaviour may be undefined
    AMQP_client();
    AMQP_client(const std::string &url,
                const std::string &topic);


private:
    void set_no_local_filter(proton::source_options &opts);
};


#endif //ZMQ_PROXY_AMQP_CLIENT_H
