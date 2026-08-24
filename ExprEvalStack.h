#include <stdio.h>

struct node
{
	char opr;
	struct node *next;
};

struct node2
{
	float x;
	struct node2 *next;
};

void push( struct node **, char);
char pop( struct node **);

void pushF( float, struct node2 **);
float popF( struct node2 **);

void push (struct node **top, char Op)
{
    struct node *nnode, *temp= *top;

    if (temp == NULL)
    {
        *top= (struct node *)malloc(sizeof(struct node));
        (*top)->opr= Op;
        (*top)->next= NULL;
    }
    else
    {

        nnode=(struct node *)malloc(sizeof(struct node));
        nnode->opr= Op;
        nnode->next= (*top);
        (*top)= nnode;
    }
}

char pop( struct node **top)
{
	struct node *temp= *top;
	char item;

	if ( temp == NULL )
	{
		return ' ';
	}
	else
	{
		item = temp->opr;
		*top = ( *top ) -> next;
		free ( temp ) ;
		return item ;
	}
}

void pushF(float x, struct node2 **top)
{
    struct node2 *nnode, *temp= *top;

    if (temp == NULL)
    {
        *top= (struct node2 *)malloc(sizeof(struct node2));
        (*top)->x= x;
        (*top)->next= NULL;
    }
    else
    {

        nnode=(struct node2 *)malloc(sizeof(struct node2));
        nnode->x= x;
        nnode->next= (*top);
        (*top)= nnode;
    }
}

float popF( struct node2 **top)
{
	struct node2 *temp= *top;
	float item;

	if ( temp == NULL )
	{
		return ' ';
	}
	else
	{
		item = temp->x;
		*top = (*top)->next;
		free ( temp ) ;
		return item ;
	}
}