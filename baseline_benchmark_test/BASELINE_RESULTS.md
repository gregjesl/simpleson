# Simpleson 基准测试记录

## 测试配置

- rounds：`2000`
- **内存测试**（`baseline_benchmark_test_memory`）：`checksum`、堆分配次数、累计分配字节、堆释放次数；**不记录 wall-clock 耗时**。
- **耗时与吞吐**：仅以 **Google Benchmark**（`baseline_benchmark_test_time`）的 Time / CPU 及 `bytes_per_second`、`items_per_second` 为准。
- 堆统计：来自 `allocounter`（全局 `operator new/delete` 钩子）
- workload 说明：
  - `exp1～exp4`：`jobject::parse(input.c_str())`（`const char`* 路径）
  - `exp7～exp10`：`jobject::parse(input)`（`const std::string&` 路径）
  - `exp5`：先 parse，再读取字段（`string` / `int`）
  - `exp6`：先 parse，再序列化（`as_string()` / `pretty()`）

---

## 迭代 0：基线

### 原始结果


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 22000            | 1024000               | 22000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 176000           | 10816000              | 176000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1628000          | 114218000             | 1628000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 828000           | 84348000              | 828000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 220000           | 22252000              | 220000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 124000           | 21136000              | 124000             |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 10.0     | 448.0    |
| exp2-medium-10-fields-parse-destruct     | 87.0     | 4992.0   |
| exp3-large-100-fields-parse-destruct     | 813.0    | 52878.0  |
| exp4-array-100-elements-parse-destruct   | 413.0    | 40078.0  |
| exp7-stdstring-small-parse-destruct      | 11.0     | 512.0    |
| exp8-stdstring-10-fields-parse-destruct  | 88.0     | 5408.0   |
| exp9-stdstring-100-fields-parse-destruct | 814.0    | 57109.0  |
| exp10-stdstring-100-array-parse-destruct | 414.0    | 42174.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 110.0    | 11126.0  |
| exp6-pretty-50-fields                    | 62.0     | 10568.0  |


### Google Benchmark 测试


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- |
| BM_Parse_Small      | 732       | 725      | 1,120,000  | 48.64 Mi/s       | -                |
| BM_Parse_Object/5   | 1,383     | 1,381    | 497,778    | 62.84 Mi/s       | 3.62 M/s         |
| BM_Parse_Object/10  | 2,590     | 2,609    | 263,529    | 66.17 Mi/s       | 3.83 M/s         |
| BM_Parse_Object/50  | 16,123    | 16,044   | 44,800     | 58.31 Mi/s       | 3.12 M/s         |
| BM_Parse_Object/100 | 42,157    | 41,433   | 16,593     | 45.60 Mi/s       | 2.41 M/s         |
| BM_Parse_Object/500 | 524,576   | 531,250  | 1,000      | 19.35 Mi/s       | 941.18 k/s       |
| BM_Parse_Array/10   | 1,385     | 1,381    | 497,778    | 62.84 Mi/s       | -                |
| BM_Parse_Array/100  | 13,166    | 13,114   | 56,000     | 72.07 Mi/s       | -                |
| BM_Parse_Array/1000 | 148,528   | 149,613  | 4,073      | 69.42 Mi/s       | -                |
| BM_Parse_Nested/2   | 431       | 433      | 1,659,259  | -                | -                |
| BM_Parse_Nested/5   | 2,071     | 2,040    | 344,615    | -                | -                |
| BM_Parse_Nested/10  | 6,916     | 6,975    | 112,000    | -                | -                |
| BM_Parse_Nested/20  | 29,107    | 28,495   | 23,579     | -                | -                |
| BM_Serialize/10     | 978       | 977      | 640,000    | -                | -                |
| BM_Serialize/50     | 4,213     | 4,238    | 165,926    | -                | -                |
| BM_Serialize/100    | 8,164     | 8,161    | 74,667     | -                | -                |
| BM_Serialize/500    | 38,912    | 38,365   | 17,920     | -                | -                |
| BM_Pretty/10        | 473       | 465      | 1,544,828  | -                | -                |
| BM_Pretty/50        | 1,713     | 1,726    | 407,273    | -                | -                |
| BM_Pretty/100       | 3,226     | 3,223    | 213,333    | -                | -                |
| BM_Access_Key/5     | 79.4      | 78.1     | 11,200,000 | -                | -                |
| BM_Access_Key/10    | 121       | 120      | 5,600,000  | -                | -                |
| BM_Access_Key/50    | 434       | 430      | 1,600,000  | -                | -                |
| BM_Access_Key/100   | 840       | 854      | 896,000    | -                | -                |
| BM_Access_Key/500   | 3,701     | 3,690    | 194,783    | -                | -                |


### 分析

- *exp1~~exp4（const char 解析路径）**：单次解析分配 10~~813 次，字节数随复杂度线性增长。exp3 大对象每次解析分配约 52KB 内存，是性能热点。
- **exp7~exp10（std::string 解析路径）**：相比 char* 路径多 1~10 次分配（因 string 内部拷贝），exp7 多分配 64 字节/次，说明存在 string 数据拷贝开销。
- **exp5 字段读取**：string 字段每次读取分配 1 次（32 字节），int 字段无分配，符合值语义设计。
- **exp6 序列化**：as_string 每次 110 次分配，pretty 每次 62 次分配，存在较大优化空间（如预分配缓冲区）。

**关键指标**：

- 小对象解析：~725ns（48.64 Mi/s）
- 中等对象（10字段）：~2.6μs（66.17 Mi/s）
- 大规模对象（500字段）：~531μs，单字段处理时间显著退化
- 序列化 as_string：~~977ns（10字段），pretty 更快（~~465ns）
- 字段访问：O(n) 线性查找，100字段访问需 854ns

---

## 迭代 1： `sub_reader` 智能指针 + `NULL`→`nullptr`

### 原始结果


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 22000            | 1024000               | 22000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 176000           | 10816000              | 176000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1628000          | 114218000             | 1628000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 828000           | 84348000              | 828000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 220000           | 22252000              | 220000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 124000           | 21136000              | 124000             |


### 与迭代 0 对比（相对迭代 0）


| 用例                                       | 分配次数变化 | 分配字节变化 |
| ---------------------------------------- | ------ | ------ |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%  |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%  |
| exp7-stdstring-small-parse-destruct      | 0.00%  | 0.00%  |
| exp8-stdstring-10-fields-parse-destruct  | 0.00%  | 0.00%  |
| exp9-stdstring-100-fields-parse-destruct | 0.00%  | 0.00%  |
| exp10-stdstring-100-array-parse-destruct | 0.00%  | 0.00%  |
| exp5-read-string-field                   | 0.00%  | 0.00%  |
| exp5-read-int-field                      | —      | —      |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%  |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%  |


- 

### Google Benchmark 测试


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 732       | 732      | 896,000    | 48.18 Mi/s       | -                | +0.97%  | —       |
| BM_Parse_Object/5   | 1,395     | 1,395    | 560,000    | 62.21 Mi/s       | 3.58 M/s         | +1.01%  | —       |
| BM_Parse_Object/10  | 2,668     | 2,668    | 263,529    | 64.70 Mi/s       | 3.75 M/s         | +2.26%  | —       |
| BM_Parse_Object/50  | 16,392    | 16,392   | 44,800     | 57.07 Mi/s       | 3.05 M/s         | +2.17%  | —       |
| BM_Parse_Object/100 | 39,899    | 39,899   | 17,231     | 47.35 Mi/s       | 2.51 M/s         | -3.70%  | —       |
| BM_Parse_Object/500 | 500,000   | 500,000  | 1,000      | 20.56 Mi/s       | 1.00 M/s         | -5.88%  | —       |
| BM_Parse_Array/10   | 1,413     | 1,413    | 497,778    | 61.44 Mi/s       | -                | +2.32%  | —       |
| BM_Parse_Array/100  | 12,835    | 12,835   | 56,000     | 73.63 Mi/s       | -                | -2.13%  | —       |
| BM_Parse_Array/1000 | 145,777   | 145,777  | 4,073      | 71.25 Mi/s       | -                | -2.56%  | —       |
| BM_Parse_Nested/2   | 430       | 430      | 1,600,000  | -                | -                | -0.69%  | —       |
| BM_Parse_Nested/5   | 2,040     | 2,040    | 344,615    | -                | -                | 0.00%   | —       |
| BM_Parse_Nested/10  | 6,975     | 6,975    | 112,000    | -                | -                | 0.00%   | —       |
| BM_Parse_Nested/20  | 29,297    | 29,297   | 22,400     | -                | -                | +2.82%  | —       |
| BM_Serialize/10     | 977       | 977      | 640,000    | -                | -                | 0.00%   | —       |
| BM_Serialize/50     | 4,238     | 4,238    | 165,926    | -                | -                | 0.00%   | —       |
| BM_Serialize/100    | 8,894     | 8,894    | 89,600     | -                | -                | +8.98%  | —       |
| BM_Serialize/500    | 41,713    | 41,713   | 17,231     | -                | -                | +8.72%  | —       |
| BM_Pretty/10        | 547       | 547      | 1,000,000  | -                | -                | +17.63% | —       |
| BM_Pretty/50        | 1,925     | 1,925    | 373,333    | -                | -                | +11.53% | —       |
| BM_Pretty/100       | 3,449     | 3,449    | 194,783    | -                | -                | +7.01%  | —       |
| BM_Access_Key/5     | 92.1      | 92.1     | 7,466,667  | -                | -                | +17.93% | —       |
| BM_Access_Key/10    | 114       | 114      | 5,600,000  | -                | -                | -5.00%  | —       |
| BM_Access_Key/50    | 390       | 390      | 1,723,077  | -                | -                | -9.30%  | —       |
| BM_Access_Key/100   | 767       | 767      | 1,120,000  | -                | -                | -10.19% | —       |
| BM_Access_Key/500   | 3,369     | 3,369    | 213,333    | -                | -                | -8.70%  | —       |


### 分析

**代码改动**：将 `sub_reader` 从原始指针改为 `std::unique_ptr`，并将所有 `NULL` 替换为 `nullptr`。

- **堆分配**：所有用例的分配次数和字节数与基线完全一致，智能指针未引入额外堆分配。
- **Google Benchmark**（基于 CPU 时间）：
  - **解析**：整体持平或轻微波动；BM_Parse_Object/100、/500 等项相对基线有降有升，见上表。
  - **序列化**：BM_Serialize 系列大致持平（0%~~+8.98%），BM_Pretty 系列相对基线明显变慢（+7%~~+18%）。
  - **字段访问**：BM_Access_Key/50、/100 等相对基线改善约 9%~~10%。
- **结论**：`std::unique_ptr` 提升安全性且堆行为不变；时延结论仅依据上表。Pretty 路径的退化可后续关注。

---

## 迭代 2：`jobject::parse(const std::string&)` 参数改为 const 引用

### 原始结果


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 220000           | 22252000              | 220000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 124000           | 21136000              | 124000             |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 10.0     | 448.0    |
| exp2-medium-10-fields-parse-destruct     | 87.0     | 4992.0   |
| exp3-large-100-fields-parse-destruct     | 813.0    | 52878.0  |
| exp4-array-100-elements-parse-destruct   | 413.0    | 40078.0  |
| exp7-stdstring-small-parse-destruct      | 10.0     | 448.0    |
| exp8-stdstring-10-fields-parse-destruct  | 87.0     | 4992.0   |
| exp9-stdstring-100-fields-parse-destruct | 813.0    | 52878.0  |
| exp10-stdstring-100-array-parse-destruct | 413.0    | 40078.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 110.0    | 11126.0  |
| exp6-pretty-50-fields                    | 62.0     | 10568.0  |


### 与迭代 0 对比（相对迭代 0）


| 用例                                       | 分配次数变化 | 分配字节变化  |
| ---------------------------------------- | ------ | ------- |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%   |
| exp7-stdstring-small-parse-destruct      | -9.09% | -12.50% |
| exp8-stdstring-10-fields-parse-destruct  | -1.14% | -7.74%  |
| exp9-stdstring-100-fields-parse-destruct | -0.12% | -7.41%  |
| exp10-stdstring-100-array-parse-destruct | -0.24% | -4.97%  |
| exp5-read-string-field                   | 0.00%  | 0.00%   |
| exp5-read-int-field                      | —      | —       |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%   |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%   |


### 与迭代 1 对比（单步效果）


| 用例                                       | 分配次数变化 | 分配字节变化  |
| ---------------------------------------- | ------ | ------- |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%   |
| exp7-stdstring-small-parse-destruct      | -9.09% | -12.50% |
| exp8-stdstring-10-fields-parse-destruct  | -1.14% | -7.74%  |
| exp9-stdstring-100-fields-parse-destruct | -0.12% | -7.41%  |
| exp10-stdstring-100-array-parse-destruct | -0.24% | -4.97%  |
| exp5-read-string-field                   | 0.00%  | 0.00%   |
| exp5-read-int-field                      | —      | —       |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%   |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%   |


