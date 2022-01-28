#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <tuple>
#include <array>
#include <string>
#include <algorithm>
#include <numeric>

// TODO: read more about virtual inheritance!
// TODO: C++ std::forward? was is das?

class Param_T {
public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call() {};
    virtual void parse(std::vector<std::string> args, std::vector<std::string> params) {};
    bool isRequired();
    std::vector<std::string> names;
    std::string _help;
};

bool Param_T::isRequired()
{
    return !std::any_of(names.begin(), names.end(), [] (auto elem) {
        return elem[0] == '-';
    });
}

template <typename T>
using CB = std::function<void(T)>;


template <typename T>
class Param: private virtual Param_T {
public:
    Param(T val, std::string name1, std::string name2)
        : _val(val)
        {
            names.insert(names.end(), {name1, name2});
        }

    Param& help(std::string help) { _help = help; return *this; };
    Param& callback(CB<T> cb) { _cb = cb; return *this; };

private:
    friend class Parser;
    void call() override { if(_cb) _cb(_val); };
    void parse(std::vector<std::string> args, std::vector<std::string> params) override;
    T _val;
    CB<T> _cb;
};

template <typename T>
void Param<T>::parse(std::vector<std::string> args, std::vector<std::string> params)
{
    ;
};

class Parser
{
private:
    std::vector<Param_T*> params;
    std::vector<std::string> _argv;
    std::vector<std::string> all_names;
public:
    Parser(int argc, char *argv[]);
    ~Parser() = default;
    void parse(const std::string);
    template <typename T>
    Param<T>& add_argument(T val, std::string name1, std::string name2="");
    std::string parse_args();
};

Parser::Parser(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        auto arg = std::string(argv[i]);
        _argv.push_back(arg);
    }
}

template <typename T>
Param<T>& Parser::add_argument(T val, std::string name1, std::string name2)
{
    all_names.insert(all_names.end(), {name1, name2});
    auto* param = new Param<T>(val, name1, name2);
    params.push_back(param);
    return *param;
}

// Parsing algorithm:
// 1. Find and mark all the Params in args -> fail if required not found.
// How to mark it? With position? Or?


std::string Parser::parse_args()
{
    std::string error;
    for (auto param: params) {
        std::string name = param->names[0];
        // find start of the parameter
        auto bParam = std::find_first_of(_argv.begin(), _argv.end(), param->names.begin(), param->names.end());
        if (bParam != _argv.end()) {
            bParam = std::next(bParam, 1);
            // std::cout<<"FOUND: "<<name<<" "<<*bParam<<std::endl;
            // Find end of the parameter
            auto eParam = std::find_first_of(bParam, _argv.end(), all_names.begin(), all_names.end());
            std::cout<<"FOUND: "<<name<<" "<<std::accumulate(bParam, eParam, std::string())<<std::endl;
        } else if (param->isRequired()) {
            error += "Parser: positional argument '" + name + "' is required!" + '\n'; 
        }
    }
    return error;
}