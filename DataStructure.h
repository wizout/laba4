#pragma once

#include <vector>
#include <string>
#include <shared_mutex>

class DataStructure {
public:
    DataStructure(int m);
    ~DataStructure();

    int get(int index);
    void set(int index, int value);
    operator std::string() const;

private:
    int m_size;
    std::vector<int> data;
    mutable std::vector<std::shared_mutex> mtxs;
};