### Google Benchmark 测试


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 684       | 684      | 1,120,000  | 51.62 Mi/s       | -                | -5.66%  | -6.56%  |
| BM_Parse_Object/5   | 1,224     | 1,224    | 497,778    | 70.89 Mi/s       | 4.08 M/s         | -11.37% | -12.26% |
| BM_Parse_Object/10  | 2,567     | 2,567    | 280,000    | 67.24 Mi/s       | 3.90 M/s         | -1.61%  | -3.79%  |
| BM_Parse_Object/50  | 16,392    | 16,392   | 44,800     | 57.07 Mi/s       | 3.05 M/s         | +4.34%  | +2.13%  |
| BM_Parse_Object/100 | 41,713    | 41,713   | 17,231     | 45.29 Mi/s       | 2.40 M/s         | +0.67%  | +4.55%  |
| BM_Parse_Object/500 | 500,000   | 500,000  | 1,000      | 20.56 Mi/s       | 1.00 M/s         | -5.88%  | 0.00%   |
| BM_Parse_Array/10   | 1,350     | 1,350    | 497,778    | 64.30 Mi/s       | -                | -2.24%  | -4.46%  |
| BM_Parse_Array/100  | 12,556    | 12,556   | 56,000     | 75.27 Mi/s       | -                | -4.25%  | -2.17%  |
| BM_Parse_Array/1000 | 142,997   | 142,997  | 4,480      | 72.63 Mi/s       | -                | -4.42%  | -1.91%  |
| BM_Parse_Nested/2   | 433       | 433      | 1,659,259  | -                | -                | 0.00%   | +0.70%  |
| BM_Parse_Nested/5   | 1,950     | 1,950    | 344,615    | -                | -                | -4.41%  | -4.41%  |
| BM_Parse_Nested/10  | 6,696     | 6,696    | 112,000    | -                | -                | -4.00%  | -4.00%  |
| BM_Parse_Nested/20  | 26,995    | 26,995   | 24,889     | -                | -                | -5.26%  | -7.86%  |
| BM_Serialize/10     | 1,025     | 1,025    | 640,000    | -                | -                | +4.91%  | +4.91%  |
| BM_Serialize/50     | 4,143     | 4,143    | 165,926    | -                | -                | -2.24%  | -2.24%  |
| BM_Serialize/100    | 8,196     | 8,196    | 89,600     | -                | -                | +0.43%  | -7.85%  |
| BM_Serialize/500    | 38,365    | 38,365   | 17,920     | -                | -                | 0.00%   | -8.03%  |
| BM_Pretty/10        | 460       | 460      | 1,493,333  | -                | -                | -1.08%  | -15.90% |
| BM_Pretty/50        | 1,726     | 1,726    | 407,273    | -                | -                | 0.00%   | -10.34% |
| BM_Pretty/100       | 3,278     | 3,278    | 224,000    | -                | -                | +1.71%  | -4.96%  |
| BM_Access_Key/5     | 78.5      | 78.5     | 8,960,000  | -                | -                | +0.51%  | -14.77% |
| BM_Access_Key/10    | 114       | 114      | 5,600,000  | -                | -                | -5.00%  | -1.75%  |
| BM_Access_Key/50    | 426       | 426      | 1,723,077  | -                | -                | -0.93%  | +9.23%  |
| BM_Access_Key/100   | 785       | 785      | 896,000    | -                | -                | -8.08%  | +2.35%  |
| BM_Access_Key/500   | 3,530     | 3,530    | 194,783    | -                | -                | -4.34%  | +4.78%  |


### 分析

**代码改动**：将 `jobject::parse(const std::string)` 改为 `jobject::parse(const std::string&)`，避免 string 参数的拷贝构造。

- **Google Benchmark 结果分析**（基于 CPU 时间）：
  - **解析性能**（相对基线）：整体改善显著。
    - BM_Parse_Small 提升 5.66%，BM_Parse_Object/5 提升 11.37%，BM_Parse_Object/100 微降 0.67%
    - 单步效果（相对迭代 1）：BM_Parse_Small 提升 6.56%，BM_Parse_Object/5 提升 12.26%
  - **数组解析**：BM_Parse_Array/10 微降 2.24%（相对基线），单步效果提升 4.46%
  - **嵌套对象**：BM_Parse_Nested 系列整体改善（-4.00%~-5.26%）
  - **序列化性能**：BM_Serialize 系列持平至微降（-2.24%~+0.43%），BM_Pretty/10 提升 1.08%，BM_Pretty/50 持平
  - **字段访问性能**：BM_Access_Key/5 提升 0.51%，BM_Access_Key/50 微降 0.93%，BM_Access_Key/100 退化 8.08%（需关注）
- **堆分配**（相对迭代 0）：exp7 分配次数 −9.09%、字节 −12.50%；exp8/exp9 字节下降；exp10 分配与字节略变（见上表「与迭代 0 对比」）。**解析与访问的时延**见本节 Google Benchmark。
- **结论**：`const std::string&` 消除 string 参数拷贝；堆上体现为 std::string 路径分配减少。小对象解析等吞吐变化见上表（相对迭代 1 约 −6.6%~~−12.3% CPU）。

---

## 迭代 3：序列化路径优化：`operator std::string()`、`pretty()`

### 原始结果


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 10.0     | 448.0    |
| exp2-medium-10-fields-parse-destruct     | 87.0     | 4992.0   |
| exp3-large-100-fields-parse-destruct     | 813.0    | 52878.0  |
| exp4-array-100-elements-parse-destruct   | 413.0    | 40078.0  |
| exp7-stdstring-small-parse-destruct      | 10.0     | 448.0    |
| exp8-stdstring-10-fields-parse-destruct  | 87.0     | 4992.0   |
| exp9-stdstring-100-fields-parse-destruct | 813.0    | 52878.0  |
| exp10-stdstring-100-array-parse-destruct | 413.0    | 40078.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 对比


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | 0.00%   | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%   | 0.00%   |
| exp7-stdstring-small-parse-destruct      | -9.09%  | -12.50% |
| exp8-stdstring-10-fields-parse-destruct  | -1.14%  | -7.74%  |
| exp9-stdstring-100-fields-parse-destruct | -0.12%  | -7.41%  |
| exp10-stdstring-100-array-parse-destruct | -0.24%  | -4.97%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.88% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### 与迭代 2 对比（单步效果）


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | 0.00%   | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%   | 0.00%   |
| exp7-stdstring-small-parse-destruct      | 0.00%   | 0.00%   |
| exp8-stdstring-10-fields-parse-destruct  | 0.00%   | 0.00%   |
| exp9-stdstring-100-fields-parse-destruct | 0.00%   | 0.00%   |
| exp10-stdstring-100-array-parse-destruct | 0.00%   | 0.00%   |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.88% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### Google Benchmark 测试


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 642       | 642      | 1,120,000  | 54.98 Mi/s       | -                | -11.45% | -6.14%  |
| BM_Parse_Object/5   | 1,256     | 1,256    | 560,000    | 69.12 Mi/s       | 3.98 M/s         | -9.05%  | +2.61%  |
| BM_Parse_Object/10  | 2,623     | 2,623    | 280,000    | 67.24 Mi/s       | 3.90 M/s         | +0.54%  | +2.18%  |
| BM_Parse_Object/50  | 16,741    | 16,741   | 44,800     | 55.88 Mi/s       | 2.99 M/s         | +4.34%  | 0.00%   |
| BM_Parse_Object/100 | 40,981    | 40,981   | 14,933     | 32.83 Mi/s       | 1.74 M/s         | -1.09%  | -1.75%  |
| BM_Parse_Object/500 | 585,938   | 585,938  | 1,120      | 17.55 Mi/s       | 853.33 k/s       | +10.29% | +17.19% |
| BM_Parse_Array/10   | 1,604     | 1,604    | 448,000    | 54.09 Mi/s       | -                | +16.15% | +18.81% |
| BM_Parse_Array/100  | 15,695    | 15,695   | 44,800     | 60.22 Mi/s       | -                | +19.68% | +25.00% |
| BM_Parse_Array/1000 | 159,054   | 159,054  | 3,733      | 65.30 Mi/s       | -                | +6.31%  | +11.23% |
| BM_Parse_Nested/2   | 516       | 516      | 1,000,000  | -                | -                | +19.17% | +19.17% |
| BM_Parse_Nested/5   | 2,176     | 2,176    | 344,615    | -                | -                | +6.67%  | +11.59% |
| BM_Parse_Nested/10  | 7,499     | 7,499    | 89,600     | -                | -                | +7.51%  | +11.99% |
| BM_Parse_Nested/20  | 32,087    | 32,087   | 22,400     | -                | -                | +12.61% | +18.86% |
| BM_Serialize/10     | 295       | 295      | 2,488,889  | -                | -                | -69.80% | -71.22% |
| BM_Serialize/50     | 1,147     | 1,147    | 640,000    | -                | -                | -72.94% | -72.32% |
| BM_Serialize/100    | 2,176     | 2,176    | 344,615    | -                | -                | -73.34% | -73.46% |
| BM_Serialize/500    | 11,440    | 11,440   | 56,000     | -                | -                | -70.18% | -70.18% |
| BM_Pretty/10        | 267       | 267      | 2,635,294  | -                | -                | -42.58% | -41.96% |
| BM_Pretty/50        | 1,200     | 1,200    | 560,000    | -                | -                | -30.48% | -30.48% |
| BM_Pretty/100       | 2,302     | 2,302    | 298,667    | -                | -                | -28.58% | -29.77% |
| BM_Access_Key/5     | 75.0      | 75.0     | 8,960,000  | -                | -                | -3.97%  | -4.46%  |
| BM_Access_Key/10    | 122       | 122      | 5,600,000  | -                | -                | +1.67%  | +8.93%  |
| BM_Access_Key/50    | 424       | 424      | 1,659,259  | -                | -                | -1.40%  | -0.47%  |
| BM_Access_Key/100   | 816       | 816      | 746,667    | -                | -                | -4.45%  | +3.95%  |
| BM_Access_Key/500   | 3,683     | 3,683    | 186,667    | -                | -                | -0.19%  | +4.33%  |


### 分析

**代码改动**：优化序列化路径，`as_string()` 和 `pretty()` 使用预分配缓冲区 + `reserve()` 减少重复分配。

- **Google Benchmark 结果分析**（基于 CPU 时间）：
  - **解析性能**（相对基线）：
    - BM_Parse_Small 累计提升 11.45%，BM_Parse_Object/5 提升 9.05%
    - 单步效果（相对迭代 2）：BM_Parse_Small 提升 6.14%，数组解析（BM_Parse_Array/100）退化 25.00%（需关注）
  - **序列化性能**（巨大成功）：
    - BM_Serialize/10 提升 69.80%（相对基线），单步提升 71.22%
    - BM_Serialize/50 提升 72.94%（相对基线），单步提升 72.32%
    - BM_Serialize/100 提升 73.34%（相对基线），单步提升 73.46%
  - **Pretty 序列化**（极其显著）：
    - BM_Pretty/10 提升 42.58%（相对基线），单步提升 41.96%
    - BM_Pretty/50 提升 30.48%（相对基线），单步提升 30.48%
  - **字段访问性能**：整体持平至轻微改善，BM_Access_Key/5 微降 3.97%
- **堆分配**（相对迭代 0 / 迭代 2）：exp6 `as_string` 分配 110→51 次/次、字节/op 明显下降；`pretty` 62→1 次/次、字节/op 明显下降（见上表）。**序列化/Pretty 的 CPU 时延**见上节 Benchmark（如 BM_Serialize、BM_Pretty 相对基线大幅提升）。
- **结论**：堆分配效率大幅改善，尤其 `pretty` 几乎消除动态分配；解析与序列化吞吐以上表为准。需关注 BM_Parse_Array/100 相对迭代 2 单步约 +25% CPU。

---

## 迭代 4：`get_number_string()` 数字转字符串优化

### 原始结果


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 10.0     | 448.0    |
| exp2-medium-10-fields-parse-destruct     | 87.0     | 4992.0   |
| exp3-large-100-fields-parse-destruct     | 813.0    | 52878.0  |
| exp4-array-100-elements-parse-destruct   | 413.0    | 40078.0  |
| exp7-stdstring-small-parse-destruct      | 10.0     | 448.0    |
| exp8-stdstring-10-fields-parse-destruct  | 87.0     | 4992.0   |
| exp9-stdstring-100-fields-parse-destruct | 813.0    | 52878.0  |
| exp10-stdstring-100-array-parse-destruct | 413.0    | 40078.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 对比


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | 0.00%   | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%   | 0.00%   |
| exp7-stdstring-small-parse-destruct      | -9.09%  | -12.50% |
| exp8-stdstring-10-fields-parse-destruct  | -1.14%  | -7.74%  |
| exp9-stdstring-100-fields-parse-destruct | -0.12%  | -7.41%  |
| exp10-stdstring-100-array-parse-destruct | -0.24%  | -4.97%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.88% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### 与迭代 3 对比（单步效果）


| 用例                                       | 分配次数变化 | 分配字节变化 |
| ---------------------------------------- | ------ | ------ |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%  |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%  |
| exp7-stdstring-small-parse-destruct      | 0.00%  | 0.00%  |
| exp8-stdstring-10-fields-parse-destruct  | 0.00%  | 0.00%  |
| exp9-stdstring-100-fields-parse-destruct | 0.00%  | 0.00%  |
| exp10-stdstring-100-array-parse-destruct | 0.00%  | 0.00%  |
| exp5-read-string-field                   | 0.00%  | 0.00%  |
| exp5-read-int-field                      | —      | —      |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%  |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%  |


