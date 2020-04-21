#include <stdio.h>
#include "string.h"

extern	char *sbrk(int);

#define	assertSame(c1, c2) \
	if (strcmp(c1, c2) == 0) ; else { assertError(__LINE__); }

#define	assertTrue(ex) \
	if (ex) ; else { assertError(__LINE__); }

#define	assertFalse(ex) \
	if (!(ex)) ; else { assertError(__LINE__); }

void
assertError(int line)
{
	fprintf(stderr, "test: string test code failed at line %d\n", line);
	exit(-1);
}

void
testDeclarations()
{
	/* declarations to test constructors */
	string s1;			/* string(); */
	string s2 = s1;			/* string(string&); */
	string s3 = "hi there";		/* string(char*); */

	assertSame(s1.text(), "");
	assertSame(s2.text(), "");
	assertSame(s3.text(), "hi there");
}

void
testAssignments()
{
	string s4 = "a constant";
	string s5, s6;

	s5 = s4;			/* s1 = s2; */
	s6 = "hello world";		/* s1 = "..." */

	assertSame(s4.text(), "a constant");
	assertSame(s5.text(), "a constant");
	assertSame(s6.text(), "hello world");
}

void
testPlus()
{
	string s7 = "one";
	string s8 = "two";
	string s9 = "three";
	string s10, s11, s12;

	s10 = s7 + s8;			/* s1 + s2 */
	s11 = s7 + " plus char*";	/* s1 + "..." */
	s12 = s7 + s8 + s9;		/* s1 + s2 + s3 */

	assertSame(s10.text(), "onetwo");
	assertSame(s11.text(), "one plus char*");
	assertSame(s12.text(), "onetwothree");
}

void
testPlusEquals()
{
	string s13 = "first";
	string s14 = "second";
	string s15;
	string s16 = "third";

	s14 += s13;			/* s1 += s2; */
	s15 += s13;			/* s1 += s2; s1 is null */
	s16 += " plus char*";		/* s1 += "..."; */

	assertSame(s14.text(), "secondfirst");
	assertSame(s15.text(), "first");
	assertSame(s16.text(), "third plus char*");
}

void
testEq()
{
	string s17 = "same";
	string s18 = "different";
	string s19 = "same";
	string s20 = "sameX";
	string s21;

	assertTrue(s17 == s17);
	assertFalse(s17 == s18);
	assertTrue(s17 == s19);
	assertFalse(s17 == s20);
	assertFalse(s17 == s21);
}

void
testNeq()
{
	string s17 = "same";
	string s18 = "different";
	string s19 = "same";
	string s20 = "sameX";
	string s21;

	assertFalse(s17 != s17);
	assertTrue(s17 != s18);
	assertFalse(s17 != s19);
	assertTrue(s17 != s20);
	assertTrue(s17 != s21);
}

void
testIndex()
{
	string s22 = "four";
	string s23;

	assertTrue(s22[0] == 'f');
	assertTrue(s22[1] == 'o');
	assertTrue(s22[2] == 'u');
	assertTrue(s22[3] == 'r');

	s23 = s22;
	s22[0] = 'F';
	assertTrue(s22[0] == 'F');
	assertTrue(s23[0] == 'f');
}

typedef	void	(*fptr)();

void
repeat(fptr test)
{
#ifdef	NOISY
	printf("Brk before test = %x\n", sbrk(0));
#endif
	for (int i = 0; i < 1000; i++)
		(*test)();
#ifdef	NOISY
	printf("Brk after test = %x\n\n", sbrk(0));
#endif
}

main()
{
#ifdef	NOISY
	printf("Brk before tests = %x\n\n", sbrk(0));
#endif
	repeat(testDeclarations);
	repeat(testAssignments);
	repeat(testPlus);
	repeat(testPlusEquals);
	repeat(testEq);
	repeat(testNeq);
	repeat(testIndex);
#ifdef	NOISY
	printf("Brk after tests = %x\n", sbrk(0));
#endif
	exit(0);
}
