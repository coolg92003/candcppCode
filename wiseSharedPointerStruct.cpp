#include <iostream>
#include <string>
#include <typeinfo>
#include <functional>
#include <memory>
struct CHome {
	std::string name;
	int age;
	bool sexn;
};
struct CCity {
	std::string name;
	std::string zipcode;
	std::vector <std::shared_ptr<struct CHome>> chome_shared_ptr;
};
void cfx_ustruct(std::shared_ptr<struct CCity> cCtiy) {
	std::cout << "\n\tcfx_ustruct: enter" << std::endl;
	std::cout << "\tcfx_ustruct: vector size of home:"    <<
		cCtiy->chome_shared_ptr.size()<< std::endl;
	std::cout << "\tcfx_ustruct: CHOME.use_cnt():" << 
		cCtiy->chome_shared_ptr.at(0).use_count() << std::endl;
	std::cout << "\tcfx_ustruct: exit" << std::endl;
}
auto shared_ptr1 = std::make_shared<CCity>();
void cfx_cstruct() {
	std::cout << "\n\tcfx_cstruct: enter" << std::endl;
	shared_ptr1->name = "QingDao";
	shared_ptr1->zipcode = "266101";
	std::cout << "\tcfx_cstruct: shared_ptr1.use_cnt():" << shared_ptr1.use_count() << std::endl;
	auto ptr2 = std::make_shared<CHome>();
	ptr2->name = "Cliff";
	ptr2->age = 28;
	ptr2->sexn = true;
	shared_ptr1->chome_shared_ptr.push_back(ptr2);
	std::cout << "\tcfx_cstruct: CHOME.use_cnt():" << ptr2.use_count() << std::endl;
	cfx_ustruct(shared_ptr1);
	std::cout << "\tcfx_cstruct: exit" << std::endl;
}
int main() {
	std::cout << "main: enter" << std::endl;
	cfx_cstruct();
	std::cout << "main: after call cfx_cstruct" << std::endl;
	std::cout << "main: shared_ptr1.use_cnt():" << shared_ptr1.use_count() << std::endl;
	std::cout << "main: CHOME.use_cnt():" << 
		shared_ptr1->chome_shared_ptr.at(0).use_count() << std::endl;
	std::cout << "main: exit" << std::endl;
	return 0;
}
