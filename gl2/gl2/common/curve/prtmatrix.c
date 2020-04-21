gl_printmatrix(mat)
float mat[4][4];
{
    int i,j;

    for (i = 0; i < 4; i++) {
	for (j = 0; j < 4; j++)
	    printf("%f\t", mat[i][j]);
	printf("\n");
    }
}
