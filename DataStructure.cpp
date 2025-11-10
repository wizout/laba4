#include "DataStructure.h"
#include <sstream>
#include <iostream>
#include <vector>

DataStructure::DataStructure(int m) : m_size(m), data(m, 0), mtxs(m) {}

DataStructure::~DataStructure() {}

int DataStructure::get(int index) {
    if (index < 0 || index >= m_size) {
        std::cerr << "Error: 'get' index out of bounds." << std::endl;
        return 0;
    }

    std::shared_lock<std::shared_mutex> lock(mtxs[index]);
    return data[index];
}

void DataStructure::set(int index, int value) {
    if (index < 0 || index >= m_size) {
        std::cerr << "Error: 'set' index out of bounds." << std::endl;
        return;
    }

    std::unique_lock<std::shared_mutex> lock(mtxs[index]);
    data[index] = value;
}

DataStructure::operator std::string() const {
    std::vector<std::shared_lock<std::shared_mutex>> locks;
    locks.reserve(m_size);

    for (int i = 0; i < m_size; ++i) {
        locks.emplace_back(mtxs[i]);
    }

    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < m_size; ++i) {
        oss << data[i];
        if (i < m_size - 1) {
            oss << ", ";
        }
    }
    oss << "]";
    return oss.str();
}