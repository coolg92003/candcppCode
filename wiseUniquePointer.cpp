#include <iostream>
#include <string>
#include <functional>
//wisepointer
#include <memory>

using FilterFactoryCb = std::function<void(std::string & pStr)>;
class CFXApple {
public:
	CFXApple(std::string aName): name(aName) {
		std::cout << "\t\t\tCFXApple: Con is called" << std::endl;
	}
	~CFXApple() {
		std::cout << "\t\t\tCFXApple: ~Con is called" << std::endl;
	}
	void display(void) const {
		std::cout << "\t\t\tCFXApple: name < " << name << " >" << std::endl;
	}
private:
	std::string name;

};
class CFXWisePointerC {
public:
	CFXWisePointerC(std::string pV) : Pvalues_(pV) {
		std::cout << "\t\t\tCFXWisePointerC: Con is called" << std::endl;
	}
	CFXWisePointerC(std::string pV, std::unique_ptr <CFXApple> && capp):Pvalues_(pV), c_apple_(std::move(capp)) {
		std::cout << "\t\t\tCFXWisePointerC: Con is called" << std::endl;
	}

	~CFXWisePointerC() {
		std::cout << "\t\t\tCFXWisePointerC: Before De-Con is called" << std::endl;
		if (!c_apple_) {
			std::cout << "\t\t\t\tc_apple_ is null "  << std::endl;
		} else {
			std::cout << "\t\t\t\tc_apple_ is Not null "  << std::endl;
		}
		std::cout << "\t\t\tCFXWisePointerC: After De-Con is called" << std::endl;
	}
	void display(void) {
		std::cout << "\t\t\tCFXWisePointerC: name is:" << CName_ << std::endl;
		std::cout << "\t\t\tCFXWisePointerC: Pvalues is:" << Pvalues_ << std::endl;
	}
	void displayApple(void) {
		if (c_apple_) {
			c_apple_->display();
		}
	}
private:
	std::string CName_{"CFXWisePointerC"};
	std::string Pvalues_{"Pvalues"};
	const std::unique_ptr <CFXApple> c_apple_;
};
void cfx_use_unique_pointer_by_para_ref(std::unique_ptr<CFXWisePointerC>& pUniquePointer)
{
	std::cout << "\t\tenter " << __func__ << std::endl;
	pUniquePointer->display();
	std::cout << "\t\texit " << __func__ << std::endl;
	return;
}
void cfx_use_unique_pointer_by_para_val(std::unique_ptr<CFXWisePointerC> pUniquePointer)
{
	std::cout << "\t\tenter " << __func__ << std::endl;
	pUniquePointer->display();
	std::cout << "\t\texit " << __func__ << std::endl;
	return;
}
void cfx_unique_pointer(void)
{
//https://www.techiedelight.com/find-name-of-the-calling-function-in-cpp/#:~:text=Find%20name%20of%20the%20calling%20function%20in%20C%2B%2B,3.%20Using%20std%3A%3Asource_location%20...%204%204.%20Using%20boost
	std::string l_calling_func_name = __builtin_FUNCTION();
	std::cout << "\tenter " << __func__ << std::endl;
	//std::cout << "\tcalling function is " << l_calling_func_name << std::endl;
	//                                      class name        paras
	auto l_unique_ptr1 = std::make_unique <CFXWisePointerC>("CFXPvalues");
	std::cout << "\t" << __func__ << ":CFX object pointer "<< l_unique_ptr1.get() << std::endl;
        std::cout << "\t" << __func__ << ":unique pointer pointer "<< &l_unique_ptr1 << std::endl;
	auto l_unique_ptr2 = std::make_unique <int>(19);
	std::cout << "\t" << __func__ << ":CFX object pointer int "<< l_unique_ptr2.get() << std::endl;
	//std::cout << "\t" << __func__ << ":CFX object value int "<< l_unique_ptr2 << std::endl;
        std::cout << "\t" << __func__ << ":unique pointer pointer int"<< &l_unique_ptr2 << std::endl;
	l_unique_ptr1->display();
	// pass reference, pointer in this function, it will be relased after exit
	//cfx_use_unique_pointer_by_para_ref(l_unique_ptr1);
	// pass value, move the pointer, pointer is not in this function
	// it will be relased after exit cfx_use_unique_pointer_by_para_val()
	cfx_use_unique_pointer_by_para_val(std::move(l_unique_ptr1));
	if (l_unique_ptr1 == nullptr) {
		std::cout << "\t" << __func__ << ":l_unique_ptr1 is nullptr after move" << std::endl;
	}
	std::cout << "\texit " << __func__ << std::endl;
	return;
}
void cfx_unique_pointer2(void)
{
/*
	std::unique_ptr p =  std::make_unique<CFXApple>("Fu_shi");
        p->display();

	CFXApple* p_obj_ptr = p.get();
	p_obj_ptr->display();

	std::unique_ptr<CFXApple> p2 = std::move(p);
	p2->display();

	CFXApple* p3 = p2.release();
	p3->display();

	std::unique_ptr p4 =  std::make_unique<CFXApple>("Fu_shi2");
	p4->display();
 	p4.reset();
*/
	//std::unique_ptr p5  {nullptr};
	//long* long_ptr {nullptr};
	std::unique_ptr p5 =  std::make_unique<CFXApple>("Fu_shi5");
	std::unique_ptr p6 =  std::make_unique<CFXWisePointerC>("Fu_shipV", std::move(p5));
	if (!p5) {
		std::cout << "\tcfx_unique_pointer2 P5 is null "  << std::endl;
	}
	p6->displayApple();
	std::cout << "\tcfx_unique_pointer2 start Test reset "  << std::endl;
	auto pReset =  std::make_unique<CFXApple>("Fu_shi_reset");
	pReset.reset();
	if (pReset == nullptr)
		std::cout << "\tcfx_unique_pointer2 pReset is nullptr after reset"  << std::endl;
	std::cout << "\tcfx_unique_pointer2 end Test reset "  << std::endl;
	std::cout << "\n\tcfx_unique_pointer2 start Test move "  << std::endl;
	auto u_ptr1 =  std::make_unique<CFXApple>("Fu_shi_move");
	std::unique_ptr<CFXApple> u_ptr2 = nullptr; 
	u_ptr2 = std::move(u_ptr1);
	if (u_ptr1 == nullptr)
		std::cout << "\tcfx_unique_pointer2 u_ptr1 is nullptr after move"  << std::endl;
	u_ptr2->display();
	std::cout << "\tcfx_unique_pointer2 end Test move "  << std::endl;
	return;
}

