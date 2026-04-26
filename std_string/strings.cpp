#include <cstring>
#include <iostream>
class String {
  char* str = nullptr;
  size_t size = 0;
  size_t capacity = 1;

 public:
  String() : str(new char[1]{'\0'}), size(0), capacity(1) {}
  String(size_t size, char c)
      : str(new char[size + 1]), size(size), capacity(size + 1) {
    memset(str, c, size);
    str[size] = '\0';
  }

  String(std::initializer_list<char> list)
      : str(new char[list.size() + 1]),
        size(list.size()),
        capacity(list.size() + 1) {
    std::copy(list.begin(), list.end(), str);
    str[size] = '\0';
  }

  String(const char* str_)
      : str(new char[strlen(str_) + 1]),
        size(strlen(str_)),
        capacity(strlen(str_) + 1) {
    std::copy(str_, str_ + size, str);
    str[size] = '\0';
  }

  String(const String& other)
      : str(new char[other.size + 1]),
        size(other.size),
        capacity(other.size + 1) {
    std::copy(other.str, other.str + other.size, str);
    str[size] = '\0';
  }
  String& operator=(String other) {
    swap(other);
    return *this;
  }

  void swap(String& other) {
    std::swap(str, other.str);
    std::swap(size, other.size);
    std::swap(capacity, other.capacity);
  }

  String operator+(const String& other) const {
    String result;
    result.size = size + other.size;
    result.capacity = result.size + 1;
    result.str = new char[result.capacity];

    std::copy(str, str + size, result.str);
    std::copy(other.str, other.str + other.size, result.str + size);
    result.str[result.size] = '\0';

    return result;
  }

  ~String() { delete[] str; }
};
int main() {
  String s1(2, 'c');
  String s2{'a', 'b'};
  String s3 = "abc";
  String s4 = s3;
  s3 = s2 = s1;
  String s5 = s1 + s2;
}
