/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <cinttypes>
#include <cstdio>

#include "cell_queue.hh"
#include "timestamp.hh"
#include "util.hh"

using namespace std;

CellQueue::CellQueue( const std::string & filename, 
                      const std::string & logfile,
                      const uint64_t queue_size )
    : next_delivery_( 0 ),
      schedule_(),
      init_timestamp_( timestamp() ),
      base_timestamp_( timestamp() ),
      queue_size_(queue_size),
      log_(),
      packet_queue_()
{
    assert_not_root();

    /* open filename and load schedule */
    FileDescriptor trace_file( SystemCall( "open", open( filename.c_str(), O_RDONLY ) ) );
    FILE *f = fdopen( trace_file.num(), "r" );
    if ( f == nullptr ) {
        throw Exception( "fopen" );
    }

    while ( true ) {
        uint64_t ms;
        int num_matched = fscanf( f, "%" PRIu64 "\n", &ms );
        if ( num_matched != 1 ) {
            break;
        }

        if ( not schedule_.empty() ) {
            if ( ms < schedule_.back() ) {
                throw Exception( filename, "timestamps must be monotonically nondecreasing" );
            }
        }

        schedule_.emplace_back( ms );
    }

    if ( schedule_.empty() ) {
        throw Exception( filename, "no valid timestamps found" );
    }

    /* open logfile if called for */
    if ( not logfile.empty() ) {
        log_.reset( new ofstream( logfile ) );
        if ( not log_->good() ) {
            throw runtime_error( logfile + ": error opening for writing" );
        }

        *log_ << "# mahimahi mm-link ( MP ) [" << filename << "] > " << logfile << endl;
        *log_ << "# command line: not available" << endl;
        *log_ << "# queue: not available" << endl;
        *log_ << "# init timestamp: " << base_timestamp_ << endl;
        *log_ << "# base timestamp: " << base_timestamp_ << endl;
    }

}

void CellQueue::record_arrival( const uint64_t arrival_time, const size_t pkt_size ) {
    /* log it */
    if ( log_ ) {
        *log_ << arrival_time << " + " << pkt_size << endl;
    }
}

void CellQueue::record_drop( const uint64_t time, const size_t pkts_dropped, const size_t bytes_dropped ) {
    /* log it */
    if ( log_ ) {
        *log_ << time << " d " << pkts_dropped << " " << bytes_dropped << endl;
    }
}

void CellQueue::record_departure_opportunity( void ) {
    /* log the delivery opportunity */
    if ( log_ ) {
        *log_ << next_delivery_time() - init_timestamp_ << " # " << PACKET_SIZE << endl;
    }
}

void CellQueue::record_departure( const uint64_t departure_time, const QueuedPacket & packet ) {
    /* log the delivery */
    if ( log_ ) {
        *log_ << departure_time - init_timestamp_ << " - " << packet.contents.size()
              << " " << departure_time - packet.arrival_time << endl;
    }
}

CellQueue::QueuedPacket::QueuedPacket( const std::string & s_contents )
    : bytes_to_transmit( s_contents.size() ),
      contents( s_contents ),
      arrival_time(timestamp())
{}

void CellQueue::read_packet( const string & contents )
{
    if (contents.size() <= queue_size_) {
        queue_size_ -= contents.size();
        packet_queue_.emplace( contents );
        record_arrival(packet_queue_.back().arrival_time - init_timestamp_, packet_queue_.back().contents.size());
    } else {
        record_drop(timestamp() - init_timestamp_, 1, contents.size());
    }
}

uint64_t CellQueue::next_delivery_time( void ) const
{
    return schedule_.at( next_delivery_ ) + base_timestamp_;
}

void CellQueue::use_a_delivery_opportunity( void )
{
    record_departure_opportunity();

    next_delivery_ = (next_delivery_ + 1) % schedule_.size();

    /* wraparound */
    if ( next_delivery_ == 0 ) {
        base_timestamp_ += schedule_.back();
    }
}

void CellQueue::write_packets( FileDescriptor & fd )
{
    uint64_t now = timestamp();

    while ( next_delivery_time() <= now ) {
        /* burn a delivery opportunity */
        unsigned int bytes_left_in_this_delivery = PACKET_SIZE;
        use_a_delivery_opportunity();

        while ( (bytes_left_in_this_delivery > 0)
                and (not packet_queue_.empty()) ) {
            packet_queue_.front().bytes_to_transmit -= bytes_left_in_this_delivery;
            bytes_left_in_this_delivery = 0;

            if ( packet_queue_.front().bytes_to_transmit <= 0 ) {
                /* restore the surplus bytes beyond what the packet requires */
                bytes_left_in_this_delivery += (- packet_queue_.front().bytes_to_transmit);
                queue_size_ += packet_queue_.front().contents.size();
                record_departure(now, packet_queue_.front());
                /* this packet is ready to go */
                fd.write( packet_queue_.front().contents );
                packet_queue_.pop();
            }
        }
    }
}

int CellQueue::wait_time( void ) const
{
    return packet_queue_.empty() ? UINT16_MAX : (next_delivery_time() - timestamp());
}
