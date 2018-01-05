#define _XOPEN_SOURCE 700
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <cstdlib>
#include <cstdio>
#include <climits>

#include "path.hh"


using std::string;
using std::vector;

static const unsigned int max_path = 4096;


static int errno_val = 0;

vector<string> PathDoc::get_paths() const
{
    return this->_path_list;
}

PathDoc::PathDoc() {
    this->_log = spdlog::get("rest");
    this->_path_list = vector<string>();
}

bool PathDoc::is_valid_dir(const string& pathname) {
    struct stat stats;
    if (stat(pathname.c_str(), &stats) == -1) {
        errno_val = errno;
        if (errno_val == EACCES) {
            this->_log->info("Searching in {}: {}.", pathname, ::strerror(errno_val));
        } else {
            _log->warn("Path: {} ({})", pathname, ::strerror(errno_val));
        }
        return false;
    }
    if (!S_ISDIR(stats.st_mode)) {
        return false;
    }
    return true;
}

void PathDoc::create_list(const string& pathname,
                          vector<string>& extensions,
                          int max_depth)
{
    if (max_depth == 0) return;
    if (_log == nullptr) {
        _log = spdlog::get("rest");
    }
    _log->debug("Create list: path {} , depth {}", pathname, max_depth);
    if (!is_valid_dir(pathname))
        return;
    if(extensions.empty()) {
        get_default_extensions(extensions);
       _log->warn("Empty extension list. Create default one.");
    } else {
        for(string& ext: extensions) {
            _log->debug("Extension in list: " + ext);
        }
    }
    struct dirent* entry;
    char* buffer;
    buffer = new char[max_path];

    char* abs_path = realpath(pathname.c_str(), buffer);
    if (abs_path == NULL) {
        _log->warn("Cannot read real path from {}", pathname);
        delete[] buffer;
        return;
    }

    DIR* dir_entry = opendir(abs_path);
    if (dir_entry == NULL) {
        _log->warn("Cannot open directory at {}", abs_path);
        delete[] buffer;
        return;
    }
    delete[] buffer;
    buffer = nullptr;
    entry = readdir(dir_entry);
    while(entry != NULL) {
        this->add_entry(pathname, entry, extensions, max_depth);
        errno = 0;
        entry = readdir(dir_entry);
    }
    if (errno != 0) {
        perror("readdir");
    }
    closedir(dir_entry);

}

void PathDoc::add_entry(const string& pathname,
                        struct dirent* entry,
                        vector<string>& extension,
                        int depth)
{
    // Validate entry parameter
    if (entry == NULL)
        return;
    if (pathname.size() == 0)
        return;

    string entry_name = entry->d_name;
    // Ignore '.', '..' special directories and hidden ones
    if(entry_name[0] == '.')
        return;

    string new_path = pathname + '/' + entry->d_name;
    if (is_valid_dir(new_path)) {
        create_list(new_path, extension, depth-1);
    } else {
        _log->debug("Path: {}", new_path);
        for(auto it = extension.begin(); it!= extension.end(); it++) {
            // Check if the last characters (extension) match searched filter
            string d_name = entry->d_name;
            size_t pos = d_name.find(it->c_str(),
                                     d_name.size() - it->size());
            if (pos != string::npos) {
                _path_list.push_back(new_path);
            }
        }
    }
}




void PathDoc::get_default_extensions(vector<string>& extension)
{
    extension.push_back(string("mp4"));
    extension.push_back(string("ogg"));
    extension.push_back(string("mp3"));
    extension.push_back(string("mkv"));
    extension.push_back(string("avi"));
}















