// baseline_benchmark_test.cpp
// 内存实验 1～6（与你给的独立分析程序同逻辑）+ 与历史基线一致的终端输出格式，便于迭代前后对比重测。

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "json.h"
#include "allocounter.hpp"

/* 勿命名为 clock：与 <ctime> 的 clock() 冲突，MSVC 会报 C2365/C2653 */
using HighResClock = std::chrono::high_resolution_clock;

static std::string makeJsonObject(int fieldCount)
{
	std::string json = "{";
	for (int i = 0; i < fieldCount; ++i)
	{
		if (i > 0)
			json += ",";
		json += "\"long_field_name_" + std::to_string(i) + "\":\"long_value_data_" + std::to_string(i) + "\"";
	}
	json += "}";
	return json;
}

static std::string makeJsonArray(int elementCount)
{
	std::string json = "[";
	for (int i = 0; i < elementCount; ++i)
	{
		if (i > 0)
			json += ",";
		json += "\"long_item_value_" + std::to_string(i) + "\"";
	}
	json += "]";
	return json;
}

static void emit_result(const char* name, int rounds, HighResClock::time_point t1, HighResClock::time_point t2, std::size_t checksum)
{
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << "=== " << name << " ===\n";
	std::cout << "rounds: " << rounds << "\n";
	std::cout << "elapsed(us): " << us << "\n";
	std::cout << "avg(us/op): " << static_cast<double>(us) / rounds << "\n";
	std::cout << "checksum: " << checksum << "\n";
	AllocCounter::report();
	std::cout << "\n";
}

/** 每轮执行 work() 并累加返回值到 checksum。实验 1～4：work 内含 parse；实验 5～6：work 只含读字段或序列化（对象在外部已 parse）。 */
template <typename F>
static void run_benchmark(const char* name, int rounds, F&& work)
{
	for (int i = 0; i < 20; ++i)
		(void)work();

	AllocCounter::resetCounter();
	auto t1 = HighResClock::now();
	std::size_t checksum = 0;
	for (int i = 0; i < rounds; ++i)
		checksum += work();
	auto t2 = HighResClock::now();
	emit_result(name, rounds, t1, t2, checksum);
}

int main()
{
	const int rounds = 2000;

	// --- 实验 1：小 JSON，长 key 绕过 SSO，parse + 析构 ---
	{
		const std::string input =
		    R"({"name_of_student":"Tom_the_great_one","age_of_student":25})";
		run_benchmark(
		    "exp1-small-json-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input.c_str());
			    return obj.size();
		    });
	}

	// --- 实验 2：中等对象 10 字段 ---
	{
		const std::string input = makeJsonObject(10);
		run_benchmark(
		    "exp2-medium-10-fields-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input.c_str());
			    return obj.size();
		    });
	}

	// --- 实验 3：大对象 100 字段 ---
	{
		const std::string input = makeJsonObject(100);
		run_benchmark(
		    "exp3-large-100-fields-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input.c_str());
			    return obj.size();
		    });
	}

	// --- 实验 4：数组 100 个元素 ---
	{
		const std::string input = makeJsonArray(100);
		run_benchmark(
		    "exp4-array-100-elements-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input.c_str());
			    return obj.size();
		    });
	}

	// --- exp7～exp10：与 exp1～exp4 相同输入，但走 jobject::parse(const std::string&)（parse(input)）---
	// 用于对比「const char* 重载」与「string 重载」；当前实现 string 重载内部转 c_str()，堆应与 exp1～4 一致；
	// 若回退到按值 parse(std::string)，此组相对 exp1～4 会多出整串拷贝的分配。
	{
		const std::string input =
		    R"({"name_of_student":"Tom_the_great_one","age_of_student":25})";
		run_benchmark(
		    "exp7-stdstring-small-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input);
			    return obj.size();
		    });
	}
	{
		const std::string input = makeJsonObject(10);
		run_benchmark(
		    "exp8-stdstring-10-fields-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input);
			    return obj.size();
		    });
	}
	{
		const std::string input = makeJsonObject(100);
		run_benchmark(
		    "exp9-stdstring-100-fields-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input);
			    return obj.size();
		    });
	}
	{
		const std::string input = makeJsonArray(100);
		run_benchmark(
		    "exp10-stdstring-100-array-parse-destruct",
		    rounds,
		    [&input]
		    {
			    json::jobject obj = json::jobject::parse(input);
			    return obj.size();
		    });
	}

	// --- 实验 5：先 parse，再只测读取（堆统计不含 parse）---
	{
		const std::string input =
		    R"({"name_of_student":"Tom_the_great_one","age_of_student":25,"city_of_origin":"Beijing_China_Asia"})";
		json::jobject obj = json::jobject::parse(input.c_str());

		run_benchmark(
		    "exp5-read-string-field",
		    rounds,
		    [&obj]
		    {
			    std::string name = obj["name_of_student"];
			    return name.size();
		    });

		run_benchmark(
		    "exp5-read-int-field",
		    rounds,
		    [&obj]
		    {
			    int age = obj["age_of_student"];
			    return static_cast<std::size_t>(age);
		    });
	}

	// --- 实验 6：50 字段对象 parse 一次，再分别测 as_string / pretty ---
	{
		const std::string input = makeJsonObject(50);
		json::jobject obj = json::jobject::parse(input.c_str());

		run_benchmark(
		    "exp6-as_string-50-fields",
		    rounds,
		    [&obj]
		    {
			    std::string output = obj.as_string();
			    return output.size();
		    });

		run_benchmark(
		    "exp6-pretty-50-fields",
		    rounds,
		    [&obj]
		    {
			    std::string output = obj.pretty();
			    return output.size();
		    });
	}

	return 0;
}
