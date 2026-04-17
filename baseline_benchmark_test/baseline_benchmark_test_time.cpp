// ============================================================
// Step 6: JSON 解析性能基准测试
// 使用 Google Benchmark 框架
// ============================================================
#include <benchmark/benchmark.h>
#include "json.h"
#include <string>
#include <sstream>

// ==================== 测试数据生成器 ====================

// 小 JSON：{"name":"Tom","age":25}
static std::string MakeSmallJson()
{
    return R"({"name":"Tom","age":25,"active":true})";
}

// 中等 JSON：N 个字段
static std::string MakeObject(int n)
{
    std::string json = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            json += ",";
        json += "\"key_" + std::to_string(i) + "\":\"value_" + std::to_string(i) + "\"";
    }
    json += "}";
    return json;
}

// 大数组：N 个字符串元素
static std::string MakeArray(int n)
{
    std::string json = "[";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            json += ",";
        json += "\"item_" + std::to_string(i) + "\"";
    }
    json += "]";
    return json;
}

// 嵌套对象
static std::string MakeNested(int depth)
{
    std::string prefix, suffix;
    for (int i = 0; i < depth; ++i)
    {
        prefix += "{\"l" + std::to_string(i) + "\":";
        suffix = "}" + suffix;
    }
    return prefix + "42" + suffix;
}

// ==================== 解析基准测试 ====================

// 测试 1: 解析小 JSON
static void BM_Parse_Small(benchmark::State &state)
{
    std::string input = MakeSmallJson();
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input.c_str());
        benchmark::DoNotOptimize(obj);
    }
    // 报告吞吐量（每秒处理的字节数）
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_Parse_Small);

// 测试 2: 解析不同大小的 JSON 对象
static void BM_Parse_Object(benchmark::State &state)
{
    int n = static_cast<int>(state.range(0));
    std::string input = MakeObject(n);
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input.c_str());
        benchmark::DoNotOptimize(obj);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
    state.SetItemsProcessed(
        static_cast<int64_t>(state.iterations()) * n);
}
// 参数：字段数 = 5, 10, 50, 100, 500
BENCHMARK(BM_Parse_Object)->Arg(5)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

// 测试 3: 解析数组
static void BM_Parse_Array(benchmark::State &state)
{
    int n = static_cast<int>(state.range(0));
    std::string input = MakeArray(n);
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input.c_str());
        benchmark::DoNotOptimize(obj);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_Parse_Array)->Arg(10)->Arg(100)->Arg(1000);

// 测试 4: 解析嵌套对象
static void BM_Parse_Nested(benchmark::State &state)
{
    int depth = static_cast<int>(state.range(0));
    std::string input = MakeNested(depth);
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input.c_str());
        benchmark::DoNotOptimize(obj);
    }
}
BENCHMARK(BM_Parse_Nested)->Arg(2)->Arg(5)->Arg(10)->Arg(20);

// ==================== 序列化基准测试 ====================

// 测试 5: 序列化
static void BM_Serialize(benchmark::State &state)
{
    int n = static_cast<int>(state.range(0));
    std::string input = MakeObject(n);
    auto obj = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        std::string result = obj.as_string();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Serialize)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

// 测试 6: Pretty 序列化
static void BM_Pretty(benchmark::State &state)
{
    int n = static_cast<int>(state.range(0));
    std::string input = MakeObject(n);
    auto obj = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        std::string result = obj.pretty();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Pretty)->Arg(10)->Arg(50)->Arg(100);

// ==================== 值访问基准测试 ====================

// 测试 7: 通过 key 访问值（最坏情况：访问最后一个 key）
static void BM_Access_Key(benchmark::State &state)
{
    int n = static_cast<int>(state.range(0));
    std::string input = MakeObject(n);
    auto obj = json::jobject::parse(input.c_str());
    std::string lastKey = "key_" + std::to_string(n - 1);
    for (auto _ : state)
    {
        std::string val = obj[lastKey];
        benchmark::DoNotOptimize(val);
    }
}
BENCHMARK(BM_Access_Key)->Arg(5)->Arg(10)->Arg(50)->Arg(100)->Arg(500);

// ==================== 不需要写 main，benchmark::benchmark_main 会提供 ====================