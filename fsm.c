#include <stdlib.h>

#include "fsm.h"
#include "vector.h"

FSM *FSMCreate(Grammar *grammar)
{
    FSM *fsm = (FSM *)malloc(sizeof(FSM));
    fsm->Grammar = grammar;
    fsm->States = VectorCreate();
    return fsm;
}

void FSMDelete(FSM *fsm)
{

}

void FSMBuildStates(FSM *fsm)
{
    if(fsm->States) VectorDelete(fsm->States);
    free(fsm);
}
