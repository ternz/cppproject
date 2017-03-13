#include <unistd.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "concurrency/concurrency.h"

using namespace std;
using namespace concurrency;

using boost::shared_ptr;

int counter = 0;
Mutex mutex;

void threadfunc() {
	while(counter < 10) {
		cout<<"counter: "<<counter<<endl;
		usleep(5000);
		++counter;
		usleep(0);
	}
}

void* threadfunc_wiftlock(void *num) {
	while(counter < 10) {
		{
			usleep(5000);
			Guard guard(mutex);
			if(counter == 10) break;
			cout<<*((int*)num)<<"counter: "<<counter<<endl;
			usleep(5000);
			++counter;
			usleep(5000);
		}
	}
}

int main(int argc, char* argv[]) {
	cout<<"run threads no lock"<<endl;
	shared_ptr<PosixThreadFactory> threadFactory(new PosixThreadFactory());
	threadFactory->setDetached(false);
	shared_ptr<Thread> thread1 = threadFactory->newThread(FunctionRunner::create(threadfunc));
	shared_ptr<Thread> thread2 = threadFactory->newThread(FunctionRunner::create(threadfunc));
	thread1->start();
	thread2->start();
	thread1->join();
	thread2->join();

	counter = 0;

	cout<<"run threads wift lock"<<endl;
	int n3=3, n4=4;
	shared_ptr<Thread> thread3 = threadFactory->newThread(FunctionRunner::create(threadfunc_wiftlock, &n3));
	shared_ptr<Thread> thread4 = threadFactory->newThread(FunctionRunner::create(threadfunc_wiftlock, &n4));
	thread3->start();
	thread4->start();
	thread3->join();
	thread4->join();
}