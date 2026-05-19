/**
 * Pipeline / FP combinator helpers (header-only)
 *
 * Mô tả:
 *   Tập hợp các "công cụ" lập trình hàm dùng chung cho tầng `core/`:
 *     - pipe(f, g, h, ...)    : ráp các hàm thành pipeline trái -> phải.
 *     - compose(f, g, h, ...) : tổ hợp hàm phải -> trái (toán học: f o g).
 *     - curry(f)              : currying một hàm 2 tham số.
 *     - filter(pred)          : HOF lọc theo predicate (filter)
 *     - map(f)                : HOF biến đổi từng phần tử (map)
 *     - reduce(init, op)      : HOF tích luỹ theo phép op (reduce/fold)
 *     - identity              : lambda x. x
 *     - constant(v)           : lambda _. v
 *
 * File này CÓ SẴN -- các bạn không phải tự cài, nhưng nên ĐỌC HIỂU để
 * dùng được cho mức 2 (composition / currying) và mức 1 (filter -> map
 * -> reduce trong Bot Easy / enumerateValidMoves / countSymbol).
 *
 * Triết lý:
 *   - Tất cả đều là HOF (higher-order function): nhận hàm và trả về hàm.
 *   - Không phụ thuộc state, không IO -- pure thuần tuý.
 *   - Container-agnostic: filter/map/reduce dùng cho mọi container có
 *     `value_type`, `begin()`, `end()` (vector, list, ...).
 */

#pragma once

/* ---------- Importing ---------- */

#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace fp {

/* ---------- compose: f o g (right-to-left) ---------- */

/**
 * Mô tả: compose(f) = f.
 */
template <typename F>
auto compose(F&& f) {
    return std::forward<F>(f);
}

/**
 * Mô tả:
 *   compose(f, g)         = lambda x. f(g(x))
 *   compose(f, g, h, ...) = f o g o h o ...
 *   Áp dụng PHẢI sang TRÁI (đúng theo ký pháp toán học).
 */
template <typename F, typename G, typename... Rest>
auto compose(F&& f, G&& g, Rest&&... rest) {
    auto inner = compose(std::forward<G>(g), std::forward<Rest>(rest)...);
    return [f = std::forward<F>(f), inner = std::move(inner)](auto&&... xs) {
        return f(inner(std::forward<decltype(xs)>(xs)...));
    };
}

/* ---------- pipe: g o f (left-to-right) ---------- */

/**
 * Mô tả: pipe(f) = f.
 */
template <typename F>
auto pipe(F&& f) {
    return std::forward<F>(f);
}

/**
 * Mô tả:
 *   pipe(f, g)         = lambda x. g(f(x))
 *   pipe(f, g, h, ...) đọc từ TRÁI sang PHẢI -- giống "data -> f -> g -> h".
 *
 * Ví dụ:
 *   auto best = pipe(
 *       enumerateCells,                                          // Board -> vector<Move>
 *       filter([&](Move m){ return isValidMove(board, m); }),    // -> vector<Move>
 *       map([&](Move m){ return std::pair{m, score(board, m)}; }),// -> vector<pair<Move,int>>
 *       reduce(std::pair{INVALID_MOVE, INT_MIN}, takeMax)        // -> pair<Move,int>
 *   );
 *   auto [bestMove, bestScore] = best(board);
 */
template <typename F, typename G, typename... Rest>
auto pipe(F&& f, G&& g, Rest&&... rest) {
    auto tail = pipe(std::forward<G>(g), std::forward<Rest>(rest)...);
    return [f = std::forward<F>(f), tail = std::move(tail)](auto&&... xs) {
        return tail(f(std::forward<decltype(xs)>(xs)...));
    };
}

/* ---------- curry: 2-arg -> chuỗi 1-arg ---------- */

