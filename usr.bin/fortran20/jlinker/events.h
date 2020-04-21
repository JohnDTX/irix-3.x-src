#ifndef events_h
#define events_h
typedef struct Event *Event;
typedef struct Breakpoint *Breakpoint;

boolean inst_tracing;
boolean single_stepping;
boolean isstopped;

#include "symbols.h"

Symbol linesym;
Symbol procsym;
Symbol pcsym;
Symbol retaddrsym;

#define addevent(cond, cmdlist) event_alloc(false, cond, cmdlist)
#define event_once(cond, cmdlist) event_alloc(true, cond, cmdlist)

bpinit(/*  */);
Event event_alloc(/* istmp, econd, cmdlist */);
boolean delevent (/* id */);
Symbol tcontainer(/* exp */);
boolean canskip(/* f */);
status(/*  */);
printevent(/* e */);
bpfree(/*  */);
boolean bpact(/*  */);
traceon(/* inst, event, cmdlist */);
traceoff(/* id */);
printnews(/*  */);
callnews(/* iscall */);
printifchanged(/* p */);
stopifchanged(/* p */);
trfree(/*  */);
fixbps(/*  */);
setallbps(/*  */);
unsetallbps(/*  */);
#endif