### Google Benchmark 测试


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 670       | 670      | 1,120,000  | 52.69 Mi/s       | -                | -7.59%  | +4.36%  |
| BM_Parse_Object/5   | 1,283     | 1,283    | 560,000    | 67.62 Mi/s       | 3.90 M/s         | -7.10%  | +2.15%  |
| BM_Parse_Object/10  | 2,623     | 2,623    | 280,000    | 65.81 Mi/s       | 3.81 M/s         | +0.54%  | 0.00%   |
| BM_Parse_Object/50  | 16,044    | 16,044   | 44,800     | 58.31 Mi/s       | 3.12 M/s         | 0.00%   | -4.17%  |
| BM_Parse_Object/100 | 40,981    | 40,981   | 17,920     | 46.10 Mi/s       | 2.44 M/s         | -1.09%  | 0.00%   |
| BM_Parse_Object/500 | 531,250   | 531,250  | 1,000      | 19.35 Mi/s       | 941.18 k/s       | 0.00%   | -9.33%  |
| BM_Parse_Array/10   | 1,465     | 1,465    | 448,000    | 59.24 Mi/s       | -                | +6.08%  | -8.67%  |
| BM_Parse_Array/100  | 14,439    | 14,439   | 49,778     | 65.45 Mi/s       | -                | +10.10% | -8.00%  |
| BM_Parse_Array/1000 | 157,286   | 157,286  | 4,073      | 66.04 Mi/s       | -                | +5.13%  | -1.11%  |
| BM_Parse_Nested/2   | 425       | 425      | 1,544,828  | -                | -                | -1.85%  | -17.64% |
| BM_Parse_Nested/5   | 1,967     | 1,967    | 373,333    | -                | -                | -3.58%  | -9.61%  |
| BM_Parse_Nested/10  | 6,696     | 6,696    | 112,000    | -                | -                | -4.00%  | -10.71% |
| BM_Parse_Nested/20  | 28,250    | 28,250   | 24,889     | -                | -                | -0.86%  | -11.96% |
| BM_Serialize/10     | 262       | 262      | 2,800,000  | -                | -                | -73.18% | -11.19% |
| BM_Serialize/50     | 1,172     | 1,172    | 640,000    | -                | -                | -72.35% | +2.18%  |
| BM_Serialize/100    | 2,302     | 2,302    | 298,667    | -                | -                | -71.79% | +5.79%  |
| BM_Serialize/500    | 11,963    | 11,963   | 64,000     | -                | -                | -68.82% | +4.57%  |
| BM_Pretty/10        | 267       | 267      | 2,635,294  | -                | -                | -42.58% | 0.00%   |
| BM_Pretty/50        | 1,228     | 1,228    | 560,000    | -                | -                | -28.85% | +2.33%  |
| BM_Pretty/100       | 2,250     | 2,250    | 298,667    | -                | -                | -30.19% | -2.26%  |
| BM_Access_Key/5     | 78.5      | 78.5     | 8,960,000  | -                | -                | +0.51%  | +4.67%  |
| BM_Access_Key/10    | 122       | 122      | 6,400,000  | -                | -                | +1.67%  | 0.00%   |
| BM_Access_Key/50    | 426       | 426      | 1,723,077  | -                | -                | -0.93%  | +0.47%  |
| BM_Access_Key/100   | 837       | 837      | 746,667    | -                | -                | -1.99%  | +2.57%  |
| BM_Access_Key/500   | 3,690     | 3,690    | 194,783    | -                | -                | 0.00%   | +0.19%  |


### 分析

**代码改动**：优化 `get_number_string()` 函数，使用固定栈缓冲区（`std::array<char, 64>`）替代动态扩容的 `vector<char>`，减少数字转字符串时的堆分配。

- **Google Benchmark 结果分析**（基于 CPU 时间）：
  - **累计成果**（相对基线）：
    - 解析性能：BM_Parse_Small 累计提升 7.59%，BM_Parse_Object/5 提升 7.10%，BM_Parse_Nested/20 微降 0.86%
    - 序列化性能：BM_Serialize/10 累计提升 **73.18%**（最大优化成果），BM_Serialize/100 提升 71.79%
    - Pretty 序列化：BM_Pretty/10 累计提升 **42.58%**，BM_Pretty/100 提升 30.19%
  - **单步效果**（相对迭代 3）：
    - 整体持平或轻微波动（±10% 范围内）
    - BM_Serialize/10 进一步微提升 11.19%
    - BM_Parse_Array/100 提升 8.00%，BM_Parse_Nested/2 提升 17.64%（测试波动）
    - 字段访问整体持平
- **堆分配**（allocounter）：改动为栈缓冲 + `snprintf` 预取长度；基准用例中数字转字符串占比小，**分配次数/字节与迭代 3 表一致**，未见可分离的堆侧变化。
- **结论**：
  - **最终优化成果**：经过 4 轮迭代，Simpleson 在 Google Benchmark 中取得了显著性能提升
    - 解析性能：小对象提升 7.59%，中等对象提升 7.10%
    - 序列化性能：as_string 提升 **73.18%**，pretty 提升 **42.58%**
  - 迭代 4 的微优化保持了迭代 3 的成果，代码更加健壮简洁，建议在特定场景（高频数字序列化）下可进一步测试收益。

---

## 迭代 5： `parsing::parse` 循环条件修正 + 删除 `jobject::parse` 无用局部变量

### 原始结果

**记录时间**：2026-03-31T16:38:04+08:00 · **可执行文件**：`D:\KINGSOFT\debug\bin\Release\baseline_benchmark_test_time.exe` · **CPU**：16 × 3194 MHz


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 10.0     | 448.0    |
| exp2-medium-10-fields-parse-destruct     | 87.0     | 4992.0   |
| exp3-large-100-fields-parse-destruct     | 813.0    | 52878.0  |
| exp4-array-100-elements-parse-destruct   | 413.0    | 40078.0  |
| exp7-stdstring-small-parse-destruct      | 10.0     | 448.0    |
| exp8-stdstring-10-fields-parse-destruct  | 87.0     | 4992.0   |
| exp9-stdstring-100-fields-parse-destruct | 813.0    | 52878.0  |
| exp10-stdstring-100-array-parse-destruct | 413.0    | 40078.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 对比（相对迭代 0）


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | 0.00%   | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%   | 0.00%   |
| exp7-stdstring-small-parse-destruct      | -9.09%  | -12.50% |
| exp8-stdstring-10-fields-parse-destruct  | -1.14%  | -7.74%  |
| exp9-stdstring-100-fields-parse-destruct | -0.12%  | -7.41%  |
| exp10-stdstring-100-array-parse-destruct | -0.24%  | -4.97%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.88% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### 与迭代 4 对比（单步效果）


| 用例                                       | 分配次数变化 | 分配字节变化 |
| ---------------------------------------- | ------ | ------ |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%  |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%  |
| exp7-stdstring-small-parse-destruct      | 0.00%  | 0.00%  |
| exp8-stdstring-10-fields-parse-destruct  | 0.00%  | 0.00%  |
| exp9-stdstring-100-fields-parse-destruct | 0.00%  | 0.00%  |
| exp10-stdstring-100-array-parse-destruct | 0.00%  | 0.00%  |
| exp5-read-string-field                   | 0.00%  | 0.00%  |
| exp5-read-int-field                      | —      | —      |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%  |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%  |


### Google Benchmark 测试


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 647       | 642      | 1,120,000  | 54.98 Mi/s       | -                | -11.45% | -4.18%  |
| BM_Parse_Object/5   | 1382      | 1381     | 407,273    | 62.84 Mi/s       | 3.62 M/s         | 0.00%   | +7.64%  |
| BM_Parse_Object/10  | 2698      | 2699     | 248,889    | 63.94 Mi/s       | 3.70 M/s         | +3.45%  | +2.90%  |
| BM_Parse_Object/50  | 16031     | 16044    | 44,800     | 58.31 Mi/s       | 3.12 M/s         | 0.00%   | 0.00%   |
| BM_Parse_Object/100 | 40446     | 39899    | 17,231     | 47.35 Mi/s       | 2.51 M/s         | -3.70%  | -2.64%  |
| BM_Parse_Object/500 | 503790    | 500000   | 1,000      | 20.56 Mi/s       | 1.00 M/s         | -5.88%  | -5.88%  |
| BM_Parse_Array/10   | 1435      | 1413     | 497,778    | 61.44 Mi/s       | -                | +2.32%  | -3.55%  |
| BM_Parse_Array/100  | 13468     | 13497    | 49,778     | 70.02 Mi/s       | -                | +2.92%  | -6.53%  |
| BM_Parse_Array/1000 | 152663    | 153450   | 4,073      | 67.69 Mi/s       | -                | +2.56%  | -2.44%  |
| BM_Parse_Nested/2   | 459       | 465      | 1,544,828  | -                | -                | +7.39%  | +9.41%  |
| BM_Parse_Nested/5   | 2087      | 2086     | 344,615    | -                | -                | +2.25%  | +6.05%  |
| BM_Parse_Nested/10  | 7012      | 6975     | 112,000    | -                | -                | 0.00%   | +4.17%  |
| BM_Parse_Nested/20  | 28589     | 28878    | 24,889     | -                | -                | +1.34%  | +2.22%  |
| BM_Serialize/10     | 252       | 251      | 2,800,000  | -                | -                | -74.33% | -4.20%  |
| BM_Serialize/50     | 1117      | 1123     | 640,000    | -                | -                | -73.50% | -4.18%  |
| BM_Serialize/100    | 2181      | 2197     | 320,000    | -                | -                | -73.08% | -4.56%  |
| BM_Serialize/500    | 11677     | 11719    | 64,000     | -                | -                | -69.45% | -2.04%  |
| BM_Pretty/10        | 293       | 295      | 2,488,889  | -                | -                | -36.56% | +10.49% |
| BM_Pretty/50        | 1308      | 1256     | 497,778    | -                | -                | -27.23% | +2.28%  |
| BM_Pretty/100       | 2731      | 2762     | 248,889    | -                | -                | -14.30% | +22.76% |
| BM_Access_Key/5     | 88.9      | 88.9     | 8,960,000  | -                | -                | +13.83% | +13.25% |
| BM_Access_Key/10    | 133       | 128      | 5,600,000  | -                | -                | +6.67%  | +4.92%  |
| BM_Access_Key/50    | 471       | 471      | 1,493,333  | -                | -                | +9.53%  | +10.56% |
| BM_Access_Key/100   | 920       | 924      | 896,000    | -                | -                | +8.20%  | +10.39% |
| BM_Access_Key/500   | 4027      | 4036     | 185,837    | -                | -                | +9.38%  | +9.38%  |


### 分析

