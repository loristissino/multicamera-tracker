#include <stdio.h>
#include "MovingObject.h"

int main(int argc, char **argv)
{

MovingObject *a= new MovingObject(3);

a->status();
a->setMethod('w');

int x, y;

for (x=10,y=-10;x>-10; x--, y++)
	{
	a->push((float)x, (float)y);
	a->status();
	}

delete a;
return 0;

}
