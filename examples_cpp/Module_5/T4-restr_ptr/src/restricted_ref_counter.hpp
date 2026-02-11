#ifndef AALTO_ELEC_CPP_COUNTER_CLASS
#define AALTO_ELEC_CPP_COUNTER_CLASS

// implement a reference counter helper class below, if you deem needed
// BEGIN SOLUTION
const int kMaxReferenceCount = 3;

class RestrictedRefCounter
{
public:
    RestrictedRefCounter() : count_(0) {}

    bool Increment()
    {
        if (count_ < kMaxReferenceCount)
        {
            count_++;
            return true;
        }
        return false;
    }

    int Decrement() { return --count_; }
    int GetCount() { return count_; }

private:
    int count_;
};
// END SOLUTION

#endif