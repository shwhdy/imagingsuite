#ifndef SAVITZKYGOLAYFILTER_H
#define SAVITZKYGOLAYFILTER_H

#include <vector>
namespace kipl {
namespace filters {

template <typename T>
class SavitzkyGolayFilter
{
public:
    enum eSavGolInterp {sgMirror, sgConstant, sgNearest, sgInterp, sgWrap};
    SavitzkyGolayFilter();

    void operator()(T *signal, size_t N, T * result, int order, int windowSize);
    std::vector<T> operator()(const std::vector<T> &signal, int window_length, int polyOrder, int deriv=0, double delta=1.0,eSavGolInterp mode = sgInterp, T cval=0.0);
    std::vector<T> coeffs(int windowLength, int polyOrder, int deriv=0, double delta=1.0);
private:
    void initialize(int order, int windowSize);
};
}}

#include "core/savitzkygolayfilter.hpp"

#endif // SAVITZKYGOLAYFILTER_H