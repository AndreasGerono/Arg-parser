#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

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
    string name;
    string _help;
    bool _required;
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


template <typename T>
using CB1 = function<void(T)>;
template <typename T>
using CB2 = function<void(vector<T>)>;

template <typename T>
class Param: private Param_T {
public:
    Param(vector<string> names);
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
Param<T>::Param(vector<string> names)
{
    for_each(names.begin(), names.end(), [&](auto &elem) {
        name += elem;
        if (elem != names.back()) {
            name += ", ";
        }
    });
};

template <typename T>
void Param<T>::parse(Iterator begin, Iterator end)
{
    auto args = vector<string>(begin, end);
    auto push_arg = [&](string arg) {
        T tmp;
        istringstream ss(arg);  // Conversion to T
        ss>>tmp;                //
        _args.push_back(tmp);
        cout<<endl<<"\tparsed: "<<typeid(tmp).name()<<" "<<tmp;
    };

    cout<<endl<<"Param vars: ";

    for_each(args.begin(), args.end(), push_arg);

    if (args.empty()) {
        push_arg("1");
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
    Type parse_type(Arg names);
    pair<int, int> parse_bounds(string naargs);
    unordered_map<string, Param_T*> params;
    vector<Param_T*> pos_params;
    vector<Param_T*> opt_params;
    vector<string> _argv;

public:
    ArgumentParser(int argc, char *argv[]);
    template <typename T=bool> Param<T>& add_argument(string name, string naargs="1");
    template <typename T=bool> Param<T>& add_argument(Arg names, string naargs="1");
    template <typename T> const T get(string arg, size_t idx=0);
    template <typename T> const vector<T> getV(string arg);
    void parse_args(void);
    void print_help(void);
};


template <typename T>
const T ArgumentParser::get(string arg, size_t idx)
{
    auto result = getV<T>(arg);
    if (idx + 1 > result.size()) {
        return T(); // always return default
    }
    return result[idx];
}

template <typename T>
const vector<T> ArgumentParser::getV(string arg)
{
    auto it = params.find(arg);
    if (it != params.end()) {
        auto tmp = static_cast<Param<T>*>(it->second);
        return tmp->_args;
    }
    auto error = "Error while accessing '" + arg + "': argument not known!";
    throw invalid_argument(error);
}

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

void ArgumentParser::parse_args(void)
{
    parse_opt_args();
    parse_pos_args();
}

void ArgumentParser::parse_opt_args(void)
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

void ArgumentParser::parse_pos_args(void)
{
    cout<<"Positional args: ";
    for (auto &param: pos_params) {
        cout<<endl<<"pos_param"<<": "<<param->name<<" ";
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
            auto error = "Error while parsing '" + param->name + "': " + e.what();
            throw_with_nested(invalid_argument(error));
        }

        aBeg = _argv.erase(aBeg, aEnd);
    }
}

void ArgumentParser::print_help(void)
{
    auto len_compare = [](auto a, auto b) { return a->name.length() < b->name.length(); };

    auto it1 = max_element(pos_params.begin(), pos_params.end(), len_compare);
    auto it2 = max_element(opt_params.begin(), opt_params.end(), len_compare);

    auto max_len = max((*it1)->name.length(), (*it2)->name.length());

    auto generate_help = [] (Param_T* param, size_t max_len) {
        auto len = max_len - param->name.length();
        string spaces(len, ' ');
        return "  " + param->name + spaces + "\t" + param->_help + "\n";
    };

    string help = "positional arguments:\n";
    for (auto param: pos_params) {
        help += generate_help(param, max_len);
    }

    help += "\noptions:\n";
    for (auto param: opt_params) {
        help += generate_help(param, max_len);
    }

    cout<<help;
}
