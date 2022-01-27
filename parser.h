#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <tuple>
#include <array>
#include <string>

// TODO: read more about virtual inheritance!
// TODO: C++ std::forward? was is das?

class Param_T {
public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call() {};
    std::string _name1;
    std::string _name2;
    std::string _help;
};

template <typename T>
using CB = std::function<void(T)>;


template <typename T>
class Param: private virtual Param_T {
public:
    Param(T val, std::string name1, std::string name2)
        : _val(val)
        {
            _name1 = name1;
            _name2 = name2;
        }

    Param& help(std::string help) { _help = help; return *this; };
    Param& callback(CB<T> cb) { _cb = cb; return *this; };

private:
    friend class Parser;
    void call() override { if(_cb) _cb(_val); };
    T _val;
    CB<T> _cb;
};


class Parser
{
private:
    std::vector<Param_T*> params;
public:
    Parser() = default;
    ~Parser() = default;
    void parse(const std::string);
    template <typename T>
    Param<T>& add_argument(T val, std::string name1, std::string name2="");
    void parse_args();
};

template <typename T>
Param<T>& Parser::add_argument(T val, std::string name1, std::string name2)
{
    auto* param = new Param<T>(val, name1, name2);
    params.push_back(param);
    return *param;
}

void Parser::parse_args()
{
    for (auto param: params) {
        param->call();
        std::cout<<param->_help<<std::endl;
        std::cout<<param->_name1<<std::endl;
        std::cout<<param->_name2<<std::endl;
    }
}