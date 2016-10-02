// ChAP 2016-10-02 what if do not use smart ptr

#include <iostream>

using namespace std;


class A {
public:
  void print() {
    cout << "I am class A instance" << endl;
  }
};

void test_without_smart_ptr() 
{
  A* p(new A);
  p->print();
  delete p;
}

int main()
{
  cout << "test without smart pointers" << endl;
  test_without_smart_ptr();
}
