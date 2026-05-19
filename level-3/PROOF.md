# ĐẶC TẢ VÀ CHỨNG MINH TOÁN HỌC (MỨC 3 - NHÁNH A)

Tệp này cung cấp đặc tả toán học chi tiết và chứng minh hình thức (Equational Reasoning) cho 2 pure functions cốt lõi của game Tic-Tac-Toe / Gomoku.

---

## 1. HÀM `isValidMove(board, move)`

### 1.1. Đặc tả toán học
Cho bàn cờ $B = (G, N)$, trong đó $G$ là ma trận hai chiều kích thước $N \times N$, và nước đi $m = (r, c)$.
Hàm $isValidMove(B, m)$ được định nghĩa như sau:
$$isValidMove(B, m) \iff 0 \le r < N \land 0 \le c < N \land G[r][c] = \text{EMPTY\_CELL}$$

Hàm chuyển đổi trạng thái $applyMove(B, m, S)$ với ký hiệu $S$ được định nghĩa:
$$applyMove(B, m, S) = \begin{cases} B' & \text{nếu } isValidMove(B, m) \\ B & \text{ngược lại} \end{cases}$$
Trong đó $B' = (G', N)$ là bàn cờ mới thỏa mãn:
- $G'[r][c] = S$
- $G'[i][j] = G[i][j] \quad \forall (i, j) \ne (r, c)$

### 1.2. Đặc tính 1 (Bất biến khi nước đi không hợp lệ)
Với mọi bàn cờ $B$, nước đi $m$, và ký hiệu $S$:
$$isValidMove(B, m) = \text{false} \implies applyMove(B, m, S) = B$$

### 1.3. Chứng minh hình thức (Equational Reasoning)
Giả sử $isValidMove(B, m) = \text{false}$.
Theo định nghĩa của hàm chuyển đổi trạng thái $applyMove(B, m, S)$:
$$applyMove(B, m, S) = \begin{cases} B' & \text{nếu } isValidMove(B, m) \\ B & \text{ngược lại} \end{cases}$$
Vì điều kiện $isValidMove(B, m)$ mang giá trị `false`, biểu thức sẽ rơi vào nhánh "ngược lại", trả về giá trị là $B$ nguyên bản.
Do đó:
$$applyMove(B, m, S) = B \quad \text{(đpcm)}$$

---

## 2. HÀM `countSymbol(board, symbol)`

### 2.1. Đặc tả toán học
Hàm $countSymbol(B, S)$ đếm số lượng ô có ký hiệu $S$ trên bàn cờ $B = (G, N)$:
$$countSymbol(B, S) = \sum_{x=0}^{N-1} \sum_{y=0}^{N-1} \mathbb{I}(G[x][y] = S)$$
Trong đó $\mathbb{I}(P)$ là hàm chỉ thị (indicator function), có giá trị bằng 1 nếu mệnh đề $P$ đúng, và bằng 0 nếu ngược lại.

### 2.2. Đặc tính 2 (Tính cộng dồn khi đánh nước hợp lệ)
Với mọi bàn cờ $B$, nước đi $m = (r, c)$, và ký hiệu $S \ne \text{EMPTY\_CELL}$:
$$isValidMove(B, m) = \text{true} \implies countSymbol(applyMove(B, m, S), S) = countSymbol(B, S) + 1$$

### 2.3. Chứng minh hình thức (Equational Reasoning)
Giả sử $isValidMove(B, m) = \text{true}$.
Đặt $B_{new} = applyMove(B, m, S) = (G_{new}, N)$.
Vì $isValidMove(B, m) = \text{true}$, theo định nghĩa của hàm $applyMove$, ta thu được bàn cờ mới có:
1. $G_{new}[r][c] = S$
2. $G_{new}[i][j] = G[i][j] \quad \forall (i, j) \ne (r, c)$

Áp dụng định nghĩa của $countSymbol$ cho $B_{new}$:
$$countSymbol(B_{new}, S) = \sum_{x=0}^{N-1} \sum_{y=0}^{N-1} \mathbb{I}(G_{new}[x][y] = S)$$

Tách tổng trên thành giá trị tại ô $(r, c)$ và các ô còn lại:
$$countSymbol(B_{new}, S) = \mathbb{I}(G_{new}[r][c] = S) + \sum_{(x, y) \ne (r, c)} \mathbb{I}(G_{new}[x][y] = S)$$

Thế các giá trị của bàn cờ $G_{new}$ vào:
- Vì $G_{new}[r][c] = S$, ta có $\mathbb{I}(G_{new}[r][c] = S) = \mathbb{I}(S = S) = 1$.
- Với mọi $(x, y) \ne (r, c)$, ta có $G_{new}[x][y] = G[x][y]$.

Khi đó biểu thức trở thành:
$$countSymbol(B_{new}, S) = 1 + \sum_{(x, y) \ne (r, c)} \mathbb{I}(G[x][y] = S) \quad (1)$$

Bây giờ xét hàm $countSymbol$ trên bàn cờ gốc $B$, tách tổng tương tự:
$$countSymbol(B, S) = \mathbb{I}(G[r][c] = S) + \sum_{(x, y) \ne (r, c)} \mathbb{I}(G[x][y] = S)$$

Từ giả thiết $isValidMove(B, m) = \text{true}$, suy ra $G[r][c] = \text{EMPTY\_CELL}$.
Vì ký hiệu quân đi $S \ne \text{EMPTY\_CELL}$, ta có $G[r][c] \ne S$, dẫn tới $\mathbb{I}(G[r][c] = S) = 0$.
Thế vào biểu thức của $countSymbol(B, S)$:
$$countSymbol(B, S) = 0 + \sum_{(x, y) \ne (r, c)} \mathbb{I}(G[x][y] = S) = \sum_{(x, y) \ne (r, c)} \mathbb{I}(G[x][y] = S) \quad (2)$$

Từ $(1)$ và $(2)$, bằng phép thế trực tiếp, ta suy ra:
$$countSymbol(B_{new}, S) = 1 + countSymbol(B, S)$$
Hay:
$$countSymbol(applyMove(B, m, S), S) = countSymbol(B, S) + 1 \quad \text{(đpcm)}$$
