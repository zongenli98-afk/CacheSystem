#include <iostream>
#include <thread>
#include <string> // 包含string，尽管当前代码未使用，但好的实践是包含所有依赖

using namespace std;

int main() {
    // 1. 获取硬件并发数 (可选，但很有用)
    unsigned c = thread::hardware_concurrency(); 
    cout << "Hardware concurrency (CPU cores/threads): " 
         << (c > 0 ? to_string(c) : "Unknown") 
         << endl;

    // 2. 线程体：Lambda 函数
    auto lambdaFn = [](){     
        // 线程执行的实际代码
        cout << "Hello multithreading from thread " 
             << this_thread::get_id() << endl;
    };

    // 3. 创建并启动线程
    cout << "Main thread (" << this_thread::get_id() << ") is launching a new thread." << endl;
    thread t(lambdaFn);       

    // 4. 等待线程结束
    // t.join() 是阻塞的，主线程会停在这里直到新线程t完成执行
    t.join();                 

    cout << "Thread t finished execution. Main thread continues." << endl;

    return 0;
}