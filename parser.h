#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <tuple>
#include <array>
#include <string>
#include <algorithm>
#include <numeric>
#include <map>

// TODO: read more about virtual inheritance!
// TODO: C++ std::forward? was is das?

class Param_T {
public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call() {};
    virtual void parse(std::vector<std::string> args, std::vector<std::string> params) {};
    bool isRequired();
    std::string _help;
};

bool Param_T::isRequired()
{
    return true;
}

template <typename T>
using CB = std::function<void(T)>;


template <typename T>
class Param: private virtual Param_T {
public:
    Param(T val): _val(val) {};
    Param& help(std::string help) { _help = help; return *this; };
    Param& callback(CB<T> cb) { _cb = cb; return *this; };

private:
    friend class Parser;
    void call() override { std::cout<<_val<<std::endl; if(_cb) _cb(_val); };
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
    std::map<std::string, Param_T*> params;
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
    auto* param = new Param<T>(val);
    params[name1] = param;
    if (!name2.empty()) {
        params[name2] = param;
    }
    return *param;
}

// Parsing algorithm:
// 1. Find and mark all the Params in args -> fail if required not found.
// How to mark it? With position? Or?


std::string Parser::parse_args()
{
    auto isParam = [&](auto i) { return params.count(i)>0; };
    std::string error;

    auto pBegin = std::find_if(_argv.begin(), _argv.end(), isParam);
    while (pBegin != _argv.end())
    {
        std::cout<<*pBegin<<" ";
        auto p = params[*pBegin];
        pBegin += 1;
        auto pEnd = std::find_if(pBegin, _argv.end(), isParam);
        std::cout<<*pBegin<<" "<<*pEnd<<": ";
        p->call();
        pBegin = pEnd;
    }
    return error;
}