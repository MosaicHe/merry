#include "merry.h"

char bind_addr[20] = {0};
int bind_port = 1111;
const char *program_name = NULL;

int merry_start(int argc, const char **argv, void (*help)(), void (*master)(),
                void (*onexit)(), void (*worker)())
{
    update_time();

    /// 初始化进程命令行信息
    init_process_title(argc, argv);

    int i = strlen(argv[0]);

    while(argv[0][--i] != '/');

    program_name = argv[0] + i + 1;

    if(getarg("help")) {
        help();
        exit(0);
    }

    if(getarg("log")) {
        open_log(getarg("log"), 40960); // filename, bufsize
    }

    /// 把进程放入后台
    if(getarg("daemon")) {
        daemonize();
    }

    process_count = 1;

    if(is_daemon == 1) {
        process_count = atoi(getarg("daemon"));

        if(process_count < 1) {
            process_count = get_cpu_num();
        }
    }

    sprintf(bind_addr, "0.0.0.0");

    if(getarg("bind")) {
        sprintf(bind_addr, "%s", getarg("bind"));
    }

    char *_port = strstr(bind_addr, ":");

    if(_port) {
        bind_addr[strlen(bind_addr) - strlen(_port)] = '\0';
        _port = _port + 1;

        if(atoi(_port) > 0 && atoi(_port) < 99999) {
            bind_port = atoi(_port);
        }
    }

    server_fd = network_bind(bind_addr, bind_port);
    LOGF(INFO, "bind %s:%d", bind_addr, bind_port);

    for(i = 0; i < process_count; i++) {
        if(is_daemon == 1) {
            fork_process(worker);

        } else {
            set_cpu_affinity(0);
            new_thread_p(worker, 0);
        }
    }

    /// 设置进程归属用户，默认 nobody
    set_process_user(/*user*/ NULL, /*group*/ NULL);

    /// 进入主进程处理
    start_master_main(master, onexit);

    return 1;
}