/* -*-mode:c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "packetshell.hh"

using namespace std;

int main( int argc, char *argv[] )
{
    try {
        /* clear environment while running as root */
        char ** const user_environment = environ;
        environ = nullptr;

        check_requirements( argc, argv );

        if ( argc < 12 ) {
            throw Exception( "Usage", string( argv[ 0 ] ) + " cell_delay cell_uplink cell_downlink cell_loss_float cell_queue_bytes wifi_delay wifi_uplink wifi_downlink wifi_loss_float wifi_queue_bytes program" );
        }

        const uint64_t cell_delay = myatoi( argv[ 1 ] );
        const std::string cell_uplink = argv[ 2 ];
        const std::string cell_downlink = argv[ 3 ];
        const float cell_loss = atof(argv[ 4 ]);
        const uint64_t cell_queue = myatoi(argv[ 5 ]);
        const uint64_t wifi_delay = myatoi( argv[ 6 ] );
        const std::string wifi_uplink = argv[ 7 ];
        const std::string wifi_downlink = argv[ 8 ];
        const float wifi_loss = atof(argv[ 9 ]);
        const uint64_t wifi_queue = myatoi(argv[ 10 ]);

        vector< string > program_to_run;
        for ( int num_args = 11; num_args < argc; num_args++ ) {
            program_to_run.emplace_back( string( argv[ num_args ] ) );
        }

        PacketShell mp_shell_app( "cw" );

        mp_shell_app.start_uplink( "[ mpshell ] ",
                                   user_environment,
                                   cell_delay,
                                   wifi_delay,
                                   cell_loss,
                                   wifi_loss,
                                   cell_queue,
                                   wifi_queue,
                                   cell_uplink,
                                   wifi_uplink,
                                   program_to_run);

        mp_shell_app.start_downlink( cell_delay, wifi_delay, cell_loss, wifi_loss, cell_queue, wifi_queue, cell_downlink, wifi_downlink );
        return mp_shell_app.wait_for_exit();
    } catch ( const Exception & e ) {
        e.perror();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
