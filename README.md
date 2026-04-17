# simpleson

轻量级 C++ JSON 解析与序列化库，基于 [gregjesl/simpleson](https://github.com/gregjesl/simpleson) 的 **C++17 现代化与性能改造**版本，除 Google Test / Google Benchmark 等**测试依赖**外，库本身无第三方运行时依赖。

## 文档导航

| 文档 | 内容 |
| ---- | ---- |
| `现代C++改造记录.md` | 按迭代记录语法、安全与工程化改动 |
| `任务二.md` | 性能优化压缩报告： |
| `baseline_benchmark_test/BASELINE_RESULTS.md` | 完整内存与 Google Benchmark 原始数据、迭代对照 |

## 项目概述

在**保持 JSON 语义与对外用法基本一致**的前提下，本仓库对 simpleson 做了：

- **正确性与 UB 修复**（如 `std::isspace`、异常规范、`tryparse` 捕获范围等）
- **性能**：序列化预分配、数字转字符串、对象模式 **FNV-1a + `unordered_multimap` 键索引** 等
- **工程质量**：单测、`operator==` 槽位比较、宏改内联、移动语义等

## 改造亮点

- **C++17**：`noexcept`、`override`、`using` 别名、`static_cast`、Rule of Five 等
- **安全**：消除部分未定义行为，收紧异常处理
- **性能**：大对象解析、序列化、Pretty、`Access_Key` 等见下表与 `任务二.md`
- **测试**：Google Test 语义测试；OpenCppCoverage 下 `json.cpp` / `json.h` 行覆盖率约 **91.6%**（以 `jobject_semantics_test/coverage_final` 报告为准）

## 性能结果（详细结果见任务二.md）

| 优化项 | 优化前（迭代 0） | 优化后（迭代 13） | 提升倍数 |
| ------ | ---------------- | ----------------- | -------- |
| 解析大对象 `BM_Parse_Object/500` | 524,576 ns | 180,664 ns | **2.90x** |
| 序列化 `BM_Serialize/100` | 8,161 ns | 2,400 ns | **3.40x** |
| Key 查找 `BM_Access_Key/100` | 854 ns | 79.5 ns | **10.74x** |
| Pretty `BM_Pretty/100` | 3,223 ns | 2,459 ns | **1.31x** |
| 解析大数组 `BM_Parse_Array/1000` | 149,613 ns | 139,509 ns | **1.07x** |
| 结构相等 `BM_StructuralEquals` ※ | 4,349 ns | 246 ns | **17.7x** |
| 混合对象序列化 `BM_Serialize_Mixed` ※ | 414 ns | 157 ns | **2.64x** |
| 布尔 / null / 字符串读 `BM_ReadBoolNullString` ※ | 230 ns | 199 ns | **1.16x** |
| 枚举键 + `has_key` `BM_ListKeys_HasKey/100` ※ | 2,131 ns | 1,904 ns | **1.12x** |

## 现代 C++ 改造

| 类别 | 示例 |
| ---- | ---- |
| 安全 / 规范 | `isspace` 用 `unsigned char`、`noexcept`、`catch (const std::exception&)` |
| 风格 | `typedef` → `using`，宏 → `inline` 函数，去掉 `goto`（如 `push_array`） |
| 资源 | `sub_reader` 智能指针、`nullptr` |
| API | `parse(const std::string&)` 为 `const` 引用等 |

完整条目见 **`现代C++改造记录.md`**。

## 单元测试

- **框架**：Google Test（`find_package(GTest CONFIG REQUIRED)`）
- **目标**：`jobject_semantics_test`（`test/jobject_semantics_test.cpp`，大量用例覆盖 `json.h` / `json.cpp`）
- **运行**：构建后执行可执行文件，或使用 `ctest`（已 `gtest_discover_tests`）

```bash
cmake -B build -S .
cmake --build build --config Release --target jobject_semantics_test
ctest --test-dir build -C Release -R jobject_semantics_test
```

Windows 下测试程序目录需能加载 **`simpleson.dll`**（CMake 已在 POST_BUILD 复制）。

### 覆盖率（OpenCppCoverage）

需 **带 PDB 的构建**（一般用 **Debug**，或为 Release 开启程序数据库）。示例（按本机路径改写 `--modules` 与 exe）：

```powershell
opencppcoverage --modules "D:\path\to\build\Release" `
  --sources "D:\path\to\homework\simpleson" `
  --export_type html:"jobject_semantics_test\coverage_out" `
  -- "D:\path\to\build\test\Release\jobject_semantics_test.exe"
```

历史报告可参考目录：`jobject_semantics_test/coverage_final/`。

## 基准测试

| 目标 | 作用 | 主要依赖 |
| ---- | ---- | -------- |
| `baseline_benchmark_test_time` | 解析 / 序列化 / Pretty / `Access_Key` 等经典 `BM_*` | `benchmark::benchmark` |
| `baseline_benchmark_test_time_comprehensive` | 混合标量、`tryparse`、合并、`operator==` 等扩展场景 | 同上 |
| `baseline_benchmark_test_memory` | exp1～exp10 等堆分配与 checksum | **`allocounter`** 目标（见下） |

```bash
cmake --build build --config Release --target baseline_benchmark_test_time
cmake --build build --config Release --target baseline_benchmark_test_time_comprehensive
./build/baseline_benchmark_test/baseline_benchmark_test_time
```

**`allocounter`**：内存基准链接名为 `allocounter` 的 CMake 接口库。若单独配置本目录导致找不到该目标，需在**父工程**中 `add_subdirectory` 到提供 `allocounter.hpp` 的工程（例如课程仓库中的 `week02/practice/allocounter`），或自行提供同名 `IMPORTED` / `INTERFACE` 目标。

详细数字与迭代对比见 **`baseline_benchmark_test/BASELINE_RESULTS.md`**。

## 构建要求

- **C++17**
- **CMake 3.8+**
- **Google Test**：配置并运行 `jobject_semantics_test` 时需要（`GTest_DIR` / `CMAKE_PREFIX_PATH` 等）
- **Google Benchmark**：配置并运行时间/扩展基准时需要
- **Windows**：生成 **`simpleson` 共享库**（`simpleson.dll`），测试与基准通过 POST_BUILD 复制到可执行文件目录

示例（vcpkg 等已安装 GTest/Benchmark 时）：

```powershell
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/vcpkg/installed/x64-windows"
cmake --build build --config Release
```

**嵌入式**：保留上游 ESP-IDF 分支逻辑，使用 `ESP_PLATFORM` 时走独立工程路径（见根目录 `CMakeLists.txt`）。

## 快速开始

```cpp
#include "json.h"
#include <iostream>

int main() {
    std::string input = R"({"name": "Alice", "age": 30, "active": true})";
    json::jobject person = json::jobject::parse(input);

    std::string name = person["name"];
    int age = person["age"];

    if (person["active"].is_true()) {
        std::cout << name << " is active\n";
    }

    json::jobject address;
    address["city"] = "New York";
    address["zip"] = 10001;
    person["address"] = address;

    std::cout << person.as_string() << "\n";
    return 0;
}
```

### 数组与嵌套

`proxy` 上**没有**连续的 `operator[]`；嵌套对象需先转为 `jobject`，例如：

```cpp
json::jobject o = json::jobject::parse(R"({"nested":{"x":"1"}})");
std::string x = std::string(o["nested"].as_object()["x"]);  // "1"

json::jobject arr = json::jobject::parse(R"([{"id":1},{"id":2}])");
int id = static_cast<int>(arr.array(0).get("id"));
```

### 布尔与 null

对 `proxy` 赋布尔值请使用 **`set_boolean(true/false)`**；置空使用 **`set_null()`**。

## API 参考（节选）

| 方法 | 说明 |
| ---- | ---- |
| `static jobject parse(const char* / const std::string&)` | 解析 JSON 文本 |
| `static bool tryparse(const char*, jobject&)` | 解析失败返回 `false` |
| `bool has_key(const std::string&)` / `list_keys()` | 对象模式 |
| `std::string get(key)` / `get(index)` | 取序列化片段 |
| `const_value array(size_t)` | 数组元素（根为数组时） |
| `std::string as_string()` / `pretty()` | 紧凑 / 缩进序列化 |
| `bool operator==(const jobject&)` | 结构相等（槽位比较） |

`entry` / `proxy` 支持到数值类型、`std::string`、`jobject` 及若干 `std::vector<T>` 的转换；细节见 **`json.h`** 注释。

## 项目结构

```
simpleson/
├── json.h / json.cpp              # 改造版库源码（CMake 目标 simpleson）
├── start/json.h / start/json.cpp # 原始参考实现（默认不参与构建）
├── CMakeLists.txt
├── README.md
├── 现代C++改造记录.md
├── 任务二.md
├── test/
│   ├── jobject_semantics_test.cpp
│   └── CMakeLists.txt
├── jobject_semantics_test/       # 覆盖率输出等（如 coverage_final）
└── baseline_benchmark_test/
    ├── baseline_benchmark_test_time.cpp
    ├── baseline_benchmark_test_time_comprehensive.cpp
    ├── baseline_benchmark_test_memory.cpp
    └── BASELINE_RESULTS.md
```

## 许可

沿用上游 simpleson 的许可。
