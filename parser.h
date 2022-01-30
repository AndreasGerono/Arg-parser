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
    virtual int min() { return 0; };
    virtual int max() { return 0; };
    void validate(int naargs);
    string _help;
    string _naargs;
};

void Param_T::validate(int naargs)
{
    int _min = min();
    int _max = max();
    if (naargs < _min) {
        auto error = "Not enought parameters, expected: " + to_string(_min) + ", reveived: " + to_string(naargs);
        throw invalid_argument(error);
    }
    if (_max != -1 && naargs > _max) {
        auto error = "Too many parameters, expected: " + to_string(_min) + ", reveived: " + to_string(naargs);
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
    Param& storeTrue();

private:
    friend class Parser;
    void parse(Iterator begin, Iterator end) override;
    void call1() override { if(_cb1) _cb1(_args[0]); };
    void call2() override { if(_cb2) _cb2(_args); };
    int max() override;
    int min() override;
    vector<T> _args;
    CB1<T> _cb1;
    CB2<T> _cb2;
};

template <typename T>
int Param<T>::min()
{
    if (_naargs == "+") {
        return 1;
    }
    auto min = stoi(_naargs);
    return min;
}

template <typename T>
int Param<T>::max()
{
    if (_naargs[0] == '+') {
        return -1;
    }
    auto max = stoi(_naargs);
    return max;
}

template <>
Param<bool>& Param<bool>::storeTrue()
{
    _naargs = "0";
    return *this;
}

template <typename T>
void Param<T>::parse(Iterator begin, Iterator end)
{
    // use loop with new T;
    auto args = vector<string>(begin, end);
    cout<<endl<<"Param vars: ";

    if (args.empty()) {
        _args.push_back(true);
    }

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

// template <>
// void Param<bool>::parse(Iterator begin, Iterator end)
// {
//     if (vals.empty()) {
//         _args = true;
//         cout<<"parsed: "<<typeid(_args).name()<<" "<<_args<<endl;
//     } else {
//         cout<<"Too many arguments!!"<<endl;
//     }
// };

class Parser
{
private:
    map<string, Param_T*> params;
    vector<string> _argv;
public:
    Parser(int argc, char *argv[]);
    ~Parser() = default;
    void parse(const string);
    template <typename T>
    Param<T>& add_argument(string name, string naargs="1");
    template <typename T>
    Param<T>& add_argument(Names names, string naargs="1");
    string parse_args();
};

Parser::Parser(int argc, char *argv[])
{
    _argv.insert(_argv.begin(), argv, argv+argc);
}

template <typename T>
Param<T>& Parser::add_argument(string name, string naargs)
{
    auto* param = new Param<T>(naargs);
    params[name] = param;
    return *param;
}

template <typename T>
Param<T>& Parser::add_argument(vector<string> names, string naargs)
{
    auto* param = new Param<T>(naargs);
    for (auto &name: names) {
        params[name] = param;
    }
    return *param;
}


string Parser::parse_args()
{
    auto isParam = [&](auto i) { return params.count(i)>0; };
    string error;

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
    return error;
}
