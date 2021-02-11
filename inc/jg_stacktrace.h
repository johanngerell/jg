#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

#if defined(_WIN32)
    #include <mutex>
    #include <process.h>
    #include <windows.h>
    #include <dbghelp.h>
    #pragma comment(lib, "dbghelp.lib")
#else
    #include <regex>
    #include <memory>
    #include <execinfo.h>
    #include <unistd.h>
    #if defined(__APPLE__)
        #include <sys/syslimits.h>
    #elif defined(__linux__) 
        #include <linux/limits.h> // PATH_MAX
    #endif
#endif

namespace jg {

struct stack_frame final
{
    std::uint64_t address;
    std::uint64_t address_displacement;
    std::string package;
    std::string function;
    std::string file;
    std::size_t line;
    std::size_t line_displacement;
};

static inline std::ostream& operator<<(std::ostream& stream, const stack_frame& frame)
{
    stream << "\t"
           << frame.function
           << " [0x" << std::hex << frame.address << " + 0x" << frame.address_displacement << "]";
    
    if (!frame.file.empty())
        stream << " at " << frame.file << "(" << std::dec << frame.line << ")";

    return stream;
}

/// @example
///     if (!invariant)
///        for (const auto& stack_frame : jg::stack_trace().take(10).skip(1).capture())
///            std::cout << stack_frame << "\n";
class stack_trace final
{
public:
    stack_trace& skip(size_t count);
    stack_trace& take(size_t count);

    std::vector<stack_frame> capture() const;

private:
    size_t m_skip = 0;
    size_t m_take = 0;
};

inline stack_trace& stack_trace::skip(size_t count)
{
    m_skip = count;
    return *this;
}

inline stack_trace& stack_trace::take(size_t count)
{
    m_take = count;
    return *this;
}

inline std::vector<stack_frame> stack_trace::capture() const
{
    std::vector<stack_frame> stack_frames;
    
#ifdef _WIN32

    std::vector<void*> back_trace(m_take);

    if (const auto frame_count = CaptureStackBackTrace(static_cast<DWORD>(m_skip + 1),
                                                       static_cast<DWORD>(back_trace.size()),
                                                       back_trace.data(),
                                                       nullptr))
    {
        stack_frames.reserve(frame_count);
        back_trace.resize(frame_count);

        SYMBOL_INFO_PACKAGE sip{};
        sip.si.SizeOfStruct = sizeof(sip.si);
        sip.si.MaxNameLen   = sizeof(sip.name);
        
        // All Win32 Sym... DbgHlp functions are single threaded and need synchronization.
        static std::mutex dbghlp_mutex;
        std::lock_guard<std::mutex> dbghlp_lock{dbghlp_mutex};
        
        if (SymInitialize(GetCurrentProcess(), NULL, TRUE))
        {
            for (void* raw_address : back_trace)
            {
                const DWORD64 address = reinterpret_cast<DWORD64>(raw_address);
                DWORD64 symbol_displacement = 0;

                if (SymFromAddr(GetCurrentProcess(), address, &symbol_displacement, &sip.si))
                {
                    jg::stack_frame frame{};
                    frame.address              = sip.si.Address;
                    frame.address_displacement = symbol_displacement;
                    frame.package              = sip.name;
                    frame.function             = sip.si.Name;

                    DWORD line_displacement = 0;
                    IMAGEHLP_LINE64 line{};
                    line.SizeOfStruct = sizeof(line);
                    
                    if (SymGetLineFromAddr64(GetCurrentProcess(), address, &line_displacement, &line))
                    {
                        frame.file              = line.FileName;
                        frame.line              = line.LineNumber;
                        frame.line_displacement = line_displacement;
                    }

                    stack_frames.push_back(std::move(frame));
                }
            }

            SymCleanup(GetCurrentProcess());
        }
    }

#else

    auto getexepath = [] () -> std::string
    {
        char result[PATH_MAX];
        const ssize_t length = readlink("/proc/self/exe", result, PATH_MAX);
        return {result, length > 0 ? static_cast<size_t>(length) : 0};
    };

    struct pipe_deleter final
    {
        void operator()(FILE* stream) const
        {
            pclose(stream);
        }
    };

    using pipe_ptr = std::unique_ptr<FILE, pipe_deleter>;

    auto make_pipe = [] (const char* command, const char* type) -> pipe_ptr
    {
        return pipe_ptr{popen(command, type)};
    };

    auto sh = [&] (const std::string& cmd) -> std::string
    {
        char buffer[128];
        auto pipe = make_pipe(cmd.c_str(), "r");
        // if (!pipe)
        //     throw std::runtime_error("popen() failed!");
        
        std::string result;
        while (!feof(pipe.get()))
            if (fgets(buffer, 128, pipe.get()))
                result += buffer;

        return result;
    };

    void* bt[1024];
    int bt_size = backtrace(bt, 1024);
    char** bt_syms = backtrace_symbols(bt, bt_size);

    std::regex re("\\[(.+)\\]");
    auto exec_path = getexepath();

    for (int i = 1; i < bt_size; i++)
    {
        std::string sym = bt_syms[i];
        std::smatch ms;
        
        if (std::regex_search(sym, ms, re))
        {
            std::string addr = ms[1];
            std::string cmd = "addr2line -e " + exec_path + " -f -C " + addr;
            auto r = sh(cmd);
            std::regex re2("\\n$");
            auto r2 = std::regex_replace(r, re2, "");
            std::cout << r2 << std::endl;
        }
    }
    free(bt_syms);

#endif

    return stack_frames;
}

} // namespace jg
