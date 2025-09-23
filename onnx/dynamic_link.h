#ifndef DYNLINKLOADER_H
#define DYNLINKLOADER_H

#include <string>

namespace PluginUtils {

    std::string makePlatformLibName(const std::string& baseName, const std::string& ext, const std::string& prefix);

    std::string getPluginDirectory();

    void* loadDynamicLibrary(const std::string& libName);

    void unloadDynamicLibrary(void* handle);

    void* getSymbol(void* handle, const char* symbolName);

}

#endif