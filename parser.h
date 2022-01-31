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
#include <sstream>
#include <typeinfo>
#include <stdexcept>
#include <utility>

// TODO: read more about virtual inheritance!
// TODO: C++ forward? was is das?
// TODO:
//      - implement array and vector?
//      - implement positional arguments?
// change of tactics -> use normal type. return pointer -> naarg as argument -> optional -> default 1?

// TO test:
// ./test.exe -pw 10 --tacho 10 20

using namespace std;
using Iterator = vector<string>::iterator;
using Names = vector<string>;

class Param_T {
public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call1() {};
    virtual void call2() {};
    virtual void parse(Iterator begin, Iterator end) {};
    void validate(int naargs);
    string _help;
    string _naargs;
    int _min;
    int _max;
};

void Param_T::validate(int naargs)
{
    if (naargs < _min) {
        auto error = "Not enought parameters, expected: " + to_string(_min) + ", reveived: " + to_string(naargs);
        throw invalid_argument(error);
    }
    if (_max != -1 && naargs > _max) {
        auto error = "Too many parameters, expected: " + to_string(_max) + ", reveived: " + to_string(naargs);
        throw invalid_argument(error);
    }
}


template <typename T>
using CB1 = function<void(T)>;
template <typename T>
using CB2 = function<void(vector<T>)>;


template <typename T>
class Param: private virtual Param_T {
public:
    Param(string naargs) { _naargs = naargs; };
    Param& callback(CB1<T> cb1) { _cb1 = cb1; return *this; };
    Param& callback(CB2<T> cb2) { _cb2 = cb2; return *this; };
    Param& help(string help) { _help = help; return *this; };
    Param& store_true() { _min = 0; _max = 0; return *this; };
    Param& is_required() { _required = true; return *this; };

private:
    friend class ArgumentParser;
    void parse(Iterator begin, Iterator end) override;
    void call1() override { if(_cb1) _cb1(_args[0]); };
    void call2() override { if(_cb2) _cb2(_args); };
    vector<T> _args;
    CB1<T> _cb1;
    CB2<T> _cb2;
    bool _required;
};


template <typename T>
void Param<T>::parse(Iterator begin, Iterator end)
{
    auto args = vector<string>(begin, end);
    cout<<endl<<"Param vars: ";

    for (auto arg: args) {
        T tmp;

        istringstream ss(arg);  // Conversion to T
        ss>>tmp;                //

        _args.push_back(tmp);
        cout<<"parsed: "<<typeid(tmp).name()<<" "<<tmp;
    }

    cout<<"; ";
    call2();
    call1();
};


class ArgumentParser
{
private:
    enum Type {
        POSITIONAL,
        OPTIONAL,
    };
    void parse_pos_args();
    void parse_opt_args();
    map<string, Param_T*> params;
    map<string, Param_T*> pos_params;
    vector<string> _argv;
public:
    ArgumentParser(int argc, char *argv[]);
    template <typename T=bool>
    Param<T>& add_argument(string name, string naargs="1");
    template <typename T=bool>
    Param<T>& add_argument(Names names, string naargs="1");
    pair<int, int> parse_bounds(string naargs);
    Type parse_type(Names names);
    void parse_args();
};

ArgumentParser::ArgumentParser(int argc, char *argv[])
{
    _argv.insert(_argv.begin(), argv+1, argv+argc);
}

template <typename T>
Param<T>& ArgumentParser::add_argument(string name, string naargs)
{
    auto names = Names{name};
    return add_argument<T>(names, naargs);
}

template <typename T>
Param<T>& ArgumentParser::add_argument(vector<string> names, string naargs)
{
    auto* param = new Param<T>(naargs);
    Type p_type = parse_type(names);
    tie(param->_min, param->_max) = parse_bounds(naargs);

    for (auto &name: names) {
        switch (p_type) {
        case OPTIONAL:
            params[name] = param;
            break;
        case POSITIONAL:
            pos_params[name] = param;
            break;
        }
    }
    return *param;
}

pair<int, int> ArgumentParser::parse_bounds(string naargs)
{
    if (naargs == "+") {
        return make_pair(1, -1);
    }

    auto min = stoi(naargs);
    if (naargs[0] == '+') {
        return make_pair(min, -1);
    }
    auto max = stoi(naargs);
    return make_pair(min, max);
}


enum ArgumentParser::Type ArgumentParser::parse_type(Names names)
{
    // conditions:
    // if one starts with - -> all starts with - and vice versa.
    auto any_optional = any_of(names.begin(), names.end(), [](string n) { return n[0] == '-'; } );
    if (any_optional) {
        auto all_optional = all_of(names.begin(), names.end(), [](string n) { return n[0] == '-'; } );
        if (all_optional) {
            return OPTIONAL;
        }
        auto error = "You can't mix optional and non optional argument names!";
        throw invalid_argument(error);
    }
    return POSITIONAL;
}

void ArgumentParser::parse_args()
{
    parse_opt_args();
    parse_pos_args();
}

void ArgumentParser::parse_opt_args()
{
    auto isParam = [&](auto i) { return params.count(i)>0; };

    auto pBeg = find_if(_argv.begin(), _argv.end(), isParam);
    while (pBeg != _argv.end())
    {
        cout<<"Beg: "<<*pBeg<<"; ";
        auto p = params[*pBeg];

        auto aBeg = next(pBeg);
        auto aEnd = find_if(aBeg, _argv.end(), isParam);  // pass after -/--
        auto naargs = distance(aBeg, aEnd);
        p->validate(naargs);
        p->parse(aBeg, aEnd);                              // pass after -/--

        pBeg = _argv.erase(pBeg, aEnd);                    // remove and advence
        cout<<"New beg: "<<*pBeg<<", "<<(pBeg == _argv.end())<<", ";
        for (auto a: _argv) {
            cout<<a<<" ";
        }
        cout<<endl;
    }
}

void ArgumentParser::parse_pos_args()
{
    cout<<"Positional args: ";
    for (auto &pos_param: pos_params) {
        cout<<endl<<"pos_param"<<*_argv.begin()<<": "<<pos_param.first<<" ";
        auto aBeg = _argv.begin();
        auto aEnd = _argv.end();
        if (pos_param.second->_max != -1) {
            aEnd = next(aBeg, pos_param.second->_max);
            if (distance(aEnd, _argv.end()) < 0) {
                aEnd = _argv.end();
            }
        }
        auto naargs = distance(aBeg, aEnd);
        pos_param.second->validate(naargs);
        pos_param.second->parse(aBeg, aEnd);
        aBeg = _argv.erase(aBeg, aEnd);
        cout<<pos_param.first<<", ";
    }
}
