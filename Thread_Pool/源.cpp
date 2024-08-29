#include <iostream>
#include "threadpool.hpp"
using namespace std;
int main()
{
	ThreadPool pool(4);
	for (int i = 0; i < 20; i++)
	{
		auto rsfu0 = pool.enqueue([](int a, int b)->int {
			cout << "当前线程: " << this_thread::get_id() << endl;
			return a + b;
			}, 10 * i, 10 * i);
		cout << "thread rs:" << rsfu0.get() << endl;
	}
	return 0;
}