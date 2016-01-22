#ifndef String_h
#define String_h

#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <strings.h>
#include <time.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <rct/List.h>

#define RCT_PRINTF_WARNING(fmt, firstarg) __attribute__ ((__format__ (__printf__, fmt, firstarg)))
class String
{
public:
    static const size_t npos = std::string::npos;

    enum CaseSensitivity
    {
        CaseSensitive,
        CaseInsensitive
    };
    String(const char *data = 0, size_t len = npos)
    {
        if (data) {
            if (len == npos)
                len = strlen(data);
            mString.assign(data, len);
        }
    }
    String(const char *start, const char *end)
    {
        if (start) {
            mString.assign(start, end);
        }
    }
    String(size_t len, char fillChar)
        : mString(len, fillChar)
    {}

    String(const String &ba)
        : mString(ba.mString)
    {}

    String(String &&ba)
        : mString(std::move(ba.mString))
    {
    }

    String(const std::string &str)
        : mString(str)
    {}

    String &operator=(const String &other)
    {
        mString = other.mString;
        return *this;
    }

    void assign(const char *data, size_t len = npos)
    {
        if (data || !len) {
            if (len == npos)
                len = strlen(data);
            mString.assign(data, len);
        } else {
            clear();
        }
    }

    size_t lastIndexOf(char ch, size_t from = npos, CaseSensitivity cs = CaseSensitive) const
    {
        if (cs == CaseSensitive)
            return mString.rfind(ch, from == npos ? std::string::npos : size_t(from));
        const char *data = mString.c_str();
        if (from == npos)
            from = mString.size() - 1;
        ch = tolower(ch);
        int f = static_cast<int>(from);
        while (f >= 0) {
            if (tolower(data[f]) == ch)
                return from;
            --f;
        }
        return npos;
    }

    size_t indexOf(char ch, size_t from = 0, CaseSensitivity cs = CaseSensitive) const
    {
        if (cs == CaseSensitive)
            return mString.find(ch, from);
        const char *data = mString.c_str();
        ch = tolower(ch);
        const size_t size = mString.size();
        while (from < size) {
            if (tolower(data[from]) == ch)
                return from;
            ++from;
        }
        return npos;
    }

    bool contains(const String &other, CaseSensitivity cs = CaseSensitive) const
    {
        return indexOf(other, 0, cs) != npos;
    }

    bool contains(char ch, CaseSensitivity cs = CaseSensitive) const
    {
        return indexOf(ch, 0, cs) != npos;
    }

    size_t chomp(const String &chars)
    {
        size_t idx = size() - 1;
        while (idx > 0) {
            if (chars.contains(at(idx - 1))) {
                --idx;
            } else {
                break;
            }
        }
        const size_t ret = size() - idx - 1;
        if (ret)
            resize(idx);
        return ret;
    }

    size_t chomp(char ch)
    {
        return chomp(String(&ch, 1));
    }

    size_t lastIndexOf(const String &ba, size_t from = npos, CaseSensitivity cs = CaseSensitive) const
    {
        if (ba.isEmpty())
            return npos;
        if (ba.size() == 1)
            return lastIndexOf(ba.first(), from, cs);
        if (cs == CaseSensitive)
            return mString.rfind(ba.mString, from == npos ? std::string::npos : size_t(from));
        if (from == npos)
            from = mString.size() - 1;
        const String lowered = ba.toLower();
        const size_t needleSize = lowered.size();
        size_t matched = 0;
        int f = static_cast<int>(from);
        while (f >= 0) {
            if (lowered.at(needleSize - matched - 1) != tolower(at(f))) {
                matched = 0;
            } else if (++matched == needleSize) {
                return f;
            }

            --f;
        }
        return npos;
    }

    size_t indexOf(const String &ba, size_t from = 0, CaseSensitivity cs = CaseSensitive) const
    {
        if (ba.isEmpty())
            return npos;
        if (ba.size() == 1)
            return indexOf(ba.first(), from, cs);
        if (cs == CaseSensitive)
            return mString.find(ba.mString, from);

        const String lowered = ba.toLower();
        const size_t count = size();
        size_t matched = 0;

        for (size_t i=from; i<count; ++i) {
            if (lowered.at(matched) != tolower(at(i))) {
                matched = 0;
            } else if (++matched == lowered.size()) {
                return i - matched + 1;
            }
        }
        return npos;
    }

    char first() const
    {
        return at(0);
    }

    char &first()
    {
        return operator[](0);
    }

    char last() const
    {
        assert(!isEmpty());
        return at(size() - 1);
    }

    char &last()
    {
        assert(!isEmpty());
        return operator[](size() - 1);
    }


    String toLower() const
    {
        std::string ret = mString;
        std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
        return ret;
    }

    String toUpper() const
    {
        std::string ret = mString;
        std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
        return ret;
    }

