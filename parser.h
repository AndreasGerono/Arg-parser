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

// TODO: read more about virtual inheritance!
// TODO: C++ forward? was is das?
// TODO:
//      - implement array and vector?
//      - implement positional arguments?

using namespace std;
using Iterator = vector<string>::iterator;

class Param_T {
public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call() {};
    virtual Iterator parse(Iterator begin, Iterator end) {};
    string _help;
};


template <typename T>
using CB = function<void(T)>;


template <typename T>
class Param: private virtual Param_T {
public:
    Param& help(string help) { _help = help; return *this; };
    Param& callback(CB<T> cb) { _cb = cb; return *this; };

private:
    friend class Parser;
    void call() override { if(_cb) _cb(_val); };
    Iterator parse(Iterator begin, Iterator end) override;
    T _val;
    CB<T> _cb;
};

template <typename T>
Iterator Param<T>::parse(Iterator begin, Iterator end)
{
    auto vals = vector<string>(begin, end);
    cout<<"param vars: ";
    for (auto a: vals) {
        cout<<a<<" ";
    }
    
    if (vals.size() == 1) {
        auto a = vals.at(0);
        istringstream ss(a);
        ss>>_val;
        cout<<"parsed: "<<typeid(_val).name()<<" "<<_val<<endl;
        call();
    } else if (vals.size() > 1) {
        cout<<"Too many arguments expected one!!"<<endl;
    } else {
        cout<<"Arg is empty!!"<<endl;
    }
    return end;
};

// template <>
// void Param<bool>::parse(Iterator begin, Iterator end)
// {
//     if (vals.empty()) {
//         _val = true;
//         cout<<"parsed: "<<typeid(_val).name()<<" "<<_val<<endl;
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
    Param<T>& add_argument(string name1, string name2="");
    string parse_args();
};

Parser::Parser(int argc, char *argv[])
{
    _argv.insert(_argv.begin(), argv, argv+argc);
}

template <typename T>
Param<T>& Parser::add_argument(string name1, string name2)
{
    auto* param = new Param<T>();
    params[name1] = param;
    if (!name2.empty()) {
        params[name2] = param;
    }
    return *param;
}


string Parser::parse_args()
{
    auto isParam = [&](auto i) { return params.count(i)>0; };
    string error;

    auto pBegin = find_if(_argv.begin(), _argv.end(), isParam);
    while (pBegin != _argv.end())
    {
        cout<<"Beg: "<<*pBegin<<", ";
        auto p = params[*pBegin];
        auto pEnd = find_if(pBegin + 1, _argv.end(), isParam);  // pass after -/--
        auto nBegin = p->parse(pBegin + 1, pEnd);               // pass after -/--, eat args-> advence
        pBegin = _argv.erase(pBegin, pEnd);                     // remove where parser advenced
        cout<<"New beg: "<<*pBegin<<", "<<(pBegin == _argv.end())<<", ";
        for (auto a: _argv) {
            cout<<a<<" ";
        }
        cout<<endl;
    }
    return error;
}
