//ChAP 2016-10-02 test for scope of shared pointers (btw, std::shared_ptr is the same as boost::shared_ptr according to habr - Yes, see chap_shared_ptr_std)

#include <iostream>
#include <boost/shared_ptr.hpp>

using namespace std;

class A {
public:
  void print() {
    cout << "I am class A instance" << endl;
  }
};

void test_shared_ptr_scope() 
{
  boost::shared_ptr<A> pout;
  cout << __FUNCTION__ << endl;
  cout << "pout unintilized=" << pout << endl;
  cout << "pout count=" << pout.use_count() << endl;
  {
    cout  << "enter inner scope" << endl;
    boost::shared_ptr<A> pin(new A);
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
