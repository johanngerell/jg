#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace jg
{

struct stack_frame final
{
    std::uint64_t address;
    std::uint64_t address_displacement;
    std::string symbol_name;
    std::string file;
    std::size_t line;
    std::size_t line_displacement;
};

static inline std::ostream& operator<<(std::ostream& stream, const stack_frame& frame)
{
    stream << "\t"
           << frame.symbol_name
           << "[0x"
           << std::hex << frame.address
           << "+"
           << std::dec << frame.address_displacement
           << "] at "
           << frame.file
           << "("
           << frame.line
           << ")";

    return stream;
}

/// @example
///     #ifndef NDEBUG
///         if (broken_invariant)
///            for (const auto& stack_frame :
///                jg::stack_trace().include_frame_count(25).skip_frame_count(1).capture())
///                    std::cout << stack_frame << "\n";
///     #endif
class stack_trace final
{
public:
    stack_trace();
    ~stack_trace();

    stack_trace& skip_frame_count(size_t count);
    stack_trace& include_frame_count(size_t count);

    std::vector<stack_frame> capture() const;

private:
    void* m_process;
    size_t m_skip_frame_count;
    size_t m_include_frame_count;
};

#ifdef _MSC_VER

inline stack_trace::stack_trace()
    : m_process(GetCurrentProcess())
    , m_skip_frame_count()
    , m_include_frame_count()
{
    if (m_process)
        m_process = SymInitialize(m_process, NULL, TRUE) ? m_process : nullptr;
}

inline stack_trace::~stack_trace()
{
    if (m_process)
        SymCleanup(m_process);
}

inline stack_trace& stack_trace::skip_frame_count(size_t count)
{
    m_skip_frame_count = count;
    return *this;
}

inline stack_trace& stack_trace::include_frame_count(size_t count)
{
    m_include_frame_count = count;
    return *this;
}

inline std::vector<stack_frame> stack_trace::capture() const
{
    if (!m_process)
        return {};

    std::vector<stack_frame> symbols;
    std::vector<void*> stack(m_include_frame_count);

    if (const auto frame_count = CaptureStackBackTrace(static_cast<DWORD>(m_skip_frame_count),
                                                       static_cast<DWORD>(stack.size()),
                                                       stack.data(),
                                                       nullptr))
    {
        symbols.reserve(frame_count);
        stack.resize(frame_count);

        SYMBOL_INFO_PACKAGE sip{};
        sip.si.SizeOfStruct = sizeof(sip.si);
        sip.si.MaxNameLen   = sizeof(sip.name);
        
        for (void* frame : stack)
        {
            DWORD64 address = reinterpret_cast<DWORD64>(frame);
            DWORD64 symbol_displacement = 0;

            if (SymFromAddr(m_process, address, &symbol_displacement, &sip.si))
            {
                IMAGEHLP_LINE64 line{};
                line.SizeOfStruct = sizeof(line);
                line.FileName = "";
                line.LineNumber = 0;
                DWORD line_displacement = 0;
                
                SymGetLineFromAddr64(m_process, address, &line_displacement, &line);
                symbols.push_back({sip.si.Address, symbol_displacement, sip.si.Name,
                                line.FileName, line.LineNumber, line_displacement});
            }
        }
    }

    return symbols;
}

#else

inline stack_trace::stack_trace()
{
}

inline stack_trace::~stack_trace()
{
}

inline stack_trace& stack_trace::skip_frame_count(size_t count)
{
    return *this;
}

inline stack_trace& stack_trace::include_frame_count(size_t count)
{
    return *this;
}

inline std::vector<stack_frame> stack_trace::capture() const
{
    return {};
}

#endif

} // namespace jg