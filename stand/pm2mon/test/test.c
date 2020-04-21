
test() {
	union {
		int i;
		int (*(*go)());
	}u;
		u.i = 4;
		(**u.go)();
}
