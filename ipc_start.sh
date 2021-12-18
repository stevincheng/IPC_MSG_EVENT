CUR_PATH=`pwd`

init()
{
    export IPC_MSG_WORK_HOME_DIR=/home/stevin/user/IPC_MSG_EVENT/ipc_msg_work_dir
    export IPC_MSG_LOG_DIR=$IPC_MSG_WORK_HOME_DIR/log
    mkdir -p $IPC_MSG_LOG_DIR

    export LD_LIBRARY_PATH=$CUR_PATH/build/lib:$LD_LIBRARY_PATH
}
start_all_app(){
    $CUR_PATH/build/bin/log_manager &
    sleep 1
    $CUR_PATH/build/bin/ipc_event_server &
    sleep 1
    $CUR_PATH/build/bin/client_a &
    sleep 1
    $CUR_PATH/build/bin/client_b &
}
init
start_all_app