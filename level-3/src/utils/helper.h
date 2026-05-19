/**
 * Helper header file
 *
 * Mô tả:
 *   Hàm template đo thời gian -- KHÁC level 2 ở chỗ:
 *   không phụ thuộc Logger global, mà nhận một callback `on_log`
 *   (ví dụ `[&](auto msg){ logger.log(msg, Logger::Level::DEBUG); }`).
 *
 * Lý do thay đổi:
 *   - Helper đo thời gian là hàm tổng quát, dùng được ở nhiều ngữ cảnh.
 *   - Tách phần ghi log ra ngoài (qua callback) → helper trở thành
 *     "pure đến mức có thể" -- chỉ side-effect đo thời gian.
 *   - Đây cũng là một ví dụ HOF (higher-order function) trong code base.
 */

#pragma once

/* ---------- Importing ---------- */

#include <functional>
#include <string>
#include <type_traits>

/* ---------- Type Definitions ---------- */

// Callback ghi log -- sinh viên có thể truyền `[](auto){}` để tắt log.
using LogSink = std::function<void(const std::string&)>;

/* ---------- Declarations ---------- */

/**
 * Mô tả:
 *   Đo thời gian thực thi của một hàm.
 *
 * Đầu vào:
 *   - label  : tên đoạn code (cho log)
 *   - func   : hàm/lambda cần đo
 *   - enabled: nếu false thì gọi func nhưng không đo/log
 *   - sink   : callback nhận chuỗi log (DI thay cho Logger global)
 *
 * Đầu ra: kiểu trả về của func (hoặc void).
 */
template <typename Function>
auto measureExecutionTime(const std::string& label,
                          Function func,
                          bool enabled,
                          const LogSink& sink) -> std::invoke_result_t<Function>;

#include "helper.tpp"
