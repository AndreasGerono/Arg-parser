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

using namespace std;

class Param_T {
public:
    Param_T() = default;
    ~Param_T() = default;
    virtual void call() {};
    virtual void parse(vector<string> &vals) {};
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
    void parse(vector<string> &vals) override;
    T _val;
    CB<T> _cb;
};

template <typename T>
void Param<T>::parse(vector<string> &vals)
{
    if (vals.size() == 1) {
        auto a = vals.at(0);
        istringstream ss(a);
        ss>>_val;
        cout<<"parsed: "<<typeid(_val).name()<<" "<<_val<<endl;
        call();
    } else if (vals.size() > 1) {
        cout<<"Too many arguments expected one!!";
    } else {
        cout<<"Arg is empty!!";
    }
};

template <>
void Param<bool>::parse(vector<string> &vals)
{
    if (vals.empty()) {
        _val = true;
        cout<<"parsed: "<<typeid(_val).name()<<" "<<_val<<endl;
    } else {
        cout<<"Too many arguments!!";
    }
};

class Parser
{
private:
    map<string, Param_T*> params;
    vector<string> _argv;
    vector<string> all_names;
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
    all_names.insert(all_names.end(), {name1, name2});
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
        cout<<*pBegin<<" ";
        auto p = params[*pBegin];
        pBegin += 1;
        auto pEnd = find_if(pBegin, _argv.end(), isParam);
        auto vals = vector<string>(pBegin, pEnd);
        for (auto a: vals) {
            cout<<a<<" ";
        }
        p->parse(vals);
        pBegin = pEnd;
    }
    return error;
}
