#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "param.h"

class ArgumentParser {
   private:
    enum Type {
        POSITIONAL,
        OPTIONAL,
    };

    void parse_pos_args(void);
    void parse_opt_args(void);
    Type parse_type(Arg names);
    std::pair<int, int> parse_bounds(std::string naargs);
    std::unordered_map<std::string, Param_T*> params;
    std::vector<Param_T*> pos_params;
    std::vector<Param_T*> opt_params;
    std::vector<std::string> _argv;

   public:
    ArgumentParser(int argc, char* argv[]);
    ~ArgumentParser();
    template <typename T = bool>
    Param<T>& add_argument(std::string name, std::string naargs = "1");
    template <typename T = bool>
    Param<T>& add_argument(Arg names, std::string naargs = "1");
    template <typename T>
    const T get(std::string arg, size_t idx = 0);
    template <typename T>
    const std::vector<T> getV(std::string arg);
    void parse_args(void);
    void print_help(void);
};


// Template functions needs to be in .h file

template <typename T>
const T ArgumentParser::get(std::string arg, size_t idx) {
    auto result = getV<T>(arg);
    if (idx + 1 > result.size()) {
        return T();  // always return default
    }
    return result[idx];
}

template <typename T>
const std::vector<T> ArgumentParser::getV(std::string arg) {
    auto it = params.find(arg);
    if (it != params.end()) {
        auto tmp = static_cast<Param<T>*>(it->second);
        return tmp->_args;
    }
    auto error = "Error while accessing '" + arg + "': argument not known!";
    throw std::invalid_argument(error);
}

template <typename T>
Param<T>& ArgumentParser::add_argument(std::string name, std::string naargs) {
    auto names = Arg{name};
    return add_argument<T>(names, naargs);
}

template <typename T>
Param<T>& ArgumentParser::add_argument(std::vector<std::string> names, std::string naargs) {
    auto* param = new Param<T>(names);
    std::tie(param->_min, param->_max) = parse_bounds(naargs);

    Type p_type = parse_type(names);
    if (p_type == POSITIONAL) {
        pos_params.push_back(param);
    } else {
        opt_params.push_back(param);
    }

    for (auto& name : names) {
        params[name] = param;
    }
    return *param;
}

#endif