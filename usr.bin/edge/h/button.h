typedef	struct {
	Pane	pane;
	int	b_state;
	int	b_oncolor;
	int	b_offcolor;
} Button;

/* procedures */
extern	Button	*NewButton();
extern	void	FreeButton();
extern	int	DrawButton();
