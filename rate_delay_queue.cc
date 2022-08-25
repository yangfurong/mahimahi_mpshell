#include "rate_delay_queue.hh"

using namespace std;

void RateDelayQueue::write_packets( FileDescriptor & fd ) {
    /* Move packets from delay_queue_ into cell_queue_ */
    string next_packet = delay_queue_.get_next();
    while ( not next_packet.empty() ) {
        //support loss rate >= 0.000000001 (0.0000001%)
        float rate = rand() % 1000000000 / (float)(1000000000);
        if (rate > loss)
            cell_queue_.read_packet( next_packet );
        next_packet = delay_queue_.get_next();
    }

    /* Write out packets from cell_queue_ into fd */
    cell_queue_.write_packets( fd );
}
