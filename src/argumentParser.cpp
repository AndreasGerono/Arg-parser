#include "argumentParser.h"

ArgumentParser::ArgumentParser(int argc, char* argv[]) {
    _argv.insert(_argv.begin(), argv + 1, argv + argc);
}

ArgumentParser::~ArgumentParser() {
    for (auto it = params.begin(); it != params.end(); ++it) {
        delete it->second;
    }
}

std::pair<int, int> ArgumentParser::parse_bounds(std::string naargs) {
    if (naargs == "+") {
        return std::make_pair(1, -1);
    }

    auto min = stoi(naargs);
    if (naargs[0] == '+') {
        return std::make_pair(min, -1);
    }
    auto max = stoi(naargs);
    return std::make_pair(min, max);
}

void ArgumentParser::parse_args(void) {
    parse_opt_args();
    parse_pos_args();
}

void ArgumentParser::parse_opt_args(void) {
    auto isParam = [&](auto i) { return (i[0] == '-' && params.count(i) > 0); };

    auto pBeg = std::find_if(_argv.begin(), _argv.end(), isParam);
    while (pBeg != _argv.end()) {
#ifdef DEBUG
        std::cout << "Beg: " << *pBeg << "; ";
#endif
        auto param = params[*pBeg];

        auto aBeg = std::next(pBeg);
        auto aEnd = std::find_if(aBeg, _argv.end(), isParam);  // pass after -/--
        auto naargs = distance(aBeg, aEnd);
        try {
            param->validate(naargs);
            param->parse(aBeg, aEnd);
        } catch (const std::exception& e) {
            auto error = "Error while parsing '" + *pBeg + "': " + e.what();
            throw_with_nested(std::invalid_argument(error));
        }
        pBeg = _argv.erase(pBeg, aEnd);
#ifdef DEBUG
        std::cout << std::endl
                  << "Left: ";
        for (auto a : _argv) {
            std::cout << a << " ";
        }
        std::cout << std::endl;
#endif
    }
}

void ArgumentParser::parse_pos_args(void) {
#ifdef DEBUG
    std::cout << "Positional args: ";
#endif

    for (auto& param : pos_params) {
#ifdef DEBUG
        std::cout << std::endl
                  << "pos_param"
                  << ": " << param->name << " ";
#endif

        auto aBeg = _argv.begin();
        auto aEnd = _argv.end();

        if (param->_max != -1) {
            aEnd = std::next(aBeg, param->_max);
            if (distance(aEnd, _argv.end()) < 0) {
                aEnd = _argv.end();
            }
        }
        auto naargs = distance(aBeg, aEnd);
        try {
            param->validate(naargs);
            param->parse(aBeg, aEnd);
        } catch (const std::exception& e) {
            auto error = "Error while parsing '" + param->name + "': " + e.what();
            throw_with_nested(std::invalid_argument(error));
        }

        aBeg = _argv.erase(aBeg, aEnd);
    }
}

enum ArgumentParser::Type ArgumentParser::parse_type(Arg names) {
    std::unordered_set<std::string> unique_names(names.begin(), names.end());
    if (names.size() != unique_names.size()) {
        auto error = "Error while adding: '" + names[0] + "': argument names must be unique!";
        throw std::invalid_argument(error);
    }

    auto any_optional = any_of(names.begin(), names.end(), [](std::string n) { return n[0] == '-'; });
    if (any_optional) {
        auto all_optional = all_of(names.begin(), names.end(), [](std::string n) { return n[0] == '-'; });
        if (all_optional) {
            return OPTIONAL;
        }
        auto error = "Error while adding: '" + names[0] + "': you can't mix optional and non optional argument names!";
        throw std::invalid_argument(error);
    }
    return POSITIONAL;
}

void ArgumentParser::print_help(void) {
    auto len_compare = [](auto a, auto b) { return a->name.length() < b->name.length(); };

    auto it1 = max_element(pos_params.begin(), pos_params.end(), len_compare);
    auto it2 = max_element(opt_params.begin(), opt_params.end(), len_compare);

    auto max_len = std::max((*it1)->name.length(), (*it2)->name.length());

    auto generate_help = [](Param_T* param, size_t max_len) {
        auto len = max_len - param->name.length();
        std::string spaces(len, ' ');
        return "  " + param->name + spaces + "\t" + param->_help + "\n";
    };

    std::string help = "positional arguments:\n";
    for (auto param : pos_params) {
        help += generate_help(param, max_len);
    }

    help += "\noptions:\n";
    for (auto param : opt_params) {
        help += generate_help(param, max_len);
    }

    std::cout << std::endl
              << help;
}
