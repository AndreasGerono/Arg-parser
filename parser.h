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
#include <unordered_set>

// TODO: read more about virtual inheritance!
// TODO: C++ forward? was is das?
// TODO:
//      - implement array and vector?
//      - implement positional arguments?
// change of tactics -> use normal type. return pointer -> naarg as argument -> optional -> default 1?

// TO test:
// ./test.exe 1 2 1 -pw 10 --tacho 10 20

using namespace std;
using Iterator = vector<string>::iterator;
using Arg = vector<string>;

class Param_T {
public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call1(void) {};
    virtual void call2(void) {};
    virtual void parse(Iterator begin, Iterator end) {};
    void validate(int naargs);
    string help(void);
    string name(void);
    vector<string> _names;  // To generate help
    string _help;
    int _min;
    int _max;
    int _no_calls;
};

void Param_T::validate(int naargs)
{
    if (naargs < _min) {
        auto error = "not enought parameters, expected: " + to_string(_min) + ", reveived: " + to_string(naargs);
        throw invalid_argument(error);
    }
    if (_max != -1 && naargs > _max) {
        auto error = "too many parameters, expected: " + to_string(_max) + ", reveived: " + to_string(naargs);
        throw invalid_argument(error);
    }
}

string Param_T::help(void)
{
    return "";
}

string Param_T::name(void)
{
    string name = "";
    for_each(_names.begin(), _names.end(), [&](auto &elem) {
        name += elem;
        if (elem != _names.back()) {
            name += ", ";
        }
    });
    return name;
}


template <typename T>
using CB1 = function<void(T)>;
template <typename T>
using CB2 = function<void(vector<T>)>;


template <typename T>
class Param: private virtual Param_T {
public:
    Param(vector<string> names) { _names = names; };
    Param& callback(CB1<T> cb1) { _cb1 = cb1; return *this; };
    Param& callback(CB2<T> cb2) { _cb2 = cb2; return *this; };
    Param& help(string help) { _help = help; return *this; };
    Param& store_true(void) { _min = 0; _max = 0; return *this; };
    Param& is_required(void) { _required = true; return *this; };

private:
    friend class ArgumentParser;
    void parse(Iterator begin, Iterator end) override;
    void call1(void) override { if(_cb1) _cb1(_args[0]); };
    void call2(void) override { if(_cb2) _cb2(_args); };
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
        cout<<endl<<"\tparsed: "<<typeid(tmp).name()<<" "<<tmp;
    }
    _no_calls += 1;
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

    void parse_pos_args(void);
    void parse_opt_args(void);
    map<string, Param_T*> params;
    vector<Param_T*> pos_params;
    vector<Param_T*> opt_params;
    vector<string> _argv;

public:
    ArgumentParser(int argc, char *argv[]);
    template <typename T=bool>
    Param<T>& add_argument(string name, string naargs="1");
    template <typename T=bool>
    Param<T>& add_argument(Arg names, string naargs="1");
    pair<int, int> parse_bounds(string naargs);
    Type parse_type(Arg names);
    void parse_args(void);
};

ArgumentParser::ArgumentParser(int argc, char *argv[])
{
    _argv.insert(_argv.begin(), argv+1, argv+argc);
}

template <typename T>
Param<T>& ArgumentParser::add_argument(string name, string naargs)
{
    auto names = Arg{name};
    return add_argument<T>(names, naargs);
}

template <typename T>
Param<T>& ArgumentParser::add_argument(vector<string> names, string naargs)
{
    auto* param = new Param<T>(names);
    tie(param->_min, param->_max) = parse_bounds(naargs);

    Type p_type = parse_type(names);
    if (p_type == POSITIONAL) {
        pos_params.push_back(param);
    } else {
        opt_params.push_back(param);
    }

    for (auto &name: names) {
        params[name] = param;
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


enum ArgumentParser::Type ArgumentParser::parse_type(Arg names)
{
    unordered_set<string> unique_names(names.begin(), names.end());
    if (names.size() != unique_names.size()) {
        auto error = "Error while adding: '" + names[0] + "': argument names must be unique!";
        throw invalid_argument(error);
    }

    auto any_optional = any_of(names.begin(), names.end(), [](string n) { return n[0] == '-'; } );
    if (any_optional) {
        auto all_optional = all_of(names.begin(), names.end(), [](string n) { return n[0] == '-'; } );
        if (all_optional) {
            return OPTIONAL;
        }
        auto error = "Error while adding: '" + names[0] + "': you can't mix optional and non optional argument names!";
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
    auto isParam = [&](auto i) { return (i[0] == '-' && params.count(i)>0); };

    auto pBeg = find_if(_argv.begin(), _argv.end(), isParam);
    while (pBeg != _argv.end())
    {
        cout<<"Beg: "<<*pBeg<<"; ";
        auto param = params[*pBeg];

        auto aBeg = next(pBeg);
        auto aEnd = find_if(aBeg, _argv.end(), isParam);        // pass after -/--
        auto naargs = distance(aBeg, aEnd);
        try {
            param->validate(naargs);
            param->parse(aBeg, aEnd);
        } catch(const exception& e) {
            auto error = "Error while parsing '" + *pBeg + "': " + e.what();
            throw_with_nested(invalid_argument(error));
        }
        pBeg = _argv.erase(pBeg, aEnd);
        cout<<endl<<"Left: ";
        for (auto a: _argv) {
            cout<<a<<" ";
        }
        cout<<endl;
    }
}

void ArgumentParser::parse_pos_args()
{
    cout<<"Positional args: ";
    for (auto &param: pos_params) {
        cout<<endl<<"pos_param"<<": "<<param->name()<<" ";
        auto aBeg = _argv.begin();
        auto aEnd = _argv.end();

        if (param->_max != -1) {
            aEnd = next(aBeg, param->_max);
            if (distance(aEnd, _argv.end()) < 0) {
                aEnd = _argv.end();
            }
        }
        auto naargs = distance(aBeg, aEnd);
        try {
            param->validate(naargs);
            param->parse(aBeg, aEnd);
        } catch(const exception& e) {
            auto error = "Error while parsing '" + param->name() + "': " + e.what();
            throw_with_nested(invalid_argument(error));
        }

        aBeg = _argv.erase(aBeg, aEnd);
    }
}
