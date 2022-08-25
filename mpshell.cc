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

        if ( argc < 20 ) {
            throw Exception( "Usage", string( argv[ 0 ] ) + " cell_delay "
            "cell_uplink cell_downlink cell_up_loss_float cell_down_loss_float "
            "cell_up_queue_bytes cell_down_queue_bytes cell_uplink_log "
            "cell_downlink_log "
            "wifi_delay wifi_uplink wifi_downlink wifi_up_loss_float "
            "wifi_down_loss_float wifi_up_queue_bytes wifi_down_queue_bytes "
            "wifi_uplink_log wifi_downlink_log log_on program" );
        }

        const uint64_t cell_delay = myatoi( argv[ 1 ] );
        const std::string cell_uplink = argv[ 2 ];
        const std::string cell_downlink = argv[ 3 ];
        const float cell_up_loss = atof(argv[ 4 ]);
        const float cell_down_loss = atof(argv[ 5 ]);
        const uint64_t cell_up_queue = myatoi(argv[ 6 ]);
        const uint64_t cell_down_queue = myatoi(argv[ 7 ]);
        const std::string cell_uplink_log = argv[ 8 ];
        const std::string cell_downlink_log = argv[ 9 ];

        const uint64_t wifi_delay = myatoi( argv[ 10 ] );
        const std::string wifi_uplink = argv[ 11 ];
        const std::string wifi_downlink = argv[ 12 ];
        const float wifi_up_loss = atof(argv[ 13 ]);
        const float wifi_down_loss = atof(argv[ 14 ]);
        const uint64_t wifi_up_queue = myatoi(argv[ 15 ]);
        const uint64_t wifi_down_queue = myatoi(argv[ 16 ]);
        const std::string wifi_uplink_log = argv[ 17 ];
        const std::string wifi_downlink_log = argv[ 18 ];
        const int log_on = myatoi(argv[19]);

        vector< string > program_to_run;
        for ( int num_args = 20; num_args < argc; num_args++ ) {
            program_to_run.emplace_back( string( argv[ num_args ] ) );
        }

        PacketShell mp_shell_app( "cw" );

        mp_shell_app.start_uplink( "[ mpshell ] ",
                                   user_environment,
                                   cell_delay,
                                   wifi_delay,
                                   cell_up_loss,
                                   wifi_up_loss,
                                   cell_up_queue,
                                   wifi_up_queue,
                                   cell_uplink,
                                   wifi_uplink,
                                   cell_uplink_log,
                                   wifi_uplink_log,
                                   log_on,
                                   program_to_run);

        mp_shell_app.start_downlink( cell_delay, wifi_delay, cell_down_loss, 
                                     wifi_down_loss, cell_down_queue, 
                                     wifi_down_queue, cell_downlink, 
                                     wifi_downlink, cell_downlink_log,
                                     wifi_downlink_log, log_on);
        return mp_shell_app.wait_for_exit();
    } catch ( const Exception & e ) {
        e.perror();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
