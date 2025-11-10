#include "Benchmarker.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>
#include <functional>

Benchmarker::Benchmarker() {
    weights_specified = {
        {"read 0", 0.10},
        {"write 0", 0.05},
        {"read 1", 0.10},
        {"write 1", 0.05},
        {"read 2", 0.10},
        {"write 2", 0.20},
        {"string", 0.40}
    };

    double equal_weight = 1.0 / 7.0;
    weights_uniform = {
        {"read 0", equal_weight},
        {"write 0", equal_weight},
        {"read 1", equal_weight},
        {"write 1", equal_weight},
        {"read 2", equal_weight},
        {"write 2", equal_weight},
        {"string", equal_weight}
    };

    weights_worst = {
        {"read 0", 0.0},
        {"write 0", 0.35},
        {"read 1", 0.0},
        {"write 1", 0.25},
        {"read 2", 0.0},
        {"write 2", 0.0},
        {"string", 0.70}
    };

    scenarios = { "SpecifiedDistribution", "UniformDistribution", "Worst Case" };
}

Benchmarker::~Benchmarker() {}

void Benchmarker::RunAllBenchmarks() {
    std::map<std::string, std::map<std::string, double>> all_weights = {
        {scenarios[0], weights_specified},
        {scenarios[1], weights_uniform},
        {scenarios[2], weights_worst}
    };

    for (const auto& scenario : scenarios) {
        for (int i = 1; i <= MAX_THREADS; ++i) {
            std::string filename = scenario + "_" + std::to_string(i) + ".txt";
            generateOpFile(filename, all_weights[scenario], NUM_OPS);
        }
    }

    std::cout << "\nStarting performance tests..." << std::endl;

    for (const auto& scenario : scenarios) {
        runTest(scenario);
    }

    std::cout << "\n...Test run complete." << std::endl;
}

void Benchmarker::runTest(const std::string& scenario_name) {
    std::cout << "\nTesting Scenario: [" << scenario_name << "]" << std::endl;

    for (int num_threads = 1; num_threads <= MAX_THREADS; ++num_threads) {

        DataStructure ds(M);

        std::vector<std::vector<Command>> all_commands(num_threads);
        for (int i = 0; i < num_threads; ++i) {
            std::string filename = scenario_name + "_" + std::to_string(i + 1) + ".txt";
            all_commands[i] = loadCommands(filename);

            if (all_commands[i].empty()) {
                std::cerr << "Fatal Error: Failed to load commands for " << filename << std::endl;
                return;
            }
        }

        std::vector<std::thread> threads;

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(
                worker,
                std::ref(ds),
                std::cref(all_commands[i])
            );
        }

        for (auto& t : threads) {
            t.join();
        }

        auto end_time = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> duration_sec = end_time - start_time;

        std::cout << std::fixed << std::setprecision(4);
        std::cout << "Test (" << num_threads << " threads): " << duration_sec.count() << " seconds" << std::endl;
    }
}

void Benchmarker::generateOpFile(const std::string& filename,
    const std::map<std::string, double>& weights,
    int num_ops)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "error opening file for writing - " << filename << std::endl;
        return;
    }

    std::vector<std::string> op_names;
    std::vector<double> op_weights;
    for (const auto& pair : weights) {
        op_names.push_back(pair.first);
        op_weights.push_back(pair.second);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> dist(op_weights.begin(), op_weights.end());

    for (int i = 0; i < num_ops; ++i) {
        std::string op = op_names[dist(gen)];

        if (op == "string") {
            file << "string\n";
        }
        else if (op.rfind("read", 0) == 0) {
            file << op << "\n";
        }
        else if (op.rfind("write", 0) == 0) {
            file << op << " 1\n";
        }
    }
    file.close();
}

std::vector<Command> Benchmarker::loadCommands(const std::string& filename) {
    std::vector<Command> commands;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "error opening file for reading: " << filename << std::endl;
        return commands;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string op_str;
        iss >> op_str;

        if (op_str == "read") {
            int index;
            iss >> index;
            commands.push_back({ OpType::READ, index, 0 });
        }
        else if (op_str == "write") {
            int index, value;
            iss >> index >> value;
            commands.push_back({ OpType::WRITE, index, value });
        }
        else if (op_str == "string") {
            commands.push_back({ OpType::STRING, 0, 0 });
        }
    }
    return commands;
}

void Benchmarker::worker(DataStructure& ds, const std::vector<Command>& commands) {
    std::ostringstream sink;

    for (const auto& cmd : commands) {
        switch (cmd.type) {
        case OpType::READ:
            sink << ds.get(cmd.index);
            break;
        case OpType::WRITE:
            ds.set(cmd.index, cmd.value);
            break;
        case OpType::STRING:
            sink << (std::string)ds;
            break;
        }
    }
}