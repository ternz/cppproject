#include <iostream>
#include "concurrency/Mutex.h"

using namespace std;
using namespace concurrency;

int main(int argc, char* argv[]) {
	cout<<"befor mutex init"<<endl; 
	Mutex mutex;
	cout<<"after mutex init"<<endl; 
	{
		Guard guard(mutex);
		cout<<"in critical zone"<<endl;
	}
}