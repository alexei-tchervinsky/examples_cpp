//ChAP 2016-10-02 test for scope of std::shared pointers
// note: compile with -std=c++11
// g++ -std=c++11 -o chap_shared_ptr_std chap_shared_ptr_std.cpp

#include <iostream>
#include <memory> // std::shared_ptr

using namespace std;

class A {
public:
  void print() {
    cout << "I am class A instance" << endl;
  }
};

void test_shared_ptr_scope() 
{
  shared_ptr<A> pout;
  cout << __FUNCTION__ << endl;
  cout << "pout unintilized=" << pout << endl;
  cout << "pout count=" << pout.use_count() << endl;
  {
    cout  << "enter inner scope" << endl;
    shared_ptr<A> pin(new A);
    cout << "pin initilazed=" << pin << endl;
    cout << "pin count=" << pin.use_count() << endl;
    
    cout << "pout=pin" << endl;
    pout = pin;
    cout << "pout=" << pout << endl;
    cout << "pout count=" << pout.use_count() << endl;
    cout << "pin=" << pin << endl;
    cout << "pin count=" << pin.use_count() << endl;

    pin->print();
    pout->print();

    cout << "exit inner scope" << endl;
  }

    cout << "pout=" << pout << endl;
    cout << "pout count=" << pout.use_count() << endl;
    // cout << "pin=" << pin << endl;
    // cout << "pin count=" << pin.use_count() << endl;
    pout->print();
}

int main()
{
  cout << "test for scope of shared pointers" << endl;
  test_shared_ptr_scope();
}
