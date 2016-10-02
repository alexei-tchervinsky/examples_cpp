// http://theboostcpplibraries.com/boost.smartpointers-special-smart-pointers

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <thread>
#include <functional>
#include <iostream>

#ifdef KDG768
void reset(boost::shared_ptr<int> &sh, boost::shared_ptr<int> &p1)
#else
void reset(boost::shared_ptr<int> &sh)
#endif
{
  std::cout << __FUNCTION__ << std::endl;
  sh.reset();
#ifdef KDG768
  p1.reset();
#endif
}

void print(boost::weak_ptr<int> &w)
{
  boost::shared_ptr<int> sh = w.lock();
  if (sh)
    std::cout << __FUNCTION__ << *sh << std::endl;
  else
    std::cout << __FUNCTION__ << " sh is NULL" << std::endl;
}
#ifdef KDG768
void hello(boost::shared_ptr<int> &p1) // kdg768
{
  if (p1)
    std::cout << __FUNCTION__ << " " << *p1 << std::endl;
  else
    std::cout << __FUNCTION__ << " p1 is NULL" << std::endl;
  
}
#endif

int main()
{
  boost::shared_ptr<int> sh{new int{99}};
  boost::weak_ptr<int> w{sh};
#ifdef KDG768
  boost::shared_ptr<int> p1 = w.lock(); // kdg768
#endif
#ifdef KDG768
  std::thread t1{reset, std::ref(sh), std::ref(p1)};
#else
  std::thread t1{reset, std::ref(sh)};
#endif
  std::thread t2{print, std::ref(w)};
#ifdef KDG768
  std::thread t3{hello, std::ref(p1)}; // kdg768
#endif
  t1.join();
  t2.join();
#ifdef KDG768
  t3.join(); // kdg768
#endif
}
