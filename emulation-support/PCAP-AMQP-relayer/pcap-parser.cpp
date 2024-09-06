#include <cstring>
#include <iostream>
#include <pcap.h>
#include <time.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include "tclap/CmdLine.h"

//Build command
//g++ pcap-parser.cpp obj/Rawsock_lib/rawsock.o obj/Rawsock_lib/minirighi_udp_checksum.o obj/Rawsock_lib/ipcsum_alth.o obj/Rawsock_lib/rawsock_lamp.o -lpthread -lqpid-proton-cpp -lpcap -Wall -O3 -Iinclude -IRawsock_lib/Rawsock_lib -I. -Wall -O3 -IRawsock_lib/Rawsock_lib -o PCAPparser

using namespace std;


int main(int argc, char *argv[])
{

    string inFile,outFile;
    unsigned char offset[2];
    int traces;

    char errbuff[PCAP_ERRBUF_SIZE];

    // Parse the command line options with the TCLAP library
    try {
        TCLAP::CmdLine cmd("PCAP parser", ' ', "1.0");

        // Arguments: short option, long option, description, is it mandatory?, default value, type indication (just a string to help the user)
        TCLAP::ValueArg<std::string> inArg("I","in","Input .pcap file",false,"/home/cnituser/PCAP-AMQP-relayer/ms-van3t-30v-3600.pcap","string");
        cmd.add(inArg);

        TCLAP::ValueArg<std::string> outArg("O","out","Output file name",false,"/home/cnituser/PCAP-AMQP-relayer/out-pcaps/trace-ID-","string");
        cmd.add(outArg);

        TCLAP::ValueArg<std::string> offset0Arg("B","offset0","StatioID offset",false,"00","string");
        cmd.add(offset0Arg);

        TCLAP::ValueArg<std::string> offset1Arg("V","offset1","StatioID offset",false,"00","string");
        cmd.add(offset1Arg);

        TCLAP::ValueArg<int> tracesArg("T","traces","Number of new traces (1 by default)",false,1,"int");
        cmd.add(tracesArg);

        cmd.parse(argc,argv);

        inFile=inArg.getValue();
        outFile=outArg.getValue();
        offset[0] = stoi(offset0Arg.getValue(),0,16);
        offset[1] = stoi(offset1Arg.getValue(),0,16);
        traces=tracesArg.getValue();

    } catch (TCLAP::ArgException &tclape) {
        std::cerr << "TCLAP error: " << tclape.error() << " for argument " << tclape.argId() << std::endl;
    }





//    // Use pcap_open_offline
//    pcap_t * pcap = pcap_open_offline(inFile.c_str(), errbuff);

    uint16_t start = static_cast<int> (offset[0])*256 + static_cast<int> (offset[1]);



    for (uint16_t counter=0 ; counter<traces ; counter++)
    {
        struct pcap_pkthdr *header;
        const u_char *data;
        u_int packetCount = 0;
        string wFile;

        // Use pcap_open_offline
        pcap_t * pcap = pcap_open_offline(inFile.c_str(), errbuff);

        wFile = outFile + std::to_string(static_cast<int> (counter + start)) + ".pcap";
        pcap_dumper_t * pcap_out = pcap_dump_open_append(pcap, wFile.c_str());

        //Read and write packet by packet on the new trace
        while (int returnValue = pcap_next_ex(pcap, &header, &data) >= 0)
        {
            printf("Packet # %i\n", ++packetCount);

            // Show the size in bytes of the packet
            printf("Packet size: %d bytes\n", header->len);

            // Show a warning if the length captured is different
            if (header->len != header->caplen)
                printf("Warning! Capture size different than packet size: %u bytes\n", header->len);

            // Just take CAMs packets
            if((header->len > 130)&&(header->len < 170)) {

                //Skip till the start of GNmetadata
                data+=42;
                //Copy the new ID offset in the second octet of the ID field
                //For the GNmetadata
                memcpy((uint8_t *)data+6,offset,2);
                //For the actual CAM StationID field
                memcpy((uint8_t *)data+72,offset,2);

                //Shrink packet size for the new pcap
                header->len -= 42;
                header->caplen -= 42;

                //Write this packet to the out .pcap file
                pcap_dump((u_char*)pcap_out, header,data);

                //Print written packet
                for (u_int i=0; (i < (header->len) ) ; i++)
                {
                    // Start printing on the next after every 16 octets
                    if ( (i % 16) == 0) printf("\n");

                    // Print each octet as hex (x), make sure there is always two characters (.2).
                    printf("%.2x ", data[i]);
                }
            }

            // Add two lines between packets
            printf("\n\n");
        }

        if (static_cast<int> (offset[1]) == 255)
        {
            offset[0] ++;
        }
        offset[1] ++;
    }
}
