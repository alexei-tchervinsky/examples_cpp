#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <iostream>

class MyObject {
public:
  void print() {
    std::cout << "This is MyObject" << std::endl;
  }
};

void printMyObjectShared(boost::shared_ptr<MyObject> p) {
  std::cout << __FUNCTION__ << std::endl;
  std::cout << "p=" << p << std::endl;
  std::cout << "p count=" << p.use_count() << std::endl;
  p->print();
}

void printMyObjectScoped(boost::scoped_ptr<MyObject> p) {
  p->print();
}

int test_shared_ptr() {
    boost::shared_ptr<MyObject> p1(new MyObject);
    boost::shared_ptr<MyObject> p2;
    boost::shared_ptr<MyObject> p3(new MyObject);
    
    std::cout << "boost::shared_ptr" << std::endl;
    std::cout << "p1=" << p1 << std::endl;
    std::cout << "p1 count=" << p1.use_count() << std::endl;
    std::cout << "p2=" << p2 << std::endl;
    std::cout << "p2 count=" << p2.use_count() << std::endl;
    std::cout << "p3=" << p3 << std::endl;
    std::cout << "p3 count=" << p3.use_count() << std::endl;
    
    p2 = p1;    
    std::cout << "p1 = p2" << std::endl;
    std::cout << "p1=" << p1 << std::endl;
    std::cout << "p1 count=" << p1.use_count() << std::endl;
    std::cout << "p2=" << p2 << std::endl;
    std::cout << "p2 count=" << p2.use_count() << std::endl;
    std::cout << "p3=" << p3 << std::endl;
    std::cout << "p3 count=" << p3.use_count() << std::endl;

    p3 = p1;
    std::cout << "p3 = p1" << std::endl;
    std::cout << "p1=" << p1 << std::endl;
    std::cout << "p1 count=" << p1.use_count() << std::endl;
    std::cout << "p2=" << p2 << std::endl;
    std::cout << "p2 count=" << p2.use_count() << std::endl;
    std::cout << "p3=" << p3 << std::endl;
    std::cout << "p3 count=" << p3.use_count() << std::endl;

    std::cout << "call function with shared ptr as a param" << std::endl;
    printMyObjectShared(p1);

    // if (p2)
    //     std::cout << "Hello, world!\n";
}

int test_scoped_ptr() {
    boost::scoped_ptr<int> p1(new int(6));
    boost::scoped_ptr<int> p2(new int(1));
    boost::scoped_ptr<MyObject> p3(new MyObject());

    std::cout << std::endl;
    std::cout << "boost::scoped_ptr" << std::endl;
    std::cout << p1 << std::endl;
    std::cout << p2 << std::endl;
    std::cout << p3 << std::endl;

    p3->print();
    //    printMyObjectScoped(p3); // Forbidden, private copy constructor

    //    p1 = p2; // Forbidden, private assignment
}

int test_weak_ptr() {
  // http://thisthread.blogspot.ru/2010/04/boostweakptr.html
  boost::weak_ptr<MyObject> p1;

  std::cout << std::endl;
  std::cout << "boost::weak_ptr" << std::endl; 
  if (p1.expired()) 
    std::cout << "p1 expired, no associated shared_ptr" << std::endl;
  {
    std::cout << "enter block {" << std::endl;
    boost::shared_ptr<MyObject> p2(new MyObject());
    std::cout << "boost::shared_ptr<MyObject> p2(new MyObject())" << std::endl;
    p2->print();
    p1 = p2;
    std::cout << "p1 = p2" << std::endl; 
    if (p1.expired())
      std::cout << "p1 expired" << std::endl;
    else 
      std::cout << "p1 is not expired" << std::endl;

      

    // p1->print(); // compile-error:  base operand of ‘->’ has non-pointer type ‘boost::weak_ptr<MyObject>’
    std::cout << "A weak_ptr does not increase the reference count: " << p2.use_count() << std::endl;
    // create a shared ptr from weak ptr
    std::cout << "create a shared ptr from weak ptr boost::shared_ptr<MyObject> p3(p1)" << std::endl;
    boost::shared_ptr<MyObject> p3(p1);
    std::cout << "Now the reference count p2 is " << p2.use_count() << std::endl;
    if (p1.expired())
      std::cout << "p1 expired" << std::endl;
    else 
      std::cout << "p1 is not expired" << std::endl;
      std::cout << "exit block }" << std::endl;
    }
    if (p1.expired())
      std::cout << "p1 expired" << std::endl;
    else 
      std::cout << "p1 is not expired" << std::endl;
    
    std::cout << "if we create a new shared ptr p4 using lock()..." << std::endl;
    boost::shared_ptr<MyObject> p4 = p1.lock();
    if(!p4)
      std::cout << "we should check it is valid before using it" << std::endl;
    try {
      std::cout << "if we create new shared ptr passing weak ptr to its ctor..." << std::endl;
      boost::shared_ptr<MyObject> p5(p1);
    }
    catch (const boost::bad_weak_ptr& bwp) 
    {
      std::cout << "...it could throw a " << bwp.what() << " exception" << std::endl;
    }

}

int main() {
  test_shared_ptr();
  test_scoped_ptr();
  test_weak_ptr();
}
