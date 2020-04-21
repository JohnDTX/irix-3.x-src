
struct first_s { int i,j,k; } fs;
struct second_s { short i0,i1,j0,j1,k0,k1;} ss;
typedef struct first_s *first_p;
typedef struct first_s first_t;
typedef struct second_s *second_p;
first_p fp;
second_p sp;

/*
assign fp=first_p(&ss)
assign fp=first_p(sp)
assign fp=sp
assign fp=&ss
*/

main() {
	int i;

	i = 4;
}