/**
 * Mô tả:
 *   Currying một hàm 2 tham số: curry(f)(x)(y) tương đương f(x, y).
 *   Phù hợp cho partial application, ví dụ:
 *       auto evalFor  = curry(evaluate);   // (Board, Symbol) -> int
 *       auto evalForX = evalFor('X');      // Board -> int (cố định symbol)
 *
 * NOTE:
 *   Phiên bản này chỉ làm cho 2-arg cho đơn giản và đủ dùng.
 *   Các bạn muốn nâng cao có thể tự mở rộng cho n-arg.
 */
template <typename F>
auto curry(F f) {
    return [f = std::move(f)](auto x) {
        return [f, x = std::move(x)](auto y) {
            return f(x, y);
        };
    };
}

/* ---------- filter / map / reduce ---------- */

/**
 * Mô tả:
 *   filter(pred)(xs) -> vector chứa các phần tử x trong xs thoả pred(x).
 *
 *   Khác với std::ranges::views::filter (trả về view lazy), bản này trả về
 *   vector cụ thể -- để dùng dễ dàng trong fp::pipe(...). Cùng signature
 *   với map / reduce: HOF nhận lambda, trả về một hàm 1-arg nhận container.
 *
 * Ví dụ:
 *   auto validMoves = fp::pipe(
 *       enumerateCells,
 *       fp::filter([&](Move m){ return isValidMove(board, m); })
 *   )(board);
 */
template <typename Pred>
auto filter(Pred pred) {
    return [pred = std::move(pred)](const auto& xs) {
        using T = typename std::decay_t<decltype(xs)>::value_type;
        std::vector<T> out;
        for (const auto& x : xs) {
            if (pred(x)) out.push_back(x);
        }
        return out;
    };
}

/**
 * Mô tả:
 *   map(f)(xs) -> vector chứa f(x) cho từng x trong xs.
 *
 *   Kiểu trả về tự suy ra qua std::invoke_result_t<F, T>.
 *
 * Ví dụ:
 *   auto scored = fp::pipe(
 *       enumerateValidMoves,
 *       fp::map([&](Move m){ return std::pair{m, evaluate(board, m)}; })
 *   )(board);  // -> vector<pair<Move, int>>
 */
template <typename F>
auto map(F f) {
    return [f = std::move(f)](const auto& xs) {
        using T = typename std::decay_t<decltype(xs)>::value_type;
        using R = std::invoke_result_t<F, const T&>;
        std::vector<R> out;
        out.reserve(xs.size());
        for (const auto& x : xs) out.push_back(f(x));
        return out;
    };
}

/**
 * Mô tả:
 *   reduce(init, op)(xs) -> tích luỹ acc = op(acc, x) qua từng x, bắt đầu
 *   từ acc = init. Tương đương `std::accumulate` nhưng dạng HOF/curried.
 *
 *   Cũng gọi là `fold` trong nhiều ngôn ngữ FP.
 *
 * Ví dụ:
 *   int total = fp::pipe(
 *       enumerateCells,
 *       fp::map([&](Move m){ return board.at(m.row, m.col) == sym ? 1 : 0; }),
 *       fp::reduce(0, std::plus<int>{})
 *   )(board);
 */
template <typename Init, typename Op>
auto reduce(Init init, Op op) {
    return [init = std::move(init), op = std::move(op)](const auto& xs) {
        auto acc = init;
        for (const auto& x : xs) acc = op(acc, x);
        return acc;
    };
}

/* ---------- identity & constant ---------- */

/**
 * Mô tả: lambda x. x -- hàm đồng nhất (rất hữu ích trong pipeline).
 */
inline constexpr auto identity = [](auto&& x) -> decltype(auto) {
    return std::forward<decltype(x)>(x);
};

/**
 * Mô tả: constant(v) = lambda _. v -- luôn trả về v bất kể đầu vào.
 */
template <typename T>
auto constant(T v) {
    return [v = std::move(v)](auto&&...) { return v; };
}

}  // namespace fp