/*
 // 创建
// 创建空智能指针
unique_ptr<int> p;
unique_ptr<int> p(nullptr)

// 创建时指定对象
unique_ptr<int> p(new int(1));

// 销毁旧对象（如果有）然后绑定新对象
p.reset(new int(1));

// 用类型为 D 的对象替代默认删除器 std::default_delete
unique_ptr<T,D> p(new int(1));
unique_ptr<T,D> p(new int(1), D());

// 所有权变化
// 释放所有权
int *pi = p.release();

// 转移所有权
p2.reset(p1.release());

// 通过移动转移所有权，转移后 p1 变成空指针
unique_ptr<int> p2 = std::move(p1);

// 显式销毁所指对象，同时智能指针变为空指针。与 p.reset() 等价
p = nullptr;
*/
int main() {
	std::cout << "main: enter" << std::endl;
	//the size of pointer
	char* char_ptr {nullptr};
	char* long_ptr {nullptr};
	//std::unique_ptr<char> u_ptr = std::make_unique<char>('a');
	//std::unique_ptr u_ptr = std::make_unique<char>('a');
	auto u_ptr {std::make_unique<char>('a')};
	std::cout << "main: size of (char_ptr): " << sizeof(char_ptr) << std::endl;
	std::cout << "main: size of (long_ptr): " << sizeof(long_ptr) << std::endl;
	std::cout << "main: size of (std::unique_ptr): " << sizeof(u_ptr) << std::endl;
	//multiple pointer
	int ival = 1024;
	//int* pi = &ival;
	auto* pi = &ival;
	int** ppi = &pi;
	int*** pppi = &ppi;
	std::cout << "main: address ival: " << &ival << " ival value: " << ival << " value: " << ival << std::endl;
	std::cout << "main: address pi: " << &pi << " pi value: " << pi  << " value: " << *pi << std::endl;
	std::cout << "main: address ppi: " << &ppi << " ppi value: " << ppi << " value: " << **ppi << std::endl;
	std::cout << "main: address pppi: " << &pppi << " pppi value: " << pppi << " value: " << ***pppi << std::endl;
	cfx_unique_pointer2();
	std::cout << "main: exit" << std::endl;
	return 0;
}
