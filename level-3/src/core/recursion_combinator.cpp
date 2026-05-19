/**
 * Recursion combinator demo (Mức 3 -- nhánh A, KHUYẾN KHÍCH)
 *
 * Mô tả:
 *   File DEMO mô tả Tổ hợp Y (Y combinator) -- kỹ thuật cho phép định nghĩa
 *   hàm đệ quy trong λ-calculus mà không cần đặt tên hàm. Đây là minh chứng
 *   rằng FP thuần đủ mạnh để biểu diễn mọi tính toán (Turing-complete).
 *
 *   Sinh viên CHỌN nhánh A có thể tham khảo / mở rộng file này.
 *
 * NOTE:
 *   - File này KHÔNG được link vào executable game (chỉ là demo độc lập).
 *   - Có một hàm main_demo() in tử kết quả tính giai thừa qua Y.
 *   - Nếu muốn chạy: compile riêng:
 *         g++ -std=c++20 -DRUN_Y_DEMO recursion_combinator.cpp -o y_demo
 *
 * Tham khảo: slide FP_Advanced -- phần "Tổ hợp Y" và "Phép tính lambda".
 */

#include <functional>
#include <iostream>

namespace fp {

/**
 * Mô tả:
 *   Tổ hợp Y kiểu C++ (call-by-value):
 *       Y = λf.(λx. f(λv. x x v))(λx. f(λv. x x v))
 *
 *   C++ là call-by-value nên không dùng được Y "thuần" (sẽ vô hạn);
 *   ta dùng biến thể H (Turing fixed-point) -- chèn 1 lớp lambda để delay.
 *
 *   Cài đặt thực tế nhờ std::function (cho phép kiểu đệ quy):
 */
template <typename A, typename R>
auto Y(std::function<std::function<R(A)>(std::function<R(A)>)> f)
        -> std::function<R(A)> {
    return [f](A x) -> R {
        // Áp dụng f vào (Y f) -- đây chính là điểm cố định.
        return f(Y(f))(x);
    };
}

}  // namespace fp

#ifdef RUN_Y_DEMO

/**
 * Mô tả:
 *   Demo: định nghĩa giai thừa qua Y, không có tên `factorial` ở RHS.
 *
 *   F = λf. λn. if n == 0 then 1 else n * f(n-1)
 *   factorial = Y F
 */
int main() {
    using F = std::function<int(int)>;

    // F: hàm bậc cao "tạo ra" giai thừa từ chính nó.
    auto Fgen = [](F self) -> F {
        return [self](int n) -> int {
            return n == 0 ? 1 : n * self(n - 1);
        };
    };

    auto fact = fp::Y<int, int>(Fgen);

    for (int n = 0; n <= 6; ++n) {
        std::cout << "fact(" << n << ") = " << fact(n) << "\n";
    }
    return 0;
}

#endif