    String trimmed(const String &trim = " \f\n\r\t\v") const
    {
        const size_t start = mString.find_first_not_of(trim);
        if (start == npos)
            return String();

        const size_t end = mString.find_last_not_of(trim);
        assert(end != npos);
        return mid(start, end - start + 1);
    }

    enum Pad {
        Beginning,
        End
    };
    String padded(Pad pad, size_t size, char fillChar = ' ', bool truncate = false) const
    {
        const size_t l = length();
        if (l == size) {
            return *this;
        } else if (l > size) {
            if (!truncate)
                return *this;
            if (pad == Beginning) {
                return right(size);
            } else {
                return left(size);
            }
        } else {
            String ret = *this;
            if (pad == Beginning) {
                ret.prepend(String(size - l, fillChar));
            } else {
                ret.append(String(size - l, fillChar));
            }
            return ret;
        }
    }

    char *data()
    {
        return &mString[0];
    }

    void clear()
    {
        mString.clear();
    }
    const char *data() const
    {
        return mString.data();
    }
    bool isEmpty() const
    {
        return mString.empty();
    }

    char at(size_t i) const
    {
        return mString.at(i);
    }

    char& operator[](size_t i)
    {
        return mString.operator[](i);
    }

    const char& operator[](size_t i) const
    {
        return mString.operator[](i);
    }

    const char *c_str() const
    {
        return mString.c_str();
    }

    const char *constData() const
    {
        return mString.data();
    }

    const char *nullTerminated() const
    {
        return mString.c_str();
    }

    size_t size() const
    {
        return mString.size();
    }

    size_t length() const
    {
        return size();
    }

    void truncate(size_t size)
    {
        if (mString.size() > size)
            mString.resize(size);
    }

    void chop(size_t s)
    {
        mString.resize(size() - s);
    }

    void resize(size_t size)
    {
        mString.resize(size);
    }

    void reserve(size_t size)
    {
        mString.reserve(size);
    }

    void prepend(const String &other)
    {
        mString.insert(0, other);
    }

    void prepend(char ch)
    {
        mString.insert(0, &ch, 1);
    }

    void insert(size_t pos, const String &text)
    {
        mString.insert(pos, text.constData(), text.size());
    }

    void insert(size_t pos, const char *str, size_t len = npos)
    {
        if (str) {
            if (len == npos)
                len = strlen(str);
            mString.insert(pos, str, len);
        }
    }

    void insert(size_t pos, char ch)
    {
        mString.insert(pos, &ch, 1);
    }

    void append(char ch)
    {
        mString += ch;
    }

    void append(const String &ba)
    {
        mString.append(ba);
    }

    String compress() const;
    String uncompress() const { return uncompress(constData(), size()); }
    static String uncompress(const char *data, size_t size);

    void append(const char *str, size_t len = npos)
    {
        if (len == npos)
            len = strlen(str);
        if (len > 0)
            mString.append(str, len);
    }

    void remove(size_t idx, size_t count)
    {
        mString.erase(idx, count);
    }

    String &operator+=(char ch)
    {
        mString += ch;
        return *this;
    }

    String &operator+=(const char *cstr)
    {
        if (cstr)
            mString += cstr;
        return *this;
    }

    String &operator+=(const String &other)
    {
        mString += other.mString;
        return *this;
    }

    size_t compare(const String &other, CaseSensitivity cs = CaseSensitive) const
    {
        if (cs == CaseSensitive)
            return mString.compare(other.mString);
        return strcasecmp(mString.c_str(), other.mString.c_str());
    }

    bool operator==(const String &other) const
    {
        return mString == other.mString;
    }

    bool operator==(const char *other) const
    {
        return other && !mString.compare(other);
    }

    bool operator!=(const String &other) const
    {
        return mString != other.mString;
    }

    bool operator!=(const char *other) const
    {
        return !other || mString.compare(other);
    }

    bool operator<(const String &other) const
    {
        return mString < other.mString;
    }

    bool operator>(const String &other) const
    {
        return mString > other.mString;
    }

    bool endsWith(char ch, CaseSensitivity c = CaseSensitive) const
    {
        const size_t s = mString.size();
        if (s) {
            return (c == CaseInsensitive
                    ? tolower(at(s - 1)) == tolower(ch)
                    : at(s - 1) == ch);
        }
        return false;
    }

    bool startsWith(char ch, CaseSensitivity c = CaseSensitive) const
    {
        if (!isEmpty()) {
            return (c == CaseInsensitive
                    ? tolower(at(0)) == tolower(ch)
                    : at(0) == ch);
        }
        return false;
    }

    bool endsWith(const String &str, CaseSensitivity cs = CaseSensitive) const
    {
        return endsWith(str.constData(), str.size(), cs);
    }

