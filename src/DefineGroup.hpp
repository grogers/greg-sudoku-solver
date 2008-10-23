#ifndef DEFINE_GROUP_HPP
#define DEFINE_GROUP_HPP

#define DEFINE_PAIR(classname, T1, T2, name1, name2) \
    struct classname { \
        classname() {} \
        classname(const T1 &a, const T2 &b) : name1(a), name2(b) {} \
        classname(const classname &x) : name1(x.name1), name2(x.name2) {} \
        const classname &operator=(const classname &x) { \
            name1 = x.name1; \
            name2 = x.name2; \
            return *this; \
        } \
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
    
#define DEFINE_TRIPLET(classname, T1, T2, T3, name1, name2, name3) \
    struct classname { \
        classname() {} \
        classname(const T1 &a, const T2 &b, const T3 &c) : \
            name1(a), name2(b), name3(c) {} \
        classname(const classname &x) : \
            name1(x.name1), name2(x.name2), name3(x.name3) {} \
        const classname &operator=(const classname &x) { \
            name1 = x.name1; \
            name2 = x.name2; \
            name3 = x.name3; \
            return *this; \
        } \
        T1 name1; \
        T2 name2; \
        T3 name3; \
        bool operator==(const classname &x) const \
        { return name1 == x.name1 && name2 == x.name2 && name3 == x.name3; } \
        bool operator!=(const classname &x) const \
        { return !(*this == x); } \
        bool operator<(const classname &x) const \
        { \
            return name1 < x.name1 || \
                (!(x.name1 < name1) && (name2 < x.name2 || \
                    (!(x.name2 < name2) && name3 < x.name3))); \
        } \
        bool operator>=(const classname &x) const \
        { return !(*this < x); } \
        bool operator>(const classname &x) const \
        { return x < *this; } \
        bool operator<=(const classname &x) const \
        { return !(x < *this); } \
    }


        
            


#endif
