#include <pistache/net.h>
#include "rest/endpoints.hh"
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>
#include "app_config.hh"
extern "C" {
#include "codecs/codec_module.h"
}
namespace spd = spdlog;
using namespace Pistache;

#include "test_main.hh"

int main(int argc, char** argv) {

    gst_init(NULL, NULL);
    init_sinks();
    set_loggers();
    set_global_level(spdlog::level::debug);
    auto main_log = spdlog::get("main");

    main_log->info("Starting...");
    int port_num = 8090;
    if (argc == 2)
        port_num = atoi(argv[1]);
    Port port(port_num);
    int threads = 1;
    Address addr(Ipv4::any(), port);
    main_log->info("Listening on 0.0.0.0:{} ",port);
    Endpoints api(addr);
    try {
        api.init(threads);
        std::thread api_thread = std::thread(&Endpoints::start, api);
        main_log->debug("Noted start of server");
        char s = 's';
        while(s != 'q') {
            std::cin >> s;
        }
        api.shutdown();
        main_log->info("called shutdown");
        api_thread.join();
    } catch(...) {
        main_log->warn("Catched an exception.");
        
    }
    return 0;
}