    bool endsWith(const char *str, size_t len = npos, CaseSensitivity cs = CaseSensitive) const
    {
        if (len == npos)
            len = strlen(str);
        const size_t s = mString.size();
        if (s >= len) {
            return (cs == CaseInsensitive ? !strncasecmp(str, constData() + s - len, len) : !strncmp(str, constData() + s - len, len));
        }
        return false;
    }


    bool startsWith(const String &str, CaseSensitivity cs = CaseSensitive) const
    {
        return startsWith(str.constData(), str.size(), cs);
    }

    bool startsWith(const char *str, size_t len = npos, CaseSensitivity cs = CaseSensitive) const
    {
        const size_t s = mString.size();
        if (len == npos)
            len = strlen(str);
        if (s >= len) {
            return (cs == CaseInsensitive ? !strncasecmp(str, constData(), len) : !strncmp(str, constData(), len));
        }
        return false;
    }

    void replace(size_t idx, size_t len, const String &with)
    {
        mString.replace(idx, len, with.mString);
    }

    void replace(const String &from, const String &to)
    {
        size_t idx = 0;
        while (true) {
            idx = indexOf(from, idx);
            if (idx == npos)
                break;
            replace(idx, from.size(), to);
            idx += to.size();
        }
    }

    size_t replace(char from, char to)
    {
        size_t count = 0;
        int i = static_cast<int>(size() - 1);
        while (i >= 0) {
            char &ch = operator[](i);
            if (ch == from) {
                ch = to;
                ++count;
            }
            --i;
        }
        return count;
    }

    String mid(size_t from, size_t l = npos) const
    {
        if (l == npos)
            l = size() - from;
        if (from == 0 && l == size())
            return *this;
        return mString.substr(from, l);
    }

    String left(size_t l) const
    {
        return mString.substr(0, l);
    }

    String right(size_t l) const
    {
        return mString.substr(size() - l, l);
    }

    operator std::string() const
    {
        return mString;
    }

    std::string& ref()
    {
        return mString;
    }

    const std::string& ref() const
    {
        return mString;
    }

    enum SplitFlag {
        NoSplitFlag = 0x0,
        SkipEmpty = 0x1,
        KeepSeparators = 0x2
    };
    List<String> split(char ch, unsigned int flags = NoSplitFlag) const
    {
        List<String> ret;
        size_t last = 0;
        const size_t add = flags & KeepSeparators ? 1 : 0;
        while (1) {
            const size_t next = indexOf(ch, last);
            if (next == npos)
                break;
            if (next > last || !(flags & SkipEmpty))
                ret.append(mid(last, next - last + add));
            last = next + 1;
        }
        if (last < size() || !(flags & SkipEmpty))
            ret.append(mid(last));
        return ret;
    }

    List<String> split(const String &split, unsigned int flags = NoSplitFlag) const
    {
        List<String> ret;
        size_t last = 0;
        while (1) {
            const size_t next = indexOf(split, last);
            if (next == npos)
                break;
            if (next > last || !(flags & SkipEmpty))
                ret.append(mid(last, next - last));
            last = next + split.size();
        }
        if (last < size() || !(flags & SkipEmpty))
            ret.append(mid(last));
        return ret;
    }

    uint64_t toULongLong(bool *ok = 0, size_t base = 10) const
    {
        errno = 0;
        char *end = 0;
        const uint64_t ret = ::strtoull(constData(), &end, base);
        if (ok)
            *ok = !errno && !*end;
        return ret;
    }
    int64_t toLongLong(bool *ok = 0, size_t base = 10) const
    {
        errno = 0;
        char *end = 0;
        const int64_t ret = ::strtoll(constData(), &end, base);
        if (ok)
            *ok = !errno && !*end;
        return ret;
    }
    uint32_t toULong(bool *ok = 0, size_t base = 10) const
    {
        errno = 0;
        char *end = 0;
        const uint32_t ret = ::strtoul(constData(), &end, base);
        if (ok)
            *ok = !errno && !*end;
        return ret;
    }
    int32_t toLong(bool *ok = 0, size_t base = 10) const
    {
        errno = 0;
        char *end = 0;
        const int32_t ret = ::strtol(constData(), &end, base);
        if (ok)
            *ok = !errno && !*end;
        return ret;
    }

    enum TimeFormat {
        DateTime,
        Time,
        Date
    };

    static String formatTime(time_t t, TimeFormat fmt = DateTime)
    {
        const char *format = 0;
        switch (fmt) {
        case DateTime:
            format = "%Y-%m-%d %H:%M:%S";
            break;
        case Date:
            format = "%Y-%m-%d";
            break;
        case Time:
            format = "%H:%M:%S";
            break;
        }

        char buf[32];
        tm tm;
        localtime_r(&t, &tm);
        const size_t w = strftime(buf, sizeof(buf), format, &tm);
        return String(buf, w);
    }

