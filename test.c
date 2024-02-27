#include "defer.h"


int fn3(int flip)
{
	printf("%s\n", __func__);
	defer {
		printf("%s second\n", __func__);
	}
	defer {
		printf("%s first\n", __func__);
		return 2;
	}
	printf("%s ret\n", __func__);
	if(flip)
		return 3;	
	else
		return 4;
		
}


void fn2()
{
	printf("%s\n", __func__);
	defer {
		printf("%s second\n", __func__);
	}
	int ret = fn3(2);
	printf("fn3 %d\n", ret);
	defer {
		printf("%s first\n", __func__);
	}
	printf("%s ret\n", __func__);
	return;
}

int fn()
{
	printf("%s\n", __func__);
	defer {
		printf("%s second\n", __func__);
	}
	fn2();
	fn3(1);
	defer {
		printf("%s first\n", __func__);
	}
	return 1;
}


int main()
{
	printf("start\n");
	int x = fn();	
	printf("end x = %d\n", x);
	printf("nest %d\n", _defer_nest);
	for(int n = 0; n < DEFER_NEST_MAX; n++) {
		printf("%d %p func %p\n", _defer_state[n].count, _defer_state[n].return_loc, _defer_func[n]);
	}
	return 0;
}

