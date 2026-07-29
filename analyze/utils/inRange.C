#ifndef INRANGE_C
#define INRANGE_C

template <typename T, typename U>
Bool_t inRange(const T& value, const U (&range)[2]) {
    return value >= range[0] && value <= range[1];
}

#endif
