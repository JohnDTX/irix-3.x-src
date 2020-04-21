/* emulate the VAX insque/remque instructions
 *
 * If you really care about speed, you will want to re-code in assembly
 * language.  This, however, is rather more portable.
 */

struct qelem {
	struct	qelem *q_forw;
	struct	qelem *q_back;
	char	q_data[1];
};


insque(elem,pred)
register struct qelem *elem, *pred;
{
	register struct qelem *next = pred->q_forw;

	elem->q_forw = next;
	elem->q_back = pred;
	next->q_back = elem;
	pred->q_forw = elem;
}

remque(elem)
register struct qelem *elem;
{
	register struct qelem *pred = elem->q_back;
	register struct qelem *next = elem->q_forw;

	pred->q_forw = next;
	next->q_back = pred;
}
