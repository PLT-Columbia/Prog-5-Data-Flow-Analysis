int sum(int a, int e){
	int res = 0;
	while (a < e){
		res = res + e;
		a++;
		e /= 2;
	}
	return res;
}
