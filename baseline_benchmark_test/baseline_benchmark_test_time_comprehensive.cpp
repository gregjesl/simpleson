// 扩展时间基准：在 baseline_benchmark_test_time.cpp 之外补充 json.cpp / API 热路径，
// 覆盖混合标量、数值数组、std::string 解析、tryparse、键枚举、合并与结构比较等。
#include <benchmark/benchmark.h>
#include "json.h"
#include <string>
#include <vector>

static std::string MakeMixedObject()
{
    return R"({"s":"hello","n":42,"f":3.14159,"e":1.5e10,"b":true,"z":null,"sub":{"x":1}})";
}

static std::string MakeNumberArray(int n)
{
    std::string json = "[";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            json += ",";
        json += std::to_string(i);
    }
    json += "]";
    return json;
}

static std::string MakeObjectKeys(int n)
{
    std::string json = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            json += ",";
        json += "\"k" + std::to_string(i) + "\":" + std::to_string(i * 7);
    }
    json += "}";
    return json;
}

static std::string MakeObjectKeysPrefix(int n, const char *prefix)
{
    std::string json = "{";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            json += ",";
        json += "\"" + std::string(prefix) + std::to_string(i) + "\":" + std::to_string(i * 3);
    }
    json += "}";
    return json;
}

static std::string MakeEscapedStringJson()
{
    return R"({"t":"line1\nline2\t\"q\""})";
}

static std::string MakeRootArrayOfObjects(int n)
{
    std::string json = "[";
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            json += ",";
        json += "{\"id\":" + std::to_string(i) + ",\"name\":\"n" + std::to_string(i) + "\"}";
    }
    json += "]";
    return json;
}

// std::string 重载（与 c_str 路径一致，保留调用形态）
static void BM_Parse_StdString(benchmark::State &state)
{
    const std::string input = MakeMixedObject();
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input);
        benchmark::DoNotOptimize(obj);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_Parse_StdString);

static void BM_Parse_MixedScalars(benchmark::State &state)
{
    const std::string input = MakeMixedObject();
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input.c_str());
        benchmark::DoNotOptimize(obj);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_Parse_MixedScalars);

static void BM_Parse_NumberArray(benchmark::State &state)
{
    const int n = static_cast<int>(state.range(0));
    const std::string input = MakeNumberArray(n);
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input.c_str());
        benchmark::DoNotOptimize(obj);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_Parse_NumberArray)->Arg(50)->Arg(200)->Arg(1000);

static void BM_TryParse_Success(benchmark::State &state)
{
    const std::string input = MakeObjectKeys(20);
    for (auto _ : state)
    {
        json::jobject out;
        const bool ok = json::jobject::tryparse(input.c_str(), out);
        benchmark::DoNotOptimize(ok);
        benchmark::DoNotOptimize(out.size());
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_TryParse_Success);

static void BM_ReadNumericFields(benchmark::State &state)
{
    const std::string input = MakeMixedObject();
    auto obj = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        const int ni = obj["n"];
        const double df = obj["f"];
        benchmark::DoNotOptimize(ni);
        benchmark::DoNotOptimize(df);
    }
}
BENCHMARK(BM_ReadNumericFields);

static void BM_ReadBoolNullString(benchmark::State &state)
{
    const std::string input = MakeMixedObject();
    auto obj = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        const bool bt = obj["b"].is_true();
        const bool isnull = obj["z"].is_null();
        const std::string s = std::string(obj["s"]);
        benchmark::DoNotOptimize(bt);
        benchmark::DoNotOptimize(isnull);
        benchmark::DoNotOptimize(s.size());
    }
}
BENCHMARK(BM_ReadBoolNullString);

static void BM_ArrayOfObjects_Access(benchmark::State &state)
{
    const int n = static_cast<int>(state.range(0));
    const std::string input = MakeRootArrayOfObjects(n);
    auto root = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        const auto el = root.array(0);
        const int id = static_cast<int>(el.get("id"));
        benchmark::DoNotOptimize(id);
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_ArrayOfObjects_Access)->Arg(10)->Arg(50);

static void BM_ListKeys_HasKey(benchmark::State &state)
{
    const int n = static_cast<int>(state.range(0));
    const std::string input = MakeObjectKeys(n);
    auto obj = json::jobject::parse(input.c_str());
    const std::string mid = "k" + std::to_string(n / 2);
    for (auto _ : state)
    {
        const auto keys = obj.list_keys();
        const bool hk = obj.has_key(mid);
        benchmark::DoNotOptimize(keys.size());
        benchmark::DoNotOptimize(hk);
    }
}
BENCHMARK(BM_ListKeys_HasKey)->Arg(20)->Arg(100);

static void BM_Merge_PlusAssign(benchmark::State &state)
{
    const std::string a = MakeObjectKeysPrefix(15, "a");
    const std::string b = MakeObjectKeysPrefix(15, "b");
    auto ja = json::jobject::parse(a.c_str());
    const auto jb = json::jobject::parse(b.c_str());
    for (auto _ : state)
    {
        json::jobject acc = ja;
        acc += jb;
        benchmark::DoNotOptimize(acc.size());
    }
}
BENCHMARK(BM_Merge_PlusAssign);

static void BM_Serialize_Mixed(benchmark::State &state)
{
    const std::string input = MakeMixedObject();
    auto obj = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        std::string out = obj.as_string();
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(BM_Serialize_Mixed);

static void BM_Pretty_Mixed(benchmark::State &state)
{
    const std::string input = MakeMixedObject();
    auto obj = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        std::string out = obj.pretty();
        benchmark::DoNotOptimize(out);
    }
}
BENCHMARK(BM_Pretty_Mixed);

static void BM_Parse_EscapedStrings(benchmark::State &state)
{
    const std::string input = MakeEscapedStringJson();
    for (auto _ : state)
    {
        auto obj = json::jobject::parse(input.c_str());
        const std::string t = std::string(obj["t"]);
        benchmark::DoNotOptimize(t.size());
    }
    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(input.size()));
}
BENCHMARK(BM_Parse_EscapedStrings);

static void BM_StructuralEquals(benchmark::State &state)
{
    const std::string input = MakeObjectKeys(40);
    const auto a = json::jobject::parse(input.c_str());
    const auto b = json::jobject::parse(input.c_str());
    for (auto _ : state)
    {
        const bool eq = (a == b);
        benchmark::DoNotOptimize(eq);
    }
}
BENCHMARK(BM_StructuralEquals);
