#include <stdio.h>
#include <math.h>
#include "MovingObject.h"

MovingObject::MovingObject()
	{
	MovingObject(QUEUESIZE);
	}

MovingObject::MovingObject(int queueSize)
	{
	if ((queueSize>QUEUESIZE)||(queueSize<1)) queueSize=QUEUESIZE;
	_realQueueSize = queueSize;
	_last = -1;
	_filled=false;
	setMethod('s');
	}

MovingObject::~MovingObject()
	{
	}

int MovingObject::push(float X, float Y)
	{
//	printf("--> Tento di inserire valori %f, %f\n", X, Y);
	_cursor=_last+1;
	if (_cursor >= _realQueueSize)
		{
		_filled=true;
		_cursor = 0;
		}
	_lastValues_X[_cursor]=X;
	_lastValues_Y[_cursor]=Y;
	_last=_cursor;
	return 1;
	}

int MovingObject::setMethod(char m)
	{
	if (!((m=='s')||(m=='w')))
		{
		_method='s';
		return 1;
		}
	_method = m;
	return 0;
	}

int MovingObject::status()
	{
	printf("Method: %c\n", _method);
	printf("First: %d - Last: %d\n", _first(), _last);
	printf("RealQueueSize: %d\n", _realQueueSize);
	printf("Filled: %s\n", _filled?"true":"false");
	printf("Valorized: %s\n", _valorized()?"true":"false");

	int i, count;
	if (_valorized()) {
		for(i=_first(), count=0; _filled?count<_realQueueSize:i<=_last;i++, count++){
			i=i%_realQueueSize;
			printf("   Values[%d]: %f, %f\n", i, _lastValues_X[i], _lastValues_Y[i]);
			}
		}
	printf("Distance: %f\n", coveredDistance());
	printf("Speed: %f\n", speed());
	printf("Direction: %f\n", direction()*180/PI);
	printf("---\n");
	return 0;
	}

float MovingObject::getMA_X()
	{
	return _getMA(_lastValues_X);
	}


float MovingObject::getMA_Y()
	{
	return _getMA(_lastValues_Y);
	}

float MovingObject::coveredDistance()
	{
	return sqrt(pow(_first_X()-_last_X(),2)+pow(_first_Y()-_last_Y(),2));
	}

float MovingObject::_getMA(float *v)
	{
	float sum=0;
	int i, count;
	int coeff;
	int den=0;
	if (_valorized()) {
		for(i=_first(), count=0; _filled?count<_realQueueSize:i<=_last;i++, count++){
			i=i%_realQueueSize;
			coeff=_method=='s'?1:count+1; den+=coeff;
			sum+=v[i]*coeff;
			}
		return sum/den;
		}
	}

bool MovingObject::_valorized()
	{
	return (_last>=0);
	}

int MovingObject::_first()
	{
	if (_filled)
		return (_last+1)%_realQueueSize;
	if (_valorized())
		return 0;
	return -1;
	}


float MovingObject::_first_X()
	{
	return _lastValues_X[_first()];
	}

float MovingObject::_first_Y()
	{
	return _lastValues_Y[_first()];
	}

float MovingObject::_last_X()
	{
	return _lastValues_X[_last];
	}

float MovingObject::_last_Y()
	{
	return _lastValues_X[_last];
	}

int MovingObject::_items()
	{
	return _filled?_realQueueSize:_last+1;
	}

float MovingObject::speed()
	{
	return _valorized()?coveredDistance()/_items()*FRAMERATE:0;
	}

float MovingObject::direction()
	{
	if (_items()>=2)
		{
		return atan( (_last_X()-_first_X()) / (_last_Y()-_first_Y()) );
		}
	else
		return 0;
	}


