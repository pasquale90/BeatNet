#include "dynamic_link.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#endif

#include <string>
#include <filesystem>
#include <iostream>

namespace PluginUtils {

    std::string makePlatformLibName(const std::string& prefix, const std::string& baseName, const std::string& ext)
    {
        return prefix+baseName+ext;
    }

    std::string getPluginDirectory() {
        #if defined(_WIN32)
            HMODULE hModule = nullptr;
            if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                    reinterpret_cast<LPCSTR>(&getPluginDirectory),&hModule)) 
            {
                return "";
            }

            char path[MAX_PATH];
            GetModuleFileNameA(hModule, path, MAX_PATH);
            return std::filesystem::path(path).parent_path().string();

        #elif defined(__APPLE__)
            Dl_info info;
            if (dladdr((void*)&getPluginDirectory, &info)) 
            {
                return std::filesystem::path(info.dli_fname).parent_path().string();
            }
            return "";

        #elif defined(__linux__)
            Dl_info info;
            if (dladdr((void*)&getPluginDirectory, &info)) 
            {
                return std::filesystem::path(info.dli_fname).parent_path().string();
            }
            return "";

        #else
            return "";
        #endif
    }

    void* loadDynamicLibrary(const std::string& libName) 
    {
        std::string basePath = getPluginDirectory();
        if (basePath.empty()) {
            std::cerr << "Failed to get plugin directory.\n";
            return nullptr;
        }

        std::filesystem::path fullPath = std::filesystem::path(basePath) / libName;

        #if defined(_WIN32)
            HMODULE handle = LoadLibraryA(fullPath.string().c_str());
            if (!handle) {
                std::cerr << "Failed to load library: " << fullPath << "\n";
            }
            return reinterpret_cast<void*>(handle);

        #elif defined(__APPLE__) || defined(__linux__)
            void* handle = dlopen(fullPath.c_str(), RTLD_NOW);
            if (!handle) {
                std::cerr << "dlopen error: " << dlerror() << "\n";
            }
            return handle;

        #else
            return nullptr;
        #endif
    }

    void unloadDynamicLibrary(void* handle) 
    {
        if (!handle) return;

        #if defined(_WIN32)
            FreeLibrary(reinterpret_cast<HMODULE>(handle));
        #elif defined(__APPLE__) || defined(__linux__)
            dlclose(handle);
        #endif
    }

    void* getSymbol(void* handle, const char* symbolName) 
    {
        if (!handle || !symbolName) {
            return nullptr;
        }

        #if defined(_WIN32)
            return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle), symbolName));
        #elif defined(__APPLE__) || defined(__linux__)
            return dlsym(handle, symbolName);
        #else
            return nullptr;
        #endif
    }
}
