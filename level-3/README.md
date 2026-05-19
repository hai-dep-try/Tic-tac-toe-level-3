# Level 3 — Functional Programming

Bài tập lớn level 3 môn **Phương pháp luận lập trình**.
Nội dung chi tiết: xem `project-lv3.pdf`.

## Build nhanh (không SDL)

```bash
mkdir build && cd build
cmake -DUSE_SDL=OFF ..
cmake --build .
```

## Build có SDL (nếu giữ GUI từ level 2)

```bash
mkdir build && cd build
cmake -DUSE_SDL=ON ..
cmake --build .
```

## Build trực tiếp bằng g++ (Linux/macOS, không cmake)

```bash
g++ -std=c++20 -Wall -Wextra -Isrc -pthread \
    src/main.cpp src/utils/config.cpp \
    src/shell/logger.cpp src/shell/engine.cpp \
    src/interface/i_renderer.cpp src/interface/i_interaction.cpp \
    src/terminal/renderer.cpp src/terminal/interaction.cpp \
    src/core/logic.cpp src/core/bot_pure.cpp src/core/parallel.cpp \
    -o game
```

## Chạy

```bash
./game --help              # in help
./game                     # interactive (terminal)
./game -j -i input.txt     # judge mode (output cho grader)
./game -p                  # bật parallel bot HARD (mức 3 nhánh B)
./game -s 12345            # đặt seed RNG
./game -v                  # log chi tiết DEBUG
```

## Demo Y combinator (tham khảo cho mức 3 nhánh A)

```bash
cd build && cmake --build . --target y_demo && ./y_demo
```

## Cấu trúc thư mục

```
src/
├── main.cpp                       composition root
├── core/                          PURE — không IO/Logger/RNG global
│   ├── types.h
│   ├── pipeline.h                 (cung cấp sẵn — pipe/compose/curry)
│   ├── logic.{h,cpp}              MỨC 1 task 1
│   ├── bot_pure.{h,cpp}           MỨC 1 task 2 + MỨC 2
│   ├── parallel.{h,cpp}           MỨC 3 nhánh B
│   └── recursion_combinator.cpp   MỨC 3 nhánh A (demo)
├── shell/                         IMPERATIVE SHELL — side-effect cô lập
│   ├── logger.{h,cpp}             MỨC 1 task 3 (DI)
│   ├── rng.h                      MỨC 1 task 2&3 (DI)
│   └── engine.{h,cpp}             MỨC 2
├── interface/, terminal/, sdl/    IO layer
└── utils/
```

## Grader

Tái dùng grader của level 1/2 — `python3 grader.py` (test trên `../testcase/`).
