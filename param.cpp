#include "param.h"

void Param_T::validate(int naargs) {
    if (naargs < _min) {
        auto error = "not enought parameters, expected: " + std::to_string(_min) + ", reveived: " + std::to_string(naargs);
        throw std::invalid_argument(error);
    }
    if (_max != -1 && naargs > _max) {
        auto error = "too many parameters, expected: " + std::to_string(_max) + ", reveived: " + std::to_string(naargs);
        throw std::invalid_argument(error);
    }
}