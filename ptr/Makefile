all:\
boost_shared_ptr \
example_weak_ptr \
chap_shared_ptr \
chap_shared_ptr_std \
chap_without_smart_ptr

boost_shared_ptr: boost_shared_ptr.cpp	
	g++ -o boost_shared_ptr boost_shared_ptr.cpp

example_weak_ptr: example_weak_ptr.cpp
	g++ -std=c++11 -pthread -o example_weak_ptr example_weak_ptr.cpp
	g++ -std=c++11 -pthread -DKDG768 -o example_weak_ptr_kdg768 example_weak_ptr.cpp

chap_shared_ptr: chap_shared_ptr.cpp
	g++  -o chap_shared_ptr chap_shared_ptr.cpp

chap_shared_ptr_std: chap_shared_ptr_std.cpp
	g++ -std=c++11 -o chap_shared_ptr_std chap_shared_ptr_std.cpp

chap_without_smart_ptr: chap_without_smart_ptr.cpp
	g++ -o chap_without_smart_ptr chap_without_smart_ptr.cpp
