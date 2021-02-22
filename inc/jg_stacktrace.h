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


#endif

    return stack_frames;
}

} // namespace jg
