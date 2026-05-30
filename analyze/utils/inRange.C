#ifndef INRANGE_C
#define INRANGE_C

template <typename T>
Bool_t inRange(const T& value, const T (&range)[2])
{
    return value >= range[0] && value <= range[1];
}

#endif
