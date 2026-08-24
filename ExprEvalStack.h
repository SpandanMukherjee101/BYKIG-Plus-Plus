#include <stdio.h>

struct node
{
	char opr;
	struct node *next;
};

struct node2
{
	struct Value x;
	struct node2 *next;
};

void push( struct node **, char);
char pop( struct node **);

void pushV( struct Value, struct node2 **);
struct Value popV( struct node2 **);

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

void pushV(struct Value x, struct node2 **top)
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

struct Value popV( struct node2 **top)
{
	struct node2 *temp= *top;
	struct Value item;

	if ( temp == NULL )
	{
        item.type = VAL_ERROR;
        item.f = 0.0;
		return item;
	}
	else
	{
		item = temp->x;
		*top = (*top)->next;
		free ( temp ) ;
		return item ;
	}
}