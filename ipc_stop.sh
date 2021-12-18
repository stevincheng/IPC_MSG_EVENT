CUR_PATH=`pwd`

stop()
{
    killall client_a
    killall client_b
    killall ipc_event_server
    killall log_manager
}

stop