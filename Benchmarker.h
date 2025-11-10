#pragma once

#include "DataStructure.h"
#include <string>
#include <vector>
#include <map>

enum class OpType {
    READ,
    WRITE,
    STRING
};

struct Command {
    OpType type;
    int index = 0;
    int value = 0;
};

class Benchmarker {
public:
    Benchmarker();
    ~Benchmarker();

    void RunAllBenchmarks();

private:
    void generateOpFile(const std::string& filename,
        const std::map<std::string, double>& weights,
        int num_ops);

    std::vector<Command> loadCommands(const std::string& filename);

    static void worker(DataStructure& ds, const std::vector<Command>& commands);

    void runTest(const std::string& scenario_name);

    const int M = 3;
    const int NUM_OPS = 100000;
    const int MAX_THREADS = 3;


    std::map<std::string, double> weights_specified;
    std::map<std::string, double> weights_uniform;
    std::map<std::string, double> weights_worst;
    
    std::vector<std::string> scenarios;
};