#include "argument.h"

namespace gcheck {

template class SequenceArgument<int>;
template class SequenceArgument<unsigned int>;
template class SequenceArgument<double>;
template class SequenceArgument<float>;
template class SequenceArgument<std::string>;

}