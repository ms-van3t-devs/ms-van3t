#ifndef CAMRELAYERAMQP_H
#define CAMRELAYERAMQP_H

#include <proton/messaging_handler.hpp>
#include <proton/connection_options.hpp>
#include <proton/container.hpp>
#include <proton/work_queue.hpp>
#include <atomic> // For std::atomic<bool>
#include <condition_variable>

//define a structure for metadatas
typedef struct _GNmetadata {
uint64_t stationID;
int32_t lat;
int32_t lon;
uint32_t gn_timestamp;
} GNmetadata_t;

typedef struct _pthread_camrelayer_args
{
	std::string m_broker_address;
	std::string m_queue_name;
	std::string m_gn_tst_prop_name;
} pthread_camrelayer_args_t;

class CAMrelayerAMQP : public proton::messaging_handler {
	// For an example of usage of work_queue() to "inject" extra work (i.e. send CAMs) from external thread, see also:
	// http://qpid.apache.org/releases/qpid-proton-0.32.0/proton/cpp/examples/multithreaded_client.cpp.html
	pthread_camrelayer_args_t cr_arg_cl;         // AMQP and application parameters
	proton::work_queue *m_work_queue_ptr;        // Pointer to a work queue for "injecting" CAMs from an external thread
	proton::sender m_sender;                     // Sender to the CAM queue/topic
    std::atomic<bool> m_sender_ready;            // = true when the sender is ready (i.e. we can send CAMs), = false otherwise

	// Qpid Proton event callbacks
	void on_container_start(proton::container& c) override;
	void on_connection_open(proton::connection& c) override;
	void on_sender_open(proton::sender& protonsender) override;
	void on_sendable (proton::sender& sndr) override;
	void on_message(proton::delivery &dlvr, proton::message &msg) override;

	public:
		// Empty constructor
		// You must call set_args just after the usage of an empty constructor, otherwise the behaviour may be undefined
		CAMrelayerAMQP();

        CAMrelayerAMQP(proton::container& cont, const std::string& url, const std::string& address);

		// Full constructor (no need to call set_args() after using this constructor)
		CAMrelayerAMQP(const pthread_camrelayer_args_t camrelay_args);

		void set_args(const pthread_camrelayer_args_t camrelay_args);

		// Public function to be called from any external thread to trigger the transmission of a CAM
		// uint8_t *buffer should contain the CAM (encoded in any way, for instance with ASN.1 UPER)
		// int bufsize should contain the size, in bytes, of "buffer"
		void sendCAM_AMQP(uint8_t *buffer, int bufsize);

		// Public function to wait for the sender to be ready, before calling sendCAM_AMQP()
		// The application, after starting the container with run(), should call wait_sender_ready()
		// before attempting any call to sendCAM_AMQP(), otherwise CAMs may not be sent
		bool wait_sender_ready(void);
};

#endif // CAMRELAYERAMQP_H
