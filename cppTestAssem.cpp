#include <iostream>
#include <string>
#include <memory>
class CFXV {
    virtual long add_p(long a, long b)=0;
};
class CFXC : public CFXV {
public:
    long add_p (long a, long b) override {
        return (a+b);
    }
    long return_md() const {
        return md_;
    }
private:
    long md_ = 18;
};
class CFXD : public CFXV {
public:
    long add_p (long a, long b) override {
        return (a+b+2);
    }
    long return_md() const {
        return mdd_;
    }
private:
    long mdd_ = 19;
};
int main  () {
    std::cout << "Hello Cliff Fuxuan Chen\n";
    auto cfxc_shared = std::make_shared<CFXC>();
    std::cout << cfxc_shared->return_md() << std::endl;
    auto cfxd_unique = std::make_unique<CFXD>();
    std::cout << cfxd_unique->return_md() << std::endl;
    return 0;
}
