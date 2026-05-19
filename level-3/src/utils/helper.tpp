/**
 * Helper tpp template implementation
 */

#include "helper.h"

/* ---------- Importing ---------- */

#include <chrono>
#include <sstream>

/* ---------- Definitions ---------- */

template <typename Function>
auto measureExecutionTime(const std::string& label,
                          Function func,
                          bool enabled,
                          const LogSink& sink) -> std::invoke_result_t<Function> {
    using ReturnT = std::invoke_result_t<Function>;

    auto start = std::chrono::high_resolution_clock::now();

    if constexpr (std::is_void_v<ReturnT>) {
        func();
        auto end = std::chrono::high_resolution_clock::now();

        if (enabled && sink) {
            std::chrono::duration<double> dur = end - start;
            std::stringstream msg;
            msg << "execution time of [" << label << "()] = " << dur.count() << "s";
            sink(msg.str());
        }
        return;

    } else {
        ReturnT result = func();
        auto end = std::chrono::high_resolution_clock::now();

        if (enabled && sink) {
            std::chrono::duration<double> dur = end - start;
            std::stringstream msg;
            msg << "execution time of [" << label << "()] = " << dur.count() << "s";
            sink(msg.str());
        }
        return result;
    }
}
