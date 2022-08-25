/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef CELL_QUEUE_HH
#define CELL_QUEUE_HH

#include <queue>
#include <cstdint>
#include <string>
#include <fstream>
#include <memory>

#include "file_descriptor.hh"

class CellQueue
{
private:
    const static unsigned int PACKET_SIZE = 1504; /* default max TUN payload size */

    struct QueuedPacket
    {
        int bytes_to_transmit;
        std::string contents;
        uint64_t arrival_time;

        QueuedPacket( const std::string & s_contents );
    };

    unsigned int next_delivery_;
    std::vector< uint64_t > schedule_;
    uint64_t init_timestamp_;
    uint64_t base_timestamp_;
    uint64_t queue_size_;
    std::unique_ptr<std::ofstream> log_;

    std::queue< QueuedPacket > packet_queue_;

    uint64_t next_delivery_time( void ) const;

    void use_a_delivery_opportunity( void );

    void record_arrival( const uint64_t arrival_time, const size_t pkt_size );
    void record_drop( const uint64_t time, const size_t pkts_dropped, const size_t bytes_dropped );
    void record_departure_opportunity( void );
    void record_departure( const uint64_t departure_time, const QueuedPacket & packet );

public:
    CellQueue( const std::string & filename, const std::string & logfile, uint64_t queue_size, const int & log_on);

    void read_packet( const std::string & contents );

    void write_packets( FileDescriptor & fd );

    int wait_time( void ) const;
};

#endif /* CELL_QUEUE_HH */
