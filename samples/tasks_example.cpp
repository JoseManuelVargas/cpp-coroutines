#include <tasks/task.hpp>

using namespace jmvm::tasks;

task<> fun7()
{
    std::cout << "    inner not wait void job with exception" << std::endl;
    throw std::exception("Dummy void exception");
}

task<int> fun6()
{
    std::cout << "    inner not wait int job with exception" << std::endl;
    throw std::exception("Dummy int exception");
}

task<int> fun4()
{
    std::cout << "    inner not wait int job" << std::endl;
    co_return 2;
}

task<int> fun5()
{
    int x = co_await fun4();
    std::cout << "    inner not wait int job" << std::endl;
    co_return 10 + x;
}

task<> fun3()
{
    int x = co_await fun4() + co_await fun5();
    std::cout << "    inner not wait job calling int jobs: " << x << std::endl;
    co_await std::suspend_always{};
    try
    {
        int y = co_await fun6();
    }
    catch (const std::exception& ex)
    {
        std::cout << "    inner exception: " << ex.what() << std::endl;
    }
    co_return;
}

task<> fun1()
{
    try
    {
        co_await fun7();
    }
    catch (const std::exception& ex)
    {
        std::cout << "    inner void exception: " << ex.what() << std::endl;
    }
    co_await std::suspend_always{};
    std::cout << "    inner first job" << std::endl;
    co_await std::suspend_always{};
}

task<> fun2()
{
    std::cout << "    inner second job" << std::endl;
    co_await std::suspend_always{};
}

task<> fun()
{
    co_await fun1();
    std::cout << "  first job" << std::endl;
    co_await std::suspend_always{};
    std::cout << "  second job" << std::endl;
    co_await std::suspend_always{};
    std::cout << "  third job" << std::endl;
    co_await std::suspend_always{};

    co_await fun2();

    std::cout << "  fourth job" << std::endl;
    co_await std::suspend_always{};

    co_await fun3();
}

int main()
{
    auto f = []() -> task<>
    {
        co_await fun();
        std::cout << "After awaiting" << std::endl;
    }();
    f.resume();

    auto f2 = []() -> task<int>
    {
        int y = co_await fun5();
        std::cout << "After awaiting: y = " << y << std::endl;
    }();
    f2.resume();
    std::cout << "Finish" << std::endl;
}