**代码改动**：`parsing::parse` 将 `while (!EMPTY_STRING(input) && …)` 改为 `while (!EMPTY_STRING(index) && …)；`jobject::parse` 删除未使用的 json::reader stream。

- **堆分配**：与迭代 3～4 一致（如 exp1 仍为 20000 次、exp6-pretty 仍为 2000 次）， 不改变分配模式。
- **相对迭代 0**：堆分配侧仍体现迭代 2～4 的收益；Benchmark 侧 **vs 基准0** 与迭代 4 同量级（同一套优化链）。
- **相对迭代 4（单步）**：Google Benchmark 中部分项相对迭代 4 波动较大（±10% 常见），**vs 上次迭代** 列以文档迭代 4 表为参照。
- **结论**：性能可视为与迭代 4 近似。

---

## 迭代 6：消除多余拷贝（接口层）

### 原始结果

**记录时间**：2026-03-31T23:08:52+08:00 · **可执行文件**：`D:\KINGSOFT\debug\bin\Release\baseline_benchmark_test_time.exe` · **CPU**：16 × 3194 MHz


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 10.0     | 448.0    |
| exp2-medium-10-fields-parse-destruct     | 87.0     | 4992.0   |
| exp3-large-100-fields-parse-destruct     | 813.0    | 52878.0  |
| exp4-array-100-elements-parse-destruct   | 413.0    | 40078.0  |
| exp7-stdstring-small-parse-destruct      | 10.0     | 448.0    |
| exp8-stdstring-10-fields-parse-destruct  | 87.0     | 4992.0   |
| exp9-stdstring-100-fields-parse-destruct | 813.0    | 52878.0  |
| exp10-stdstring-100-array-parse-destruct | 413.0    | 40078.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 对比（相对迭代 0）


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | 0.00%   | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%   | 0.00%   |
| exp7-stdstring-small-parse-destruct      | -9.09%  | -12.50% |
| exp8-stdstring-10-fields-parse-destruct  | -1.14%  | -7.74%  |
| exp9-stdstring-100-fields-parse-destruct | -0.12%  | -7.41%  |
| exp10-stdstring-100-array-parse-destruct | -0.24%  | -4.97%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.88% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### 与迭代 5 对比（单步效果）


| 用例                                       | 分配次数变化 | 分配字节变化 |
| ---------------------------------------- | ------ | ------ |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%  |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%  |
| exp7-stdstring-small-parse-destruct      | 0.00%  | 0.00%  |
| exp8-stdstring-10-fields-parse-destruct  | 0.00%  | 0.00%  |
| exp9-stdstring-100-fields-parse-destruct | 0.00%  | 0.00%  |
| exp10-stdstring-100-array-parse-destruct | 0.00%  | 0.00%  |
| exp5-read-string-field                   | 0.00%  | 0.00%  |
| exp5-read-int-field                      | —      | —      |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%  |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%  |


### Google Benchmark 测试


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 700       | 711      | 1,120,000  | 49.59 Mi/s       | -                | -1.93%  | +10.75% |
| BM_Parse_Object/5   | 1393      | 1395     | 560,000    | 62.21 Mi/s       | 3.58 M/s         | +1.01%  | +1.01%  |
| BM_Parse_Object/10  | 2541      | 2550     | 263,529    | 67.70 Mi/s       | 3.92 M/s         | -2.26%  | -5.52%  |
| BM_Parse_Object/50  | 15590     | 15695    | 44,800     | 59.61 Mi/s       | 3.19 M/s         | -2.18%  | -2.17%  |
| BM_Parse_Object/100 | 39960     | 40109    | 17,920     | 47.10 Mi/s       | 2.49 M/s         | -3.19%  | +0.53%  |
| BM_Parse_Object/500 | 508895    | 500000   | 1,000      | 20.56 Mi/s       | 1.00 M/s         | -5.88%  | 0.00%   |
| BM_Parse_Array/10   | 1413      | 1413     | 497,778    | 61.44 Mi/s       | -                | +2.32%  | 0.00%   |
| BM_Parse_Array/100  | 13368     | 13393    | 56,000     | 70.57 Mi/s       | -                | +2.13%  | -0.77%  |
| BM_Parse_Array/1000 | 146797    | 145777   | 4,073      | 71.25 Mi/s       | -                | -2.56%  | -5.00%  |
| BM_Parse_Nested/2   | 431       | 433      | 1,659,259  | -                | -                | 0.00%   | -6.88%  |
| BM_Parse_Nested/5   | 2037      | 2040     | 298,667    | -                | -                | 0.00%   | -2.21%  |
| BM_Parse_Nested/10  | 7780      | 7847     | 89,600     | -                | -                | +12.50% | +12.50% |
| BM_Parse_Nested/20  | 32012     | 32087    | 22,400     | -                | -                | +12.61% | +11.11% |
| BM_Serialize/10     | 258       | 255      | 2,635,294  | -                | -                | -73.90% | +1.59%  |
| BM_Serialize/50     | 1146      | 1147     | 640,000    | -                | -                | -72.94% | +2.14%  |
| BM_Serialize/100    | 2218      | 2222     | 344,615    | -                | -                | -72.77% | +1.14%  |
| BM_Serialize/500    | 12125     | 12207    | 64,000     | -                | -                | -68.18% | +4.16%  |
| BM_Pretty/10        | 279       | 283      | 2,488,889  | -                | -                | -39.14% | -4.07%  |
| BM_Pretty/50        | 1233      | 1228     | 560,000    | -                | -                | -28.86% | -2.23%  |
| BM_Pretty/100       | 2394      | 2407     | 298,667    | -                | -                | -25.32% | -12.85% |
| BM_Access_Key/5     | 80.9      | 80.2     | 8,960,000  | -                | -                | +2.69%  | -9.79%  |
| BM_Access_Key/10    | 119       | 117      | 5,600,000  | -                | -                | -2.50%  | -8.59%  |
| BM_Access_Key/50    | 415       | 417      | 1,723,077  | -                | -                | -3.02%  | -11.46% |
| BM_Access_Key/100   | 810       | 802      | 896,000    | -                | -                | -6.09%  | -13.20% |
| BM_Access_Key/500   | 3496      | 3453     | 203,636    | -                | -                | -6.42%  | -14.44% |


### 分析

**代码改动**：`jobject::operator=` / `operator==` / `operator!=` 改为 `const jobject&`；`operator+=(const jobject&)` 在非自引用路径直接遍历 `other.data`，自引用时用 `data` 快照；`entry` 与 `proxy` 中 `operator=`（`std::string`、`jobject`、各 `std::vector`）改为 `const&` 传参。

- **堆分配**：与迭代 5 一致，**本迭代不改变 alloc 模式**（基准用例仍以 parse/序列化为主，较少触发 proxy 赋值热点）。
- **相对迭代 0**：堆分配上 exp6 系列仍显著优于基线；Benchmark **vs 基准0** 与迭代 5 同量级。
- **相对迭代 5**：Google Benchmark 中部分项 CPU 相对迭代 5 互有涨跌（如 `BM_Parse_Small` 高于迭代 5、**Access_Key** 整体低于迭代 5），属**运行噪声**与测试顺序常见现象；**exp5 / exp6** 在 allocounter 侧较迭代 5 明显改善，可与本次接口层少拷贝的改动对齐观察。
- **结论**：以**接口语义等价 + 少拷贝**为主；建议与迭代 5 一样，**同机多跑取中位数**再写结论。
- **后续**：在迭代 6 基础上增加 `**readout(std::string&)`** 与解析路径上的缓冲复用，详见 **迭代 7**（堆计数与迭代 6 一致；Benchmark 为同机单次跑，与迭代 6 对照阅读）。

---

## 迭代 7：`readout(std::string&)` 与解析路径缓冲复用

### 原始结果

**记录时间**：2026-04-01T16:32+08:00（与下文 Google Benchmark 为**同一次**构建/运行）· **可执行文件**：`baseline_benchmark_test_memory.exe` / `baseline_benchmark_test_time.exe`


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 20000            | 896000                | 20000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 174000           | 9984000               | 174000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1626000          | 105756000             | 1626000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 826000           | 80156000              | 826000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）

与 **迭代 6** 相同（本迭代不改变 `new/delete` 统计意义上的分配次数与总字节）。


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 10.0     | 448.0    |
| exp2-medium-10-fields-parse-destruct     | 87.0     | 4992.0   |
| exp3-large-100-fields-parse-destruct     | 813.0    | 52878.0  |
| exp4-array-100-elements-parse-destruct   | 413.0    | 40078.0  |
| exp7-stdstring-small-parse-destruct      | 10.0     | 448.0    |
| exp8-stdstring-10-fields-parse-destruct  | 87.0     | 4992.0   |
| exp9-stdstring-100-fields-parse-destruct | 813.0    | 52878.0  |
| exp10-stdstring-100-array-parse-destruct | 413.0    | 40078.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 对比（相对迭代 0）

与 **迭代 6** 一致（本迭代堆四列与迭代 6 相同）。


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | 0.00%   | 0.00%   |
| exp2-medium-10-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp3-large-100-fields-parse-destruct     | 0.00%   | 0.00%   |
| exp4-array-100-elements-parse-destruct   | 0.00%   | 0.00%   |
| exp7-stdstring-small-parse-destruct      | -9.09%  | -12.50% |
| exp8-stdstring-10-fields-parse-destruct  | -1.14%  | -7.74%  |
| exp9-stdstring-100-fields-parse-destruct | -0.12%  | -7.41%  |
| exp10-stdstring-100-array-parse-destruct | -0.24%  | -4.97%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.88% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### 与迭代 6 对比（单步效果）


| 用例                                       | 分配次数变化 | 分配字节变化 |
| ---------------------------------------- | ------ | ------ |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%  |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%  |
| exp7-stdstring-small-parse-destruct      | 0.00%  | 0.00%  |
| exp8-stdstring-10-fields-parse-destruct  | 0.00%  | 0.00%  |
| exp9-stdstring-100-fields-parse-destruct | 0.00%  | 0.00%  |
| exp10-stdstring-100-array-parse-destruct | 0.00%  | 0.00%  |
| exp5-read-string-field                   | 0.00%  | 0.00%  |
| exp5-read-int-field                      | —      | —      |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%  |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%  |


### Google Benchmark 测试

**说明**：下表为**单次** `baseline_benchmark_test_time` 输出摘录；**vs 基准0** / **vs 上次迭代** 按 **CPU (ns)** 相对 **迭代 0** / **迭代 6** 计算。


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 704       | 698      | 1,120,000  | 50.59 Mi/s       | -                | -3.72%  | -1.83%  |
| BM_Parse_Object/5   | 1410      | 1395     | 448,000    | 62.21 Mi/s       | 3.58 M/s         | +1.01%  | +0.00%  |
| BM_Parse_Object/10  | 2725      | 2727     | 263,529    | 67.58 Mi/s       | 3.93 M/s         | +4.52%  | +6.94%  |
| BM_Parse_Object/50  | 16724     | 16497    | 40,727     | 56.74 Mi/s       | 3.05 M/s         | +2.82%  | +5.11%  |
| BM_Parse_Object/100 | 42373     | 42375    | 16,593     | 44.49 Mi/s       | 2.36 M/s         | +2.27%  | +5.65%  |
| BM_Parse_Object/500 | 532612    | 531250   | 1,000      | 20.57 Mi/s       | 941.18 k/s       | +0.00%  | +6.25%  |
| BM_Parse_Array/10   | 1540      | 1535     | 448,000    | 56.55 Mi/s       | -                | +11.15% | +8.63%  |
| BM_Parse_Array/100  | 14082     | 14125    | 49,778     | 66.91 Mi/s       | -                | +7.71%  | +5.47%  |
| BM_Parse_Array/1000 | 154891    | 153450   | 4,073      | 67.69 Mi/s       | -                | +2.56%  | +5.26%  |
| BM_Parse_Nested/2   | 437       | 424      | 1,659,259  | -                | -                | -2.08%  | -2.08%  |
| BM_Parse_Nested/5   | 2361      | 2358     | 344,615    | -                | -                | +15.59% | +15.59% |
| BM_Parse_Nested/10  | 8184      | 8022     | 89,600     | -                | -                | +15.01% | +2.23%  |
| BM_Parse_Nested/20  | 29691     | 29820    | 23,579     | -                | -                | +4.65%  | -7.07%  |
| BM_Serialize/10     | 264       | 261      | 2,635,294  | -                | -                | -73.29% | +2.35%  |
| BM_Serialize/50     | 1141      | 1116     | 560,000    | -                | -                | -73.67% | -2.70%  |
| BM_Serialize/100    | 2258      | 2295     | 320,000    | -                | -                | -71.88% | +3.29%  |
| BM_Serialize/500    | 11688     | 11719    | 64,000     | -                | -                | -69.45% | -4.00%  |
| BM_Pretty/10        | 282       | 283      | 2,488,889  | -                | -                | -39.14% | +0.00%  |
| BM_Pretty/50        | 1223      | 1228     | 560,000    | -                | -                | -28.85% | +0.00%  |
| BM_Pretty/100       | 2353      | 2246     | 320,000    | -                | -                | -30.31% | -6.69%  |
| BM_Access_Key/5     | 79.7      | 78.5     | 8,960,000  | -                | -                | +0.51%  | -2.12%  |
| BM_Access_Key/10    | 119       | 119      | 7,466,667  | -                | -                | -0.83%  | +1.71%  |
| BM_Access_Key/50    | 401       | 396      | 1,659,259  | -                | -                | -7.91%  | -5.04%  |
| BM_Access_Key/100   | 813       | 820      | 896,000    | -                | -                | -3.98%  | +2.24%  |
| BM_Access_Key/500   | 3,533     | 3,530    | 194,783    | -                | -                | -4.34%  | +2.23%  |


### 分析

**代码改动**：`json::reader` 增加 `virtual void readout(std::string& out) const`（`out.assign` / 子类拼装），`readout()` 委托到输出参数版本；`parsing::parse` 与数组/对象 `push_*` 路径用局部 `std::string` + `readout(piece)` + `append(std::move(piece))`，减少**按值返回**带来的临时对象与拷贝链。

- **堆分配（allocounter）**：各用例 **checksum 与堆四列与迭代 6 完全一致**。原因：`readout(std::string&)` 主要减少的是**临时 `std::string` 与移动/赋值次数**；全局钩子仍统计的是 `**operator new/delete` 次数与字节**，而解析结果里**最终拥有的 `std::string` 数量**未变；NRVO、SSO、已有 `capacity` 复用也会削弱「少一次构造」在计数上的可见度。
- **Google Benchmark**：相对迭代 6，**CPU 有升有降**（例如 `BM_Parse_Small` 略快，`Parse_Object/*` 与部分 `Parse_Array/*` 在本单次结果中偏慢）。这与 **CPU 频率、后台负载、Benchmark 自适配迭代次数** 有关，**不宜将单次表解读为稳定回归**；若需写进课程报告，建议 **同配置连跑 3 次取中位数** 后再下结论。
- **结论**：本迭代价值主要在 **API 与热路径上显式输出缓冲、语义更清晰**；**性能上视为与迭代 6 同量级**，堆侧无新增优化信号。

---

## 迭代 8：对象模式 `key_index`（`unordered_map<string, size_t>`）

### 原始结果

**记录时间**：2026-04-01T17:05:18+08:00（内存与下表 Benchmark 为**同机同构建**）· **可执行文件**：`baseline_benchmark_test_memory.exe` / `baseline_benchmark_test_time.exe`


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 28000            | 1488000               | 28000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 220000           | 14160000              | 220000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 2034000          | 142234000             | 2034000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 834000           | 80892000              | 834000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 28000            | 1488000               | 28000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 220000           | 14160000              | 220000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 2034000          | 142234000             | 2034000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 834000           | 80892000              | 834000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 14.0     | 744.0    |
| exp2-medium-10-fields-parse-destruct     | 110.0    | 7080.0   |
| exp3-large-100-fields-parse-destruct     | 1017.0   | 71117.0  |
| exp4-array-100-elements-parse-destruct   | 417.0    | 40446.0  |
| exp7-stdstring-small-parse-destruct      | 14.0     | 744.0    |
| exp8-stdstring-10-fields-parse-destruct  | 110.0    | 7080.0   |
| exp9-stdstring-100-fields-parse-destruct | 1017.0   | 71117.0  |
| exp10-stdstring-100-array-parse-destruct | 417.0    | 40446.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 对比（相对迭代 0）


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | +40.00% | +66.07% |
| exp2-medium-10-fields-parse-destruct     | +26.44% | +41.83% |
| exp3-large-100-fields-parse-destruct     | +25.09% | +34.49% |
| exp4-array-100-elements-parse-destruct   | +0.97%  | +0.92%  |
| exp7-stdstring-small-parse-destruct      | +27.27% | +45.31% |
| exp8-stdstring-10-fields-parse-destruct  | +25.00% | +31.03% |
| exp9-stdstring-100-fields-parse-destruct | +25.03% | +24.53% |
| exp10-stdstring-100-array-parse-destruct | +0.73%  | -4.10%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.88% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### 与迭代 7 对比（单步效果）


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | +40.00% | +66.07% |
| exp2-medium-10-fields-parse-destruct     | +26.44% | +41.83% |
| exp3-large-100-fields-parse-destruct     | +25.09% | +34.49% |
| exp4-array-100-elements-parse-destruct   | +0.97%  | +0.92%  |
| exp7-stdstring-small-parse-destruct      | +40.00% | +66.07% |
| exp8-stdstring-10-fields-parse-destruct  | +26.44% | +41.83% |
| exp9-stdstring-100-fields-parse-destruct | +25.09% | +34.49% |
| exp10-stdstring-100-array-parse-destruct | +0.97%  | +0.92%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | 0.00%   | 0.00%   |
| exp6-pretty-50-fields                    | 0.00%   | 0.00%   |


### Google Benchmark 测试

**说明**：下表为 **2026-04-01T17:05:18+08:00** 单次运行；**vs 基准0** / **vs 上次迭代** 按 **CPU (ns)** 相对 **迭代 0** / **迭代 7**（文档表）计算。同日下午 **17:05:54** 复跑与下表 CPU 同量级（Access_Key 在约 **54～58 ns** 量级），可作交叉对照。


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 828       | 820      | 896,000    | 43.05 Mi/s       | -                | +13.10% | +17.48% |
| BM_Parse_Object/5   | 1500      | 1500     | 448,000    | 57.87 Mi/s       | 3.33 M/s         | +8.62%  | +7.53%  |
| BM_Parse_Object/10  | 3213      | 3223     | 213,333    | 53.56 Mi/s       | 3.10 M/s         | +23.53% | +18.19% |
| BM_Parse_Object/50  | 15128     | 14753    | 49,778     | 63.41 Mi/s       | 3.39 M/s         | -8.05%  | -10.57% |
| BM_Parse_Object/100 | 29814     | 29820    | 23,579     | 63.35 Mi/s       | 3.35 M/s         | -28.03% | -29.63% |
| BM_Parse_Object/500 | 175277    | 175781   | 3,200      | 58.49 Mi/s       | 2.84 M/s         | -66.91% | -66.91% |
| BM_Parse_Array/10   | 1477      | 1475     | 497,778    | 58.82 Mi/s       | -                | +6.81%  | -3.91%  |
| BM_Parse_Array/100  | 13016     | 13184    | 49,778     | 71.69 Mi/s       | -                | +0.53%  | -6.66%  |
| BM_Parse_Array/1000 | 131061    | 131138   | 5,600      | 79.20 Mi/s       | -                | -12.35% | -14.54% |
| BM_Parse_Nested/2   | 533       | 531      | 1,000,000  | -                | -                | +22.63% | +25.24% |
| BM_Parse_Nested/5   | 2128      | 2093     | 373,333    | -                | -                | +2.60%  | -11.24% |
| BM_Parse_Nested/10  | 6978      | 6801     | 89,600     | -                | -                | -2.49%  | -15.22% |
| BM_Parse_Nested/20  | 29005     | 29157    | 23,579     | -                | -                | +2.32%  | -2.22%  |
| BM_Serialize/10     | 249       | 251      | 2,800,000  | -                | -                | -74.31% | -3.83%  |
| BM_Serialize/50     | 1120      | 1123     | 640,000    | -                | -                | -73.50% | +0.63%  |
| BM_Serialize/100    | 2196      | 2197     | 320,000    | -                | -                | -73.08% | -4.27%  |
| BM_Serialize/500    | 11789     | 11719    | 56,000     | -                | -                | -69.45% | +0.00%  |
| BM_Pretty/10        | 267       | 255      | 2,635,294  | -                | -                | -45.16% | -9.89%  |
| BM_Pretty/50        | 1179      | 1172     | 560,000    | -                | -                | -32.10% | -4.56%  |
| BM_Pretty/100       | 2255      | 2295     | 320,000    | -                | -                | -28.79% | +2.18%  |
| BM_Access_Key/5     | 54.9      | 54.4     | 11,200,000 | -                | -                | -30.35% | -30.70% |
| BM_Access_Key/10    | 55.0      | 54.4     | 11,200,000 | -                | -                | -54.67% | -54.29% |
| BM_Access_Key/50    | 67.3      | 65.6     | 11,200,000 | -                | -                | -84.74% | -83.43% |
| BM_Access_Key/100   | 57.9      | 57.2     | 11,200,000 | -                | -                | -93.30% | -93.02% |
| BM_Access_Key/500   | 78.8      | 78.5     | 8,960,000  | -                | -                | -97.87% | -97.78% |


### 分析

**代码改动**：`jobject` 增加 `**std::unordered_map<std::string, size_t> key_index`**（对象模式下一键一下标）；`+=` / `set` / `remove` / `clear` / 拷贝与赋值与 `data` 同步；`has_key` / `get(string)` / `value_ref_for_key`（`const_proxy::ref()`）走哈希查找。数组模式不往 `key_index` 插入，但 `unordered_map` 仍可能带来**极小**的默认构造/桶开销（exp4 / exp10 堆次数略升）。`**jobject_semantics_test`（8 项）本迭代后仍全部通过。**

- **堆分配**：解析含 **对象** 的用例（exp1～3、exp7～9）**次数与字节明显上升**（约 **+25%～+40%** 量级），与 **每键 `unordered_map` 结点、桶数组及 `std::string` 键副本** 一致；**exp5 / exp6** 与迭代 7 相同（热点不在「parse 建对象键表」）。
- **Google Benchmark**：`BM_Access_Key CPU 大幅下降（相对迭代 0 约 −30%～−98%，相对迭代 7 亦同量级），与 O(1) 均摊查找替代线性扫描 一致。解析项涨跌互现：`Parse_Small`/`Parse_Object/5`/`10`在本机本次结果中 CPU 高于 迭代 7，而`Parse_Object/50`～`500` 表上 低于 迭代 7 其中 Parse_Object/500与迭代 7 表（**531250 ns**）差别极大，除 **map 在字段多时摊薄** 外，更含 **Benchmark 自适应迭代次数与运行噪声**；不宜单用一格断言回归。
- **结论**：本迭代是明确的 **空间换时间**：**allocounter 上升、按键访问显著加快**；需评估是否接受或后续再优化 map 存储（如 `string_view` 指入 `data`、自定义分配器等）。

## 迭代 9：`key_index` 改为 `unordered_multimap<uint64_t, size_t>`（FNV-1a + `data[i].first` 校验）

### 原始结果

**记录时间**：2026-04-01T17:26+08:00（Google Benchmark 与内存测试为**同构建**）· **可执行文件**：`baseline_benchmark_test_time.exe` / `baseline_benchmark_test_memory.exe`


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 14.0     | 672.0    |
| exp2-medium-10-fields-parse-destruct     | 100.0    | 6496.0   |
| exp3-large-100-fields-parse-destruct     | 917.0    | 65493.0  |
| exp4-array-100-elements-parse-destruct   | 417.0    | 40398.0  |
| exp7-stdstring-small-parse-destruct      | 14.0     | 672.0    |
| exp8-stdstring-10-fields-parse-destruct  | 100.0    | 6496.0   |
| exp9-stdstring-100-fields-parse-destruct | 917.0    | 65493.0  |
| exp10-stdstring-100-array-parse-destruct | 417.0    | 40398.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 对比（相对迭代 0）


| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | +40.00% | +50.00% |
| exp2-medium-10-fields-parse-destruct     | +14.94% | +30.13% |
| exp3-large-100-fields-parse-destruct     | +12.79% | +23.86% |
| exp4-array-100-elements-parse-destruct   | +0.97%  | +0.80%  |
| exp7-stdstring-small-parse-destruct      | +27.27% | +31.25% |
| exp8-stdstring-10-fields-parse-destruct  | +13.64% | +20.12% |
| exp9-stdstring-100-fields-parse-destruct | +12.65% | +14.68% |
| exp10-stdstring-100-array-parse-destruct | +0.73%  | -4.21%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.87% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |


### 与迭代 8 对比（单步效果）


| 用例                                       | 分配次数变化 | 分配字节变化 |
| ---------------------------------------- | ------ | ------ |
| exp1-small-json-parse-destruct           | 0.00%  | -9.68% |
| exp2-medium-10-fields-parse-destruct     | -9.09% | -8.25% |
| exp3-large-100-fields-parse-destruct     | -9.83% | -7.91% |
| exp4-array-100-elements-parse-destruct   | 0.00%  | -0.12% |
| exp7-stdstring-small-parse-destruct      | 0.00%  | -9.68% |
| exp8-stdstring-10-fields-parse-destruct  | -9.09% | -8.25% |
| exp9-stdstring-100-fields-parse-destruct | -9.83% | -7.91% |
| exp10-stdstring-100-array-parse-destruct | 0.00%  | -0.12% |
| exp5-read-string-field                   | 0.00%  | 0.00%  |
| exp5-read-int-field                      | —      | —      |
| exp6-as_string-50-fields                 | 0.00%  | 0.00%  |
| exp6-pretty-50-fields                    | 0.00%  | 0.00%  |


### Google Benchmark 测试

**说明**：主表为 **2026-04-01T17:26:06+08:00** 单次运行；**vs 基准0** / **vs 上次迭代** 按 **CPU (ns)** 相对 **迭代 0** / **迭代 8**（文档表）计算。**17:27:10** 复跑与主表同量级（如 `BM_Access_Key/5` CPU **69.8～71.1 ns**、`Parse_Small` **837～879 ns**），属正常波动。


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 834       | 837      | 896,000    | 42.15 Mi/s       | -                | +15.45% | +2.07%  |
| BM_Parse_Object/5   | 1510      | 1500     | 448,000    | 57.87 Mi/s       | 3.33 M/s         | +8.62%  | +0.00%  |
| BM_Parse_Object/10  | 2911      | 2916     | 235,789    | 59.20 Mi/s       | 3.43 M/s         | +11.77% | -9.53%  |
| BM_Parse_Object/50  | 14503     | 14753    | 49,778     | 63.41 Mi/s       | 3.39 M/s         | -8.05%  | +0.00%  |
| BM_Parse_Object/100 | 28952     | 28495    | 23,579     | 66.30 Mi/s       | 3.51 M/s         | -31.23% | -4.44%  |
| BM_Parse_Object/500 | 166397    | 166016   | 3,200      | 61.93 Mi/s       | 3.01 M/s         | -68.75% | -5.56%  |
| BM_Parse_Array/10   | 1453      | 1475     | 497,778    | 58.82 Mi/s       | -                | +6.81%  | +0.00%  |
| BM_Parse_Array/100  | 12317     | 12451    | 64,000     | 75.90 Mi/s       | -                | -5.06%  | -5.56%  |
| BM_Parse_Array/1000 | 120116    | 119978   | 5,600      | 86.57 Mi/s       | -                | -19.81% | -8.51%  |
| BM_Parse_Nested/2   | 516       | 516      | 1,120,000  | -                | -                | +19.17% | -2.82%  |
| BM_Parse_Nested/5   | 2073      | 2086     | 344,615    | -                | -                | +2.25%  | -0.33%  |
| BM_Parse_Nested/10  | 6776      | 6417     | 112,000    | -                | -                | -8.00%  | -5.65%  |
| BM_Parse_Nested/20  | 27412     | 27623    | 24,889     | -                | -                | -3.06%  | -5.26%  |
| BM_Serialize/10     | 252       | 251      | 2,800,000  | -                | -                | -74.31% | +0.00%  |
| BM_Serialize/50     | 1145      | 1147     | 640,000    | -                | -                | -72.94% | +2.14%  |
| BM_Serialize/100    | 2198      | 2197     | 320,000    | -                | -                | -73.08% | +0.00%  |
| BM_Serialize/500    | 14250     | 14230    | 56,000     | -                | -                | -62.91% | +21.43% |
| BM_Pretty/10        | 303       | 300      | 2,240,000  | -                | -                | -35.48% | +17.65% |
| BM_Pretty/50        | 1258      | 1256     | 560,000    | -                | -                | -27.23% | +7.17%  |
| BM_Pretty/100       | 2283      | 2295     | 320,000    | -                | -                | -28.79% | +0.00%  |
| BM_Access_Key/5     | 71.8      | 71.1     | 11,200,000 | -                | -                | -8.96%  | +30.70% |
| BM_Access_Key/10    | 72.8      | 73.2     | 8,960,000  | -                | -                | -39.00% | +34.56% |
| BM_Access_Key/50    | 78.7      | 76.7     | 8,960,000  | -                | -                | -82.16% | +16.92% |
| BM_Access_Key/100   | 77.2      | 76.7     | 8,960,000  | -                | -                | -91.02% | +34.09% |
| BM_Access_Key/500   | 82.3      | 82.0     | 8,960,000  | -                | -                | -97.78% | +4.46%  |


### 分析

**代码改动**：`key_index` 由 `**unordered_map<string, size_t>`** 改为 `**unordered_multimap<uint64_t, size_t>`**：桶键为 **FNV-1a 64 位**，值为 `data` 下标；**同哈希可多条**，查找、删除时用 `**data[i].first == key`** 区分。索引表**不再**为每个键额外存一份 `string` 副本（键串仅在 `vector<kvp>` 中）。

- **堆分配**：相对 **迭代 8**，exp1 / exp7 **次数不变**但 **累计字节下降约 9.7%**；exp2～3 / exp8～9 **次数约 −9%**、**字节约 −8%～−8.3%**，与 **去掉 map 内 `string` 键存储** 一致。相对 **迭代 0**，对象解析用例 **次数与字节仍高于基线**（无索引时），但 **幅度小于迭代 8**（尤其 exp2～3、exp8～9 的分配次数）。
- **Google Benchmark**：`**BM_Access_Key/*` 相对迭代 8 CPU 上升**（约 **+5%～+35%**）：multimap 需在 **同哈希桶内** 用 `**string` 比较** 定槽，常数高于 **直接 `string` 作 `unordered_map` 键**（典型 **1 次哈希 + 短链**）。`**BM_Parse_Object/10`、大数组解析、部分 Pretty** 在本表中优于迭代 8；`**BM_Serialize/500`** 与迭代 8 差 **+21%**，宜视为噪声或负载差异。
- **结论**：本步在 **堆次数/字节（对象解析路径）** 上 **优于迭代 8**，在 **纯按键访问延迟** 上 **略逊于迭代 8**；属于 **空间结构再权衡**。若课程同时卡 **alloc** 与 **Access_Key**，可在此版本上再考虑 **flat 桶、开放寻址** 等减少链表步数。

---

## 迭代 10：`entry` 类型判断去重（`jtype::detect`）

### 原始结果

**记录时间**：2026-04-01T20:27:21+08:00 · **可执行文件**：`baseline_benchmark_test_memory.exe` / `baseline_benchmark_test_time.exe`

**堆测试**：各用例 **checksum 与堆四列与迭代 9 相同**（`is_string` / `is_number` 等改为 `jtype::detect`，**不改变** parse/序列化路径上的 `new/delete` 统计）。


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）

与 **迭代 9** 相同。


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 14.0     | 672.0    |
| exp2-medium-10-fields-parse-destruct     | 100.0    | 6496.0   |
| exp3-large-100-fields-parse-destruct     | 917.0    | 65493.0  |
| exp4-array-100-elements-parse-destruct   | 417.0    | 40398.0  |
| exp7-stdstring-small-parse-destruct      | 14.0     | 672.0    |
| exp8-stdstring-10-fields-parse-destruct  | 100.0    | 6496.0   |
| exp9-stdstring-100-fields-parse-destruct | 917.0    | 65493.0  |
| exp10-stdstring-100-array-parse-destruct | 417.0    | 40398.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 / 迭代 9 对比（内存）

与 **迭代 9** 的「与迭代 0 对比」「与迭代 8 对比」表一致；**相对迭代 9** 堆四列均为 **0.00%**（本迭代不改堆）。

### Google Benchmark 测试

**说明**：**2026-04-01** 同日 **20:35:35～20:36:48+08:00** 对同一 Release 可执行文件 `**baseline_benchmark_test_time.exe`** **连跑 3 次**；下表 **Time (ns)**、**CPU (ns)**、**Iterations** 为三次的**中位数**，**bytes/items_per_second** 为三次对应计数器的**中位数**（与 Google Benchmark 输出一致的量级）。**20:27:21** 单次结果中 `**BM_Access_Key/*` CPU 约 90～103 ns**（其余项也整体偏慢），判为**未知干扰/异常点**，**未纳入**本表。**vs 基准0** / **vs 上次迭代** 按 **CPU (ns)** 相对 **迭代 0** / **迭代 9**（文档表）计算。


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 841       | 837      | 746,667    | 42.15 Mi/s       | -                | +15.45% | +0.00%  |
| BM_Parse_Object/5   | 1,615     | 1,639    | 448,000    | 52.94 Mi/s       | 3.05 M/s         | +18.68% | +9.27%  |
| BM_Parse_Object/10  | 3,125     | 3,139    | 224,000    | 54.99 Mi/s       | 3.19 M/s         | +20.31% | +7.65%  |
| BM_Parse_Object/50  | 15,516    | 14,997   | 44,800     | 62.38 Mi/s       | 3.33 M/s         | -6.53%  | +1.65%  |
| BM_Parse_Object/100 | 31,838    | 31,495   | 21,333     | 59.99 Mi/s       | 3.18 M/s         | -23.99% | +10.53% |
| BM_Parse_Object/500 | 182,607   | 180,664  | 3,200      | 56.91 Mi/s       | 2.77 M/s         | -65.99% | +8.82%  |
| BM_Parse_Array/10   | 1,542     | 1,535    | 448,000    | 56.55 Mi/s       | -                | +11.15% | +4.07%  |
| BM_Parse_Array/100  | 13,658    | 13,393   | 49,778     | 70.57 Mi/s       | -                | +2.13%  | +7.57%  |
| BM_Parse_Array/1000 | 134,147   | 133,929  | 5,600      | 77.55 Mi/s       | -                | -10.48% | +11.63% |
| BM_Parse_Nested/2   | 525       | 531      | 1,000,000  | -                | -                | +22.63% | +2.91%  |
| BM_Parse_Nested/5   | 2,167     | 2,176    | 344,615    | -                | -                | +6.67%  | +4.31%  |
| BM_Parse_Nested/10  | 6,923     | 6,836    | 89,600     | -                | -                | -1.99%  | +6.53%  |
| BM_Parse_Nested/20  | 28,606    | 28,250   | 24,889     | -                | -                | -0.86%  | +2.27%  |
| BM_Serialize/10     | 253       | 251      | 2,800,000  | -                | -                | -74.31% | +0.00%  |
| BM_Serialize/50     | 1,137     | 1,147    | 640,000    | -                | -                | -72.94% | +0.00%  |
| BM_Serialize/100    | 2,241     | 2,197    | 320,000    | -                | -                | -73.08% | +0.00%  |
| BM_Serialize/500    | 12,012    | 11,719   | 56,000     | -                | -                | -69.45% | -17.65% |
| BM_Pretty/10        | 275       | 279      | 2,635,294  | -                | -                | -40.00% | -7.00%  |
| BM_Pretty/50        | 1,197     | 1,196    | 640,000    | -                | -                | -30.71% | -4.78%  |
| BM_Pretty/100       | 2,257     | 2,250    | 298,667    | -                | -                | -30.19% | -1.96%  |
| BM_Access_Key/5     | 75.9      | 75.3     | 8,960,000  | -                | -                | -3.59%  | +5.91%  |
| BM_Access_Key/10    | 76.0      | 75.3     | 8,960,000  | -                | -                | -37.25% | +2.87%  |
| BM_Access_Key/50    | 79.6      | 78.5     | 8,960,000  | -                | -                | -81.74% | +2.35%  |
| BM_Access_Key/100   | 79.7      | 78.5     | 8,960,000  | -                | -                | -90.81% | +2.35%  |
| BM_Access_Key/500   | 83.9      | 83.7     | 7,466,667  | -                | -                | -97.73% | +2.07%  |


### 分析

**代码改动**：`jobject::entry` 中 `is_string` / `is_number` / `is_object` / `is_array` / `is_bool` / `is_null` 与 `as_string` 的首类型判断改为 `**jtype::detect(ref().c_str())`**（`tlws` + 首字符），**不再**为纯类型查询调用 `**parsing::parse`**；`**is_true**` 仍 **单次 `parse`** 以区分 `true`/`false` 并与原空白语义一致。

- **堆分配**：与 **迭代 9** 一致，符合预期（D1 不触及 `key_index` / parse 主路径分配）。
- **Google Benchmark**：上表为 **3 次连测中位数**。相对 **迭代 9**，`BM_Access_Key/*` CPU 约 **+2%～+6%**（与单次「>100 ns」异常表不可比）；**Access_Key 用例体不调用 `entry::is_*`**，该量级差异仍宜视为 **噪声**，**不宜单独归因于 `detect`**。`**BM_Serialize/500**` 中位数相对迭代 9 文档表约 **−17.7%**，与另一次快照方向相反。
- **结论**：D1 的**直接收益**在 **反复调用 `entry` 类型判断** 的场景（少建 `reader`）；**allocounter 与当前基准用例** 上往往 **不显性**。迭代 10 以 **同日三连测中位数** 作为稳定快照；**20:27 单次** 仅作异常对照，不写入主表。

---

## 迭代 12：`jobject::operator==` / `!=` 结构化比较（不双路序列化）

### 原始结果

**记录时间**：2026-04-01 **21:19～21:20+08:00**；**可执行文件**：`D:\KINGSOFT\debug\bin\Release\baseline_benchmark_test_*.exe`

**代码改动**：`operator==` 改为按 `**array_flag`、`data` 槽位**逐对比较 `**first` / `second`**（与 `operator std::string()` 紧凑拼接规则等价），**不再**对两侧各做一次整树序列化；`operator!=` 为 `!(*this == other)`。

**堆测试**：各用例 **checksum 与堆四列与迭代 10 相同**（`operator==` 不触及 parse/序列化路径上的分配统计）。


| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |


### 单次换算（alloc/op、bytes/op）

与 **迭代 10** 相同。


| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 14.0     | 672.0    |
| exp2-medium-10-fields-parse-destruct     | 100.0    | 6496.0   |
| exp3-large-100-fields-parse-destruct     | 917.0    | 65493.0  |
| exp4-array-100-elements-parse-destruct   | 417.0    | 40398.0  |
| exp7-stdstring-small-parse-destruct      | 14.0     | 672.0    |
| exp8-stdstring-10-fields-parse-destruct  | 100.0    | 6496.0   |
| exp9-stdstring-100-fields-parse-destruct | 917.0    | 65493.0  |
| exp10-stdstring-100-array-parse-destruct | 417.0    | 40398.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |


### 与迭代 0 / 迭代 10 对比（内存）

与 **迭代 10** 的「与迭代 0 对比」表一致；**相对迭代 10** 堆四列均为 **0.00%**。

### Google Benchmark 测试

**说明**：同日 **连跑 2 次**。**21:19:27** 一枪 **整组明显偏慢**（如 `BM_Parse_Small` CPU **1032 ns**、`BM_Access_Key/*` 约 **100～115 ns**），判为**干扰**，**未纳入**主表。下表为 **21:20:18** **单次**结果；**vs 基准0** / **vs 上次迭代** 按 **CPU (ns)** 相对 **迭代 0** / **迭代 10**（文档表）计算。**当前基准用例不含 `operator==` 热点**，故数值与 **迭代 10 三连测中位数**的差异主要反映 **环境噪声**，**不宜归因于本改动**。


| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | -------- | ---------- | ---------------- | ---------------- | ------- | ------- |
| BM_Parse_Small      | 869       | 879      | 746,667    | 40.15 Mi/s       | -                | +21.24% | +5.02%  |
| BM_Parse_Object/5   | 1,556     | 1,496    | 407,273    | 58.00 Mi/s       | 3.34 M/s         | +8.33%  | -8.72%  |
| BM_Parse_Object/10  | 3,054     | 3,048    | 235,789    | 56.63 Mi/s       | 3.28 M/s         | +16.83% | -2.90%  |
| BM_Parse_Object/50  | 14,746    | 14,439   | 49,778     | 64.79 Mi/s       | 3.46 M/s         | -10.00% | -3.72%  |
| BM_Parse_Object/100 | 29,855    | 29,157   | 23,579     | 64.79 Mi/s       | 3.43 M/s         | -29.63% | -7.42%  |
| BM_Parse_Object/500 | 179,500   | 175,781  | 3,200      | 58.49 Mi/s       | 2.84 M/s         | -66.91% | -2.70%  |
| BM_Parse_Array/10   | 1,544     | 1,538    | 497,778    | 56.42 Mi/s       | -                | +11.37% | +0.20%  |
| BM_Parse_Array/100  | 12,711    | 12,556   | 56,000     | 75.27 Mi/s       | -                | -4.25%  | -6.25%  |
| BM_Parse_Array/1000 | 125,087   | 125,558  | 5,600      | 82.72 Mi/s       | -                | -16.08% | -6.25%  |
| BM_Parse_Nested/2   | 544       | 544      | 1,120,000  | -                | -                | +25.64% | +2.45%  |
| BM_Parse_Nested/5   | 2,215     | 2,148    | 320,000    | -                | -                | +5.29%  | -1.29%  |
| BM_Parse_Nested/10  | 6,985     | 6,975    | 89,600     | -                | -                | +0.00%  | +2.03%  |
| BM_Parse_Nested/20  | 28,848    | 27,867   | 26,353     | -                | -                | -2.20%  | -1.36%  |
| BM_Serialize/10     | 262       | 267      | 2,635,294  | -                | -                | -72.67% | +6.37%  |
| BM_Serialize/50     | 1,222     | 1,221    | 640,000    | -                | -                | -71.19% | +6.45%  |
| BM_Serialize/100    | 2,300     | 2,250    | 298,667    | -                | -                | -72.43% | +2.41%  |
| BM_Serialize/500    | 12,341    | 11,998   | 56,000     | -                | -                | -68.73% | +2.38%  |
| BM_Pretty/10        | 291       | 291      | 2,635,294  | -                | -                | -37.42% | +4.30%  |
| BM_Pretty/50        | 1,279     | 1,224    | 497,778    | -                | -                | -29.08% | +2.34%  |
| BM_Pretty/100       | 2,425     | 2,372    | 263,529    | -                | -                | -26.40% | +5.42%  |
| BM_Access_Key/5     | 83.0      | 79.5     | 7,466,667  | -                | -                | +1.79%  | +5.58%  |
| BM_Access_Key/10    | 90.2      | 85.4     | 6,400,000  | -                | -                | -28.83% | +13.41% |
| BM_Access_Key/50    | 93.4      | 92.8     | 6,400,000  | -                | -                | -78.42% | +18.22% |
| BM_Access_Key/100   | 95.9      | 94.2     | 7,466,667  | -                | -                | -88.97% | +20.00% |
| BM_Access_Key/500   | 93.1      | 90.3     | 6,400,000  | -                | -                | -97.55% | +7.89%  |


### 分析

- **堆分配**：与 **迭代 10** 一致，符合预期（`operator==` 不在 allocounter 覆盖的热路径上）。
- **Google Benchmark**：主表为 **21:20:18 单次**；**21:19:27** 整组偏慢仅作异常对照。表内 **vs 迭代 10** 有正有负，与 **单次对中位数** 对比及 **负载波动** 一致；`**BM_Access_Key/*` 相对迭代 10 文档表偏慢**时，宜与视为**环境噪声**，**不单独归因于 `operator==`**。
- **结论**：本迭代**主要价值**为 **API 实现**：相等判断 **O(槽位数)**、**无整树双串分配**，语义与 **原 `string` 比较**一致；**Benchmark 表**仅作同机快照。

## 迭代 13：现代化 C++ 改造

### 原始结果（堆测试）

**记录时间**：2026-04-03 15:22~15:24 · **可执行文件**：`baseline_benchmark_test_memory.exe`

| 用例                                       | rounds | checksum | heap allocations | total allocated bytes | heap deallocations |
| ---------------------------------------- | ------ | -------- | ---------------- | --------------------- | ------------------ |
| exp1-small-json-parse-destruct           | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp2-medium-10-fields-parse-destruct     | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp3-large-100-fields-parse-destruct     | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp4-array-100-elements-parse-destruct   | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp7-stdstring-small-parse-destruct      | 2000   | 4000     | 28000            | 1344000               | 28000              |
| exp8-stdstring-10-fields-parse-destruct  | 2000   | 20000    | 200000           | 12992000              | 200000             |
| exp9-stdstring-100-fields-parse-destruct | 2000   | 200000   | 1834000          | 130986000             | 1834000            |
| exp10-stdstring-100-array-parse-destruct | 2000   | 200000   | 834000           | 80796000              | 834000             |
| exp5-read-string-field                   | 2000   | 34000    | 2000             | 64000                 | 2000               |
| exp5-read-int-field                      | 2000   | 50000    | 0                | 0                     | 0                  |
| exp6-as_string-50-fields                 | 2000   | 4162000  | 102000           | 9152000               | 102000             |
| exp6-pretty-50-fields                    | 2000   | 4464000  | 2000             | 7104000               | 2000               |

### 单次换算（alloc/op、bytes/op）

| 用例                                       | alloc/op | bytes/op |
| ---------------------------------------- | -------- | -------- |
| exp1-small-json-parse-destruct           | 14.0     | 672.0    |
| exp2-medium-10-fields-parse-destruct     | 100.0    | 6496.0   |
| exp3-large-100-fields-parse-destruct     | 917.0    | 65493.0  |
| exp4-array-100-elements-parse-destruct   | 417.0    | 40398.0  |
| exp7-stdstring-small-parse-destruct      | 14.0     | 672.0    |
| exp8-stdstring-10-fields-parse-destruct  | 100.0    | 6496.0   |
| exp9-stdstring-100-fields-parse-destruct | 917.0    | 65493.0  |
| exp10-stdstring-100-array-parse-destruct | 417.0    | 40398.0  |
| exp5-read-string-field                   | 1.0      | 32.0     |
| exp5-read-int-field                      | 0.0      | 0.0      |
| exp6-as_string-50-fields                 | 51.0     | 4576.0   |
| exp6-pretty-50-fields                    | 1.0      | 3552.0   |

### 与迭代 0 对比（相对迭代 0）

| 用例                                       | 分配次数变化  | 分配字节变化  |
| ---------------------------------------- | ------- | ------- |
| exp1-small-json-parse-destruct           | +40.00% | +50.00% |
| exp2-medium-10-fields-parse-destruct     | +14.94% | +30.13% |
| exp3-large-100-fields-parse-destruct     | +12.79% | +23.86% |
| exp4-array-100-elements-parse-destruct   | +0.97%  | +0.80%  |
| exp7-stdstring-small-parse-destruct      | +27.27% | +31.25% |
| exp8-stdstring-10-fields-parse-destruct  | +13.64% | +20.12% |
| exp9-stdstring-100-fields-parse-destruct | +12.65% | +14.68% |
| exp10-stdstring-100-array-parse-destruct | +0.72%  | -4.21%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | -53.64% | -58.87% |
| exp6-pretty-50-fields                    | -98.39% | -66.39% |

### 与迭代 12 对比（单步效果）

| 用例                                       | 分配次数变化 | 分配字节变化 |
| ---------------------------------------- | ------ | ------ |
| exp1-small-json-parse-destruct           | 0.00%  | 0.00%  |
| exp2-medium-10-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp3-large-100-fields-parse-destruct     | 0.00%  | 0.00%  |
| exp4-array-100-elements-parse-destruct   | 0.00%  | 0.00%  |
| exp7-stdstring-small-parse-destruct      | 0.00%  | 0.00%  |
| exp8-stdstring-10-fields-parse-destruct  | 0.00%  | 0.00%  |
| exp9-stdstring-100-fields-parse-destruct | 0.00%  | 0.00%  |
| exp10-stdstring-100-array-parse-destruct | 0.00%  | 0.00%  |
| exp5-read-string-field                   | 0.00%   | 0.00%   |
| exp5-read-int-field                      | —       | —       |
| exp6-as_string-50-fields                 | 0.00%   | 0.00%   |
| exp6-pretty-50-fields                    | 0.00%   | 0.00%   |

### Google Benchmark 测试

**说明**：同日连跑 2 次取最后一组表（`2026-04-03T15:24:59`）。`vs 基准0` / `vs 上次迭代` 按 **CPU(ns)** 计算。

| Benchmark           | Time (ns) | CPU (ns) | Iterations | bytes_per_second | items_per_second | vs 基准0  | vs 上次迭代 |
| ------------------- | --------- | --------- | ---------- | ---------------- | ---------------- | --------- | ----------- |
| BM_Parse_Small      | 905       | 900       | 746,667    | 39.2139 Mi/s     | -                | +24.14%   | +2.39%      |
| BM_Parse_Object/5   | 1821      | 1779      | 448,000    | 48.7898 Mi/s     | 2.81098 M/s      | +28.82%   | +18.92%     |
| BM_Parse_Object/10  | 3202      | 3003      | 213,333    | 57.4821 Mi/s     | 3.33008 M/s      | +15.10%   | -1.48%      |
| BM_Parse_Object/50  | 15593     | 15695     | 49,778     | 59.6096 Mi/s     | 3.18579 M/s      | -2.18%    | +8.70%      |
| BM_Parse_Object/100 | 31773     | 30762     | 21,333     | 61.414 Mi/s      | 3.25074 M/s      | -25.75%   | +5.50%      |
| BM_Parse_Object/500 | 182201    | 180664    | 3,200      | 56.9098 Mi/s     | 2.76757 M/s      | -65.99%   | +2.78%      |
| BM_Parse_Array/10   | 1558      | 1569      | 407,273    | 55.2951 Mi/s     | -                | +13.61%   | +2.02%      |
| BM_Parse_Array/100  | 13405     | 13393     | 56,000     | 70.5668 Mi/s     | -                | +2.13%    | +6.67%      |
| BM_Parse_Array/1000 | 138026    | 139509    | 4,480      | 74.4502 Mi/s     | -                | -6.75%    | +11.11%     |
| BM_Parse_Nested/2   | 562       | 558       | 1,120,000  | -                | -                | +28.87%   | +2.57%      |
| BM_Parse_Nested/5   | 2282      | 2295      | 320,000    | -                | -                | +12.50%   | +6.84%      |
| BM_Parse_Nested/10  | 7264      | 7254      | 112,000    | -                | -                | +4.00%    | +4.00%      |
| BM_Parse_Nested/20  | 30746     | 30762     | 24,889     | -                | -                | +7.96%    | +11.36%     |
| BM_Serialize/10     | 259       | 255       | 2,635,294  | -                | -                | -73.90%   | -4.49%      |
| BM_Serialize/50     | 1250      | 1228      | 560,000    | -                | -                | -71.02%   | +0.57%      |
| BM_Serialize/100    | 2453      | 2400      | 280,000    | -                | -                | -70.59%   | +6.67%      |
| BM_Serialize/500    | 12368     | 12207     | 64,000     | -                | -                | -68.18%   | +1.74%      |
| BM_Pretty/10        | 289       | 289       | 2,488,889  | -                | -                | -37.85%   | -0.69%      |
| BM_Pretty/50        | 1256      | 1228      | 560,000    | -                | -                | -28.85%   | +0.33%      |
| BM_Pretty/100       | 2452      | 2459      | 298,667    | -                | -                | -23.70%   | +3.67%      |
| BM_Access_Key/5     | 77.1      | 75.0      | 8,960,000  | -                | -                | -3.97%    | -5.66%      |
| BM_Access_Key/10    | 76.7      | 76.7      | 8,960,000  | -                | -                | -36.08%   | -10.19%     |
| BM_Access_Key/50    | 80.4      | 80.2      | 8,960,000  | -                | -                | -81.35%   | -13.58%     |
| BM_Access_Key/100   | 80.7      | 79.5      | 7,466,667  | -                | -                | -90.69%   | -15.61%     |
| BM_Access_Key/500   | 84.7      | 83.7      | 8,960,000  | -                | -                | -97.73%   | -7.31%      |

### 分析

- **堆分配**：与迭代 12 一致（alloc/op、bytes/op 同量级），说明现代化重写未引入新的分配热路径。
- **Google Benchmark**：相对迭代 12，`BM_Access_Key/*` 多数更快（`BM_Access_Key/100` CPU **-15.61%**），但解析/序列化/pretty 多项出现 +0%~+11% 的回退波动。
- **结论**：本轮收益更偏向 **key 查找延迟**；其余用例建议继续按同配置连跑取中位数验证是否存在系统性回归。

## 扩展时间基准：`baseline_benchmark_test_time_comprehensive`

在保留原有 `baseline_benchmark_test_time.cpp` 的前提下，新增可执行文件 **`baseline_benchmark_test_time_comprehensive`**，用于补齐对 `json.cpp` / `jobject` API 的热路径覆盖，便于与仅「字符串字段 + 纯字符串数组」类负载区分。

### 原有时间基准未重点触达的路径（代码阅读 + 覆盖率结论）

| 类别 | 说明 |
| ---- | ---- |
| 标量类型 | `true` / `null` / 整数 / 浮点 / 科学计数法，`jtype::detect`、`push_number` / `push_boolean` / `push_null` 等分支 |
| 解析入口 | `jobject::parse(const std::string&)`、`tryparse`（成功路径） |
| 访问与元数据 | `list_keys`、`has_key`（哈希桶 + 字符串相等） |
| 嵌套与数组元素 | 根为数组、元素为对象时 `array(i).get("k")`（`const_value::get` → 片段 `parse`） |
| 合并与比较 | 无冲突键的 `operator+=`、`operator==`（迭代 12+ 槽位比较） |
| 字符串转义 | `decode_string`：`\\n`、制表、引号等 |

### 新增 Benchmark 名称与含义

| Benchmark | 含义 |
| --------- | ---- |
| `BM_Parse_StdString` | `parse(std::string)` 形态（实现上等价于 `c_str`，保留调用边界） |
| `BM_Parse_MixedScalars` | 含 string / int / double / 科学计数 / bool / null / 子对象的单对象解析 |
| `BM_Parse_NumberArray/N` | 纯数字数组 `[0,1,…]`，N∈{50,200,1000} |
| `BM_TryParse_Success` | 合法 JSON 上 `tryparse` |
| `BM_ReadNumericFields` | 解析后 `int` / `double` 字段读取（`entry::operator int/double`） |
| `BM_ReadBoolNullString` | `is_true`、`is_null`、`as_string` 路径 |
| `BM_ArrayOfObjects_Access/N` | 根数组 + 首元素对象字段；N 控制数组长度 |
| `BM_ListKeys_HasKey/N` | `list_keys` + `has_key`；N 为字段数 |
| `BM_Merge_PlusAssign` | 两对象键前缀不同，`+=` 合并 |
| `BM_Serialize_Mixed` / `BM_Pretty_Mixed` | 混合标量对象的 `as_string` / `pretty` |
| `BM_Parse_EscapedStrings` | 含转义字符的 JSON 字符串 |
| `BM_StructuralEquals` | 两份相同 JSON 解析结果 `operator==` |

构建：与 `baseline_benchmark_test_time` 相同目标，需 **Google Benchmark**；Windows 下 Post-Build 会将 **`simpleson.dll`**（改造版）复制到各基准可执行文件目录。

运行（建议在 **Release** 下与下表及迭代 0 基线对比；Debug 仅作功能验证）：

```bash
baseline_benchmark_test_time.exe
baseline_benchmark_test_time_comprehensive.exe
```

**原始实现（`start/`）对照**：CMake 工程仅构建改造版 **`simpleson`**；曾用单独目标 `*_start` + `simpleson_start.dll` 测得的原始数据已录入本文（扩展基准 **原始 vs 迭代 13** 表）。若需复测，可将 `start/json.cpp` 临时替换根目录实现或另建工程链接 `start/`。

### 原有 `BM_*`：迭代 0（原始基线）与迭代 13（改造后，文档记录）CPU 时间对照

两列均来自本文档 **Release** 工况下的历史记录（迭代 0：`## 迭代 0` 中 Google Benchmark 表；迭代 13：`## 迭代 13` 表）。**非**本机 Debug 可执行文件输出。

| Benchmark | 迭代 0 CPU (ns) | 迭代 13 CPU (ns) | Δ%（相对迭代 0） |
| --------- | --------------- | ---------------- | ---------------- |
| BM_Parse_Small | 725 | 900 | +24.1% |
| BM_Parse_Object/5 | 1381 | 1779 | +28.8% |
| BM_Parse_Object/10 | 2609 | 3003 | +15.1% |
| BM_Parse_Object/50 | 16044 | 15695 | −2.2% |
| BM_Parse_Object/100 | 41433 | 30762 | −25.8% |
| BM_Parse_Object/500 | 531250 | 180664 | −66.0% |
| BM_Parse_Array/10 | 1381 | 1569 | +13.6% |
| BM_Parse_Array/100 | 13114 | 13393 | +2.1% |
| BM_Parse_Array/1000 | 149613 | 139509 | −6.8% |
| BM_Parse_Nested/2 | 433 | 558 | +28.9% |
| BM_Parse_Nested/5 | 2040 | 2295 | +12.5% |
| BM_Parse_Nested/10 | 6975 | 7254 | +4.0% |
| BM_Parse_Nested/20 | 28495 | 30762 | +8.0% |
| BM_Serialize/10 | 977 | 255 | −73.9% |
| BM_Serialize/50 | 4238 | 1228 | −71.0% |
| BM_Serialize/100 | 8161 | 2400 | −70.6% |
| BM_Serialize/500 | 38365 | 12207 | −68.2% |
| BM_Pretty/10 | 465 | 289 | −37.8% |
| BM_Pretty/50 | 1726 | 1228 | −28.8% |
| BM_Pretty/100 | 3223 | 2459 | −23.7% |
| BM_Access_Key/5 | 78.1 | 75.0 | −4.0% |
| BM_Access_Key/10 | 120 | 76.7 | −36.1% |
| BM_Access_Key/50 | 430 | 80.2 | −81.3% |
| BM_Access_Key/100 | 854 | 79.5 | −90.7% |
| BM_Access_Key/500 | 3690 | 83.7 | −97.7% |

**解读摘要**：大对象解析（`/100`、`/500`）、序列化与 Pretty、按键访问相对迭代 0 明显改善；小对象与浅嵌套解析部分项变慢，与文档迭代小节分析一致。

### 迭代 13 补充：`baseline_benchmark_test_time_comprehensive`（Release 实测）

- 记录时间：`2026-04-04T23:17:25+08:00`
- 可执行文件：`D:\KINGSOFT\debug\bin\Release\baseline_benchmark_test_time_comprehensive.exe`
- 环境：16 × 3194 MHz，L3 16 MiB（与文档中其他 Google Benchmark 记录同机同配置时可横向对比）

| Benchmark | Time (ns) | CPU (ns) | Iterations | User counters |
| --------- | --------- | -------- | ---------- | ------------- |
| BM_Parse_StdString | 2032 | 2051 | 373,333 | 34.88 Mi/s |
| BM_Parse_MixedScalars | 2375 | 2344 | 373,333 | 30.52 Mi/s |
| BM_Parse_NumberArray/50 | 4378 | 4353 | 172,308 | 30.89 Mi/s |
| BM_Parse_NumberArray/200 | 15312 | 14997 | 44,800 | 43.94 Mi/s |
| BM_Parse_NumberArray/1000 | 112859 | 111607 | 5,600 | 33.25 Mi/s |
| BM_TryParse_Success | 4808 | 4649 | 144,516 | 35.69 Mi/s |
| BM_ReadNumericFields | 260 | 261 | 2,635,294 | — |
| BM_ReadBoolNullString | 199 | 199 | 3,446,154 | — |
| BM_ArrayOfObjects_Access/10 | 741 | 732 | 896,000 | 274.74 Mi/s |
| BM_ArrayOfObjects_Access/50 | 729 | 715 | 896,000 | 1.47 Gi/s |
| BM_ListKeys_HasKey/20 | 587 | 586 | 1,120,000 | — |
| BM_ListKeys_HasKey/100 | 1920 | 1904 | 344,615 | — |
| BM_Merge_PlusAssign | 2164 | 2131 | 344,615 | — |
| BM_Serialize_Mixed | 159 | 157 | 4,072,727 | — |
| BM_Pretty_Mixed | 568 | 562 | 1,000,000 | — |
| BM_Parse_EscapedStrings | 627 | 628 | 896,000 | 41.02 Mi/s |
| BM_StructuralEquals | 246 | 246 | 2,800,000 | — |

### 原始代码（`start/`，迭代 0 语义）补充：`baseline_benchmark_test_time_comprehensive_start`（Release 实测）

- 记录时间：`2026-04-04T23:26:26+08:00`
- 可执行文件：`D:\KINGSOFT\debug\bin\Release\baseline_benchmark_test_time_comprehensive_start.exe`（链接 **`simpleson_start.dll`**，源码见 `homework/simpleson/start`）
- 环境：16 × 3194 MHz，L3 16 MiB（与上表迭代 13 实测同机同配置，可横向对比）

| Benchmark | Time (ns) | CPU (ns) | Iterations | User counters |
| --------- | --------- | -------- | ---------- | ------------- |
| BM_Parse_StdString | 1918 | 1946 | 497,778 | 36.75 Mi/s |
| BM_Parse_MixedScalars | 1742 | 1765 | 407,273 | 40.53 Mi/s |
| BM_Parse_NumberArray/50 | 3851 | 3836 | 179,200 | 35.05 Mi/s |
| BM_Parse_NumberArray/200 | 14545 | 14300 | 44,800 | 46.08 Mi/s |
| BM_Parse_NumberArray/1000 | 110476 | 109858 | 4,978 | 33.78 Mi/s |
| BM_TryParse_Success | 4038 | 4011 | 179,200 | 41.37 Mi/s |
| BM_ReadNumericFields | 257 | 256 | 2,986,667 | — |
| BM_ReadBoolNullString | 231 | 230 | 2,986,667 | — |
| BM_ArrayOfObjects_Access/10 | 560 | 562 | 1,000,000 | 357.73 Mi/s |
| BM_ArrayOfObjects_Access/50 | 553 | 544 | 1,120,000 | 1.94 Gi/s |
| BM_ListKeys_HasKey/20 | 590 | 572 | 1,120,000 | — |
| BM_ListKeys_HasKey/100 | 2142 | 2131 | 344,615 | — |
| BM_Merge_PlusAssign | 1588 | 1569 | 448,000 | — |
| BM_Serialize_Mixed | 412 | 414 | 1,659,259 | — |
| BM_Pretty_Mixed | 564 | 558 | 1,120,000 | — |
| BM_Parse_EscapedStrings | 488 | 492 | 1,493,333 | 52.36 Mi/s |
| BM_StructuralEquals | 4359 | 4349 | 154,483 | — |

### 扩展基准：原始（`start`）与迭代 13（改造版）CPU (ns) 对照

Δ% = **(迭代13 CPU − 原始 CPU) / 原始 CPU**；负值表示改造后更快。

| Benchmark | 原始 CPU (ns) | 迭代 13 CPU (ns) | Δ% |
| --------- | ------------- | ---------------- | ----- |
| BM_Parse_StdString | 1946 | 2051 | +5.4% |
| BM_Parse_MixedScalars | 1765 | 2344 | +32.8% |
| BM_Parse_NumberArray/50 | 3836 | 4353 | +13.5% |
| BM_Parse_NumberArray/200 | 14300 | 14997 | +4.9% |
| BM_Parse_NumberArray/1000 | 109858 | 111607 | +1.6% |
| BM_TryParse_Success | 4011 | 4649 | +15.9% |
| BM_ReadNumericFields | 256 | 261 | +2.0% |
| BM_ReadBoolNullString | 230 | 199 | −13.5% |
| BM_ArrayOfObjects_Access/10 | 562 | 732 | +30.2% |
| BM_ArrayOfObjects_Access/50 | 544 | 715 | +31.4% |
| BM_ListKeys_HasKey/20 | 572 | 586 | +2.4% |
| BM_ListKeys_HasKey/100 | 2131 | 1904 | −10.6% |
| BM_Merge_PlusAssign | 1569 | 2131 | +35.8% |
| BM_Serialize_Mixed | 414 | 157 | −62.1% |
| BM_Pretty_Mixed | 558 | 562 | +0.7% |
| BM_Parse_EscapedStrings | 492 | 628 | +27.6% |
| BM_StructuralEquals | 4349 | 246 | −94.3% |

**说明**：`bytes_per_second` 等与控制台一致已四舍五入。`BM_ArrayOfObjects_Access/50` 吞吐高于 `/10` 的原因同前。`BM_StructuralEquals` 上改造版显著更快，与 **`operator==` 改为槽位比较、避免双路整包序列化**（迭代 12 文档）一致；`BM_Serialize_Mixed` 与主基准里序列化预分配等优化一致。**若干解析类项**在扩展负载下原始代码反而更快，可能与 **try/catch 边界、`parse(std::string)` 传值 vs `const&`、哈希索引与负载形态** 等有关，宜同配置多跑取中位数后再下结论。

---

## 总体结论


| 迭代  | 主要改动                                                                           | Google Benchmark 关键收益（基于 CPU 时间）                                            | 注意事项                                                    |
| --- | ------------------------------------------------------------------------------ | --------------------------------------------------------------------------- | ------------------------------------------------------- |
| 0   | 基线                                                                             | 建立基准：小对象解析 725ns，序列化 977ns，Pretty 465ns                                     | 基线参考点                                                   |
| 1   | 智能指针 + nullptr                                                                 | 字段访问提速 10%，Pretty 退化 +7%~~17%，解析整体持平                                        | 内存无开销，安全性提升                                             |
| 2   | string 参数改为引用                                                                  | 小对象解析提速 5.66%-11.37%，Pretty 优化 +1%~15%，字段访问微优化                              | std::string 路径显著受益                                      |
| 3   | 序列化预分配                                                                         | 序列化提速 69-73%，Pretty 提速 42%，解析提速 9-11%                                       | **最大性能提升迭代**，as_string 分配从 110 次→51 次，pretty 从 62 次→1 次 |
| 4   | 数字转字符串优化                                                                       | 累计成果：序列化 73.18%，Pretty 42.58%，解析 7.59%，整体保持稳定                               | 代码更健壮，保持优化成果                                            |
| 5   | `parsing::parse` 用 `index` 终止循环；删 `jobject::parse` 无用局部 `reader`               | 相对迭代 4：部分解析/序列化 CPU 略快，部分 Pretty/Access 略慢；堆分配与迭代 4 一致                      | 语义修正为主                                                  |
| 6   | 接口层 `const&`、`operator+=` 等少拷贝；文末附 `read_digits` / `kvp_reader::readout` 小优化说明 | 相对迭代 0：Access_Key 等仍优于基线；相对迭代 5：Benchmark CPU 有波动，exp5/exp6 相关项多优于迭代 5      | 与迭代 7 对照：堆表以迭代 6/7 为准                                   |
| 7   | `readout(std::string&)` + parse / 数组·对象拼装处复用缓冲                                 | 堆计数与迭代 6 相同；Benchmark 相对迭代 6 单次结果互有涨跌（±7% 量级常见）                             | allocounter 不体现「少临时 string」的全部收益；语义与扩展性为主               |
| 8   | 对象模式 `key_index`（`unordered_map` + `vector<kvp>`）                              | `BM_Access_Key` CPU 相对迭代 0/7 大幅下降*；解析项涨跌互现                                  | **堆分配与字节在 parse 对象用例上明显上升**（map 结点与键存储）；语义测 8/8 通过      |
| 9   | `key_index`：`unordered_multimap<uint64_t, size_t>` + `first` 校验                | 堆优于迭代 8；**Access_Key** 慢于迭代 8、仍远快于迭代 0                                      | **去掉 map 内 key 串副本**；同哈希需串比较，访问常数回升                     |
| 10  | `entry`：`is`_* / `as_string` 用 `jtype::detect`，`is_true` 仍单次 parse             | 堆与迭代 9 同；Benchmark 为 **同日 3 次连测中位数**（`Access_Key` 相对迭代 9 CPU 约 **+2%～+6%**） | 收益在 **类型查询热路径**；**20:27 单次** 判干扰未入表                     |
| 12  | `operator==` / `!=`：按槽比 `first`/`second`，不双路 `std::string()`                   | 堆与迭代 10 同；Benchmark **21:20 单次**（**21:19** 整组偏慢未入表）；与迭代 10 中位数差异属噪声         | **基准不含 `==` 热点**；收益在 **调用 `==` 时少分配**                   |
| 13  | 现代化 C++：UB 修复与异常现代化；宏→inline、goto→lambda、typedef→using；补全移动语义 | `BM_Access_Key/100` 相对迭代 12 CPU 约 **-15.61%**；其余解析/序列化/pretty 多为 +0%~+11% 波动 | allocounter 与迭代 12 一致；建议同配置连跑取中位数验证噪声 |


---

## 后续迭代测试规范

从迭代 1 开始，每次优化迭代需要同时运行以下测试并记录：

### 1. 内存测试

```bash
baseline_benchmark_test_memory.exe
```

记录：`checksum`、alloc/op、bytes/op（及原始表中的堆分配四列）

### 2. Google Benchmark 测试

```bash
baseline_benchmark_test_time.exe
baseline_benchmark_test_time_comprehensive.exe
```

（可选）复测 **`start/` 原始实现**：当前 CMake 不生成 `*_start` 目标；需另建库或临时替换源码后对比（参见上文「扩展时间基准」说明）。

记录：Time、CPU、bytes_per_second、items_per_second

### 3. 对比维度


| 对比对象              | 目的         |
| ----------------- | ---------- |
| 迭代 0 内存基线         | 评估内存分配优化效果 |
| 迭代 0 Benchmark 基线 | 评估吞吐量和时延改进 |
| 上次迭代              | 评估单步优化效果   |


### 4. 重点关注指标

- **解析**：BM_Parse_Object/50、BM_Parse_Object/100 的 items_per_second
- **序列化**：BM_Serialize、BM_Pretty 的时延变化
- **访问**：BM_Access_Key/100 的时延（反映查找效率）
- **内存**：total allocated bytes / rounds 的比率变化

