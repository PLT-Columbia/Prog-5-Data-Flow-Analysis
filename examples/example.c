int sum(int e){
    int res = 0;
    while (res < e){
        if (res %2 == 0) {
            res = res + e;
	}
    }
    return res;
}
