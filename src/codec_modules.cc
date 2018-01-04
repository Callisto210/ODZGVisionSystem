#include <pistache/net.h>
#include "rest/endpoints.hh"
#include <spdlog/spdlog.h>
#include <thread>
#include <vector>
#include <csignal>
#include <cstdlib>
#include "app_config.hh"
extern "C" {
#include <gst/gst.h>
}
#include "codecs/codec_module.hh"
namespace spd = spdlog;
using namespace Pistache;

static GMainLoop *loop;
static bool loop_bool = true;
void sigterm_handler(int signo)
{
    fprintf(stderr,"Signal handler is actively used.\n");
    if (signo == SIGINT)
        spd::get("main")->info("Handled SIGINT - closing server...");
         std::cin.putback('q');
         std::cin.sync();
         std::cin.setstate(std::ios_base::eofbit);
    if (signo == SIGTERM) {
        spd::get("main")->info("Catched SIGTERM - closing server...");
    }
    loop_bool = false;
}

int main(int argc, char** argv)
{
    gst_init(NULL, NULL);
    init_log_sinks();
    set_loggers();
    set_global_level(spdlog::level::debug);
    auto main_log = spdlog::get("main");

    main_log->info("Starting...");
    int port_num = 8090;
    if (argc == 2 && argv[1][0] != '0')
        port_num = std::atoi(argv[1]);
    Port port(static_cast<uint16_t>(port_num));
    int threads = 1;

    /*
     * Standard IPv4 localhost address
     */
    Address addr(Ipv4::any(), port);
    Endpoints api(addr);

    main_log->info("Listening on 0.0.0.0:{} ",port);
    try {
        if (signal(SIGINT, sigterm_handler) < 0)
            main_log->warn("Cannot set signal handler for SIGINT {}", std::strerror(errno));
        api.init(threads);
        std::thread api_thread = std::thread(&Endpoints::start, api);
        main_log->debug("Noted start of server");
        char s = 's';
        loop = g_main_loop_new(NULL, FALSE);
        std::thread backend = std::thread(g_main_loop_run, loop);
        if (signal(SIGINT, sigterm_handler) < 0){
            main_log->warn("Cannot set signal handler for SIGINT {}", strerror(errno));
        }
        while(s != 'q' && loop_bool) {
            s = 'q';
            std::cin >> s;
            if( std::cin.eof() || std::cin.fail()) {
                main_log->info("End of file or stdin failed");
                break;
            }
        }
        api.shutdown();
        main_log->info("called shutdown");
        if (g_main_loop_is_running(loop)) {
            g_main_loop_quit(loop);
        }
        if (backend.joinable()) {
            backend.join();
        }
        if (api_thread.joinable()) {
            api_thread.join();
        }
    } catch (std::runtime_error& e) {
        main_log->error(std::string("Error") + e.what());
    } catch(...) {
        main_log->warn("Catched an exception.");
        
    }
    main_log->debug("Unref main loop");
    g_main_loop_unref(loop);
    return 0;
}
