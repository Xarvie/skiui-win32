#include <iostream>

#include "skiui-lite.h"

//==================================================================================
///98年C++写法 需要独立的函数实现裸指针回调 需要#define SKIUI_LITE_CXX98
int* global_i = NULL;
void func()
{
    int& i = *global_i;
    std::cout<<"啦啦肥按钮被点击了,i的值为："<< i <<std::endl;
}
void CXX98()
{
    global::init(NULL, NULL);
    Window* win1 = Window::create(NULL, "SkiUI-lite 窗口 Demo");
    Label* label1 = Label::create(win1, ("你再说一遍"));
    Label* label2 = Label::create(win1, ("有种说两遍"), 0, 30);
    Button* button1 = Button::create(win1, ("啦啦肥"), 0, 60);

    int i = 1;
    global_i = &i;
    button1->evClick(func);

    global::loop();
}
//==================================================================================
//==================================================================================
//现代化C++写法 支持C++98裸指针，支持C++03/11/14/17/20
void CXX11()
{
    global::init(nullptr, nullptr);
    auto win1 = Window::create(nullptr, "SkiUI-lite 窗口 Demo");
    auto label1 = Label::create(win1, ("你再说一遍"));
    auto label2 = Label::create(win1, ("有种说两遍"), 0, 30);
    auto button1 = Button::create(win1, ("啦啦肥"), 0, 60);

    int i = 1;
    button1->evClick([&] { std::cout << "啦啦肥按钮被点击了,i的值为：" << i << std::endl; });

    global::loop();
}
//==================================================================================

int main() {
//  return CXX98();
    return CXX11();
}
