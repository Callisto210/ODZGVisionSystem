#include <pistache/net.h>
#include "rest/endpoints.hh"
#include <spdlog/spdlog.h>
#include <thread>
namespace spd = spdlog;
using namespace Pistache;


int main(void) {

    auto main_log = spdlog::stdout_color_mt("main");
    main_log->info("Starting...");
    Port port(8090);
    int threads = 1;
    Address addr(Ipv4::any(), port);
    main_log->info("Listening on 0.0.0.0:{} ",port);
    Endpoints api(addr);
    try {
        api.init(threads);
        std::thread api_thread = std::thread(&Endpoints::start, api);
        main_log->debug("Noted start of server");
        char s;
        std::cin>>s;
        api.shutdown();
        main_log->info("called shutdown");
        api_thread.join();
    } catch(...) {
        main_log->warn("Catched an exception.");
        
    }
    return 0;
}