    String toHex() const { return toHex(*this); }
    static String toHex(const String &hex) { return toHex(hex.constData(), hex.size()); }
    static String toHex(const void *data, size_t len);

    static String number(int8_t num, size_t base = 10) { return String::number(static_cast<int64_t>(num), base); }
    static String number(uint8_t num, size_t base = 10) { return String::number(static_cast<int64_t>(num), base); }
    static String number(int16_t num, size_t base = 10) { return String::number(static_cast<int64_t>(num), base); }
    static String number(uint16_t num, size_t base = 10) { return String::number(static_cast<int64_t>(num), base); }
    static String number(int32_t num, size_t base = 10) { return String::number(static_cast<int64_t>(num), base); }
    static String number(uint32_t num, size_t base = 10) { return String::number(static_cast<int64_t>(num), base); }
    static String number(int64_t num, size_t base = 10)
    {
        const char *format = 0;
        switch (base) {
        case 10: format = "%lld"; break;
        case 16: format = "0x%llx"; break;
        case 8: format = "%llo"; break;
        case 1: {
            String ret;
            while (num) {
                ret.append(num & 1 ? '1' : '0');
                num >>= 1;
            }
            return ret; }
        default:
            assert(0);
            return String();
        }
        char buf[32];
        const size_t w = ::snprintf(buf, sizeof(buf), format, num);
        return String(buf, w);
    }

    static String number(uint64_t num, size_t base = 10)
    {
        const char *format = 0;
        switch (base) {
        case 10: format = "%llu"; break;
        case 16: format = "0x%llx"; break;
        case 8: format = "%llo"; break;
        case 1: {
            String ret;
            while (num) {
                ret.append(num & 1 ? '1' : '0');
                num >>= 1;
            }
            return ret; }
        default:
            assert(0);
            return String();
        }
        char buf[32];
        const size_t w = ::snprintf(buf, sizeof(buf), format, num);
        return String(buf, w);
    }

    static String number(double num, size_t prec = 2)
    {
        char format[32];
        snprintf(format, sizeof(format), "%%.%zuf", prec);
        char buf[32];
        const size_t w = ::snprintf(buf, sizeof(buf), format, num);
        return String(buf, w);
    }

    static String join(const List<String> &list, char ch)
    {
        return String::join(list, String(&ch, 1));
    }

    static String join(const List<String> &list, const String &sep)
    {
        String ret;
        const size_t sepSize = sep.size();
        size_t size = std::max<size_t>(0, list.size() - 1) * sepSize;
        const size_t count = list.size();
        for (size_t i=0; i<count; ++i)
            size += list.at(i).size();
        ret.reserve(size);
        for (size_t i=0; i<count; ++i) {
            const String &b = list.at(i);
            ret.append(b);
            if (sepSize && i + 1 < list.size())
                ret.append(sep);
        }
        return ret;
    }
    template <size_t StaticBufSize = 4096>
    static String format(const char *format, ...) RCT_PRINTF_WARNING(1, 2);

    template <size_t StaticBufSize = 4096>
    static String format(const char *format, va_list args)
    {
        va_list copy;
        va_copy(copy, args);

        char buffer[StaticBufSize];
        const size_t size = ::vsnprintf(buffer, StaticBufSize, format, args);
        assert(size >= 0);
        String ret;
        if (size < StaticBufSize) {
            ret.assign(buffer, size);
        } else {
            ret.resize(size);
            ::vsnprintf(&ret[0], size+1, format, copy);
        }
        va_end(copy);
        return ret;
    }
private:
    std::string mString;
};

template <size_t StaticBufSize>
String String::format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const String ret = String::format<StaticBufSize>(format, args);
    va_end(args);
    return ret;
}

inline bool operator==(const char *l, const String &r)
{
    return r.operator==(l);
}

inline bool operator!=(const char *l, const String &r)
{
    return r.operator!=(l);
}

inline const String operator+(const String &l, const char *r)
{
    String ret = l;
    ret += r;
    return ret;
}

inline const String operator+(const char *l, const String &r)
{
    String ret = l;
    ret += r;
    return ret;
}

inline const String operator+(const String &l, char ch)
{
    String ret = l;
    ret += ch;
    return ret;
}

inline const String operator+(char l, const String &r)
{
    String ret;
    ret.reserve(r.size() + 1);
    ret += l;
    ret += r;
    return ret;
}

inline const String operator+(const String &l, const String &r)
{
    String ret = l;
    ret += r;
    return ret;
}

namespace std
{
template <> struct hash<String> : public unary_function<String, size_t>
{
    size_t operator()(const String& value) const
    {
        std::hash<std::string> h;
        return h(value);
    }
};
}


#endif
