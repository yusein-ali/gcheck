#include "argument.h"

namespace gcheck {
    
template class RandomSizeContainer<int>;
template class RandomSizeContainer<unsigned int>;
template class RandomSizeContainer<double>;
template class RandomSizeContainer<float>;
template class RandomSizeContainer<std::string>;
template class SequenceArgument<int>;
template class SequenceArgument<unsigned int>;
template class SequenceArgument<double>;
template class SequenceArgument<float>;
template class SequenceArgument<std::string>;

}