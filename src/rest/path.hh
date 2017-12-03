#ifndef PATH_HH
#define PATH_HH
#include "spdlog/spdlog.h"
#include <vector>
#include <string>

class PathDoc {
public:
    PathDoc();
    PathDoc(const PathDoc&) = default;
    PathDoc(PathDoc&&) = default;
    PathDoc& operator=(const PathDoc&) = default;
    PathDoc& operator=(PathDoc&&) = default;
    virtual ~PathDoc() = default;

    std::vector<std::string> get_paths() const;
    void add_entry(const std::string& pathname, struct dirent* entry, std::vector<std::string>& extension,int depth);
    void get_default_extensions(std::vector<std::string>& extension);
    void create_list(const std::string& pathname, std::vector<std::string>& extensions, int max_depth);
private:
    std::vector<std::string> _path_list;
    std::shared_ptr<spdlog::logger> _log;
    bool is_valid_dir(const std::string& pathname);
};
#endif // PATH_HH
