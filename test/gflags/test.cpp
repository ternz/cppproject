#include <stdlib.h>
#include <iostream>
#include <gflags/gflags.h>
using namespace std;

DEFINE_bool(bval, false, "boolean value");
DEFINE_int32(i32val, 0, "int32 value");
DEFINE_int64(i64val, 0, "int64 value");
DEFINE_uint64(u64val, 0, "uint64 value");
DEFINE_double(dval, 0,"double value");
DEFINE_string(sval, "", "string value");

int main(int argc, char* argv[]) {
	google::ParseCommandLineFlags(&argc, &argv, true);
	cout<<"bval: "<<FLAGS_bval<<endl
		<<"i32val: "<<FLAGS_i32val<<endl
		<<"i64val: "<<FLAGS_i64val<<endl
		<<"u64val: "<<FLAGS_u64val<<endl
		<<"dval: "<<FLAGS_dval<<endl
		<<"sval: "<<FLAGS_sval<<endl;
}
