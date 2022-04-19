#ifndef PARAM_H
#define PARAM_H

#pragma once

#include <vector>
#include <sstream>
#include <functional>
#include <iostream>

using Iterator = std::vector<std::string>::iterator;
using Arg = std::vector<std::string>;

class Param_T {
   public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call1(void){};
    virtual void call2(void){};
    virtual void parse(Iterator begin, Iterator end){};
    void validate(int naargs);
    std::string name;
    std::string _help;
    bool _required;
    int _min;
    int _max;
    int _no_calls;
};

template <typename T>
using CB1 = std::function<void(T)>;
template <typename T>
using CB2 = std::function<void(std::vector<T>)>;

template <typename T>
class Param : private Param_T {
   public:
    Param(std::vector<std::string> names);
    Param& callback(CB1<T> cb1) {
        _cb1 = cb1;
        return *this;
    };
    Param& callback(CB2<T> cb2) {
        _cb2 = cb2;
        return *this;
    };
    Param& help(std::string help) {
        _help = help;
        return *this;
    };
    Param& store_true(void) {
        _min = 0;
        _max = 0;
        return *this;
    };
    Param& is_required(void) {
        _required = true;
        return *this;
    };

   private:
    friend class ArgumentParser;
    void parse(Iterator begin, Iterator end) override;
    void call1(void) override {
        if (_cb1) _cb1(_args[0]);
    };
    void call2(void) override {
        if (_cb2) _cb2(_args);
    };
    std::vector<T> _args;
    CB1<T> _cb1;
    CB2<T> _cb2;
    bool _required;
};

template <typename T>
Param<T>::Param(std::vector<std::string> names) {
    for_each(names.begin(), names.end(), [&](auto& elem) {
        name += elem;
        if (elem != names.back()) {
            name += ", ";
        }
    });
};

template <typename T>
void Param<T>::parse(Iterator begin, Iterator end) {
    auto args = std::vector<std::string>(begin, end);
    auto push_arg = [&](std::string arg) {
        T tmp;
        std::istringstream ss(arg);  // Conversion to T
        ss >> tmp;                   //
        _args.push_back(tmp);

#ifdef DEBUG
        std::cout << std::endl
                  << "\tparsed: " << typeid(tmp).name() << " " << tmp;
#endif

    };

#ifdef DEBUG
    std::cout << std::endl
              << "Param vars: ";
#endif

    for_each(args.begin(), args.end(), push_arg);

    if (args.empty()) {
        push_arg("1");
    }

    _no_calls += 1;

#ifdef DEBUG
    std::cout << "; ";
#endif

    call2();
    call1();
};

#endif