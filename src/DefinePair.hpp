#ifndef DEFINE_PAIR_HPP
#define DEFINE_PAIR_HPP

#include <utility>

#define DEFINE_PAIR(classname, T1, T2, name1, name2) \
    struct classname { \
        classname() {} \
        classname(const T1 &a, const T2 &b) : name1(a), name2(b) {} \
        classname(const classname &x) : name1(x.name1), name2(x.name2) {} \
        /*classname &operator=(const classname &x) = default; */\
        T1 name1; \
        T2 name2; \
        bool operator==(const classname &x) const \
        { return name1 == x.name1 && name2 == x.name2; } \
        bool operator!=(const classname &x) const \
        { return !(*this == x); } \
        bool operator<(const classname &x) const \
        { return name1 < x.name1 || (!(x.name1 < name1) && name2 < x.name2); } \
        bool operator>=(const classname &x) const \
        { return !(*this < x); } \
        bool operator>(const classname &x) const \
        { return x < *this; } \
        bool operator<=(const classname &x) const \
        { return !(x < *this); } \
    }
    



        
            


#endif
