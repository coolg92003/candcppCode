#include <stdio.h>
#include <iostream>
class C2PYTHON {
public:
	void print_any(const char* whatIs) const {
		std::cout << "The printed stuff is < " << whatIs << " >" << std::endl;
	}
};
extern "C" {  // Required for C++ code to be compatible with C calling conventions
    void print_any_char(const char* what) {
        //printf("Hello, World from C!\n");
        C2PYTHON c2p;
	c2p.print_any(what);
    }
}
