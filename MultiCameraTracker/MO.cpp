#include "StdAfx.h"
#include "MO.h"

#include <stdio.h>
#include <math.h>



int MovingObject::_init(int queueSize, char method, int frameRate)
	{
	_realQueueSize = queueSize;
	_last = -1;
	_filled=false;
	_count = 0;
	_cursor = 0;
	setMethod(method);
	setFrameRate(frameRate);
	return 0;
	}

MovingObject::MovingObject()
	{
	_init(DEFAULTQUEUESIZE, DEFAULTMETHOD, DEFAULTFRAMERATE);
	}

MovingObject::MovingObject(int queueSize)
	{
	if (queueSize>MAXQUEUESIZE) queueSize=DEFAULTQUEUESIZE;
	if (queueSize<2) queueSize=2;
	_init(queueSize, DEFAULTMETHOD, DEFAULTFRAMERATE);

	}

MovingObject::~MovingObject()
	{
	}

int MovingObject::pushCoordinates(float X, float Y)
	{

		if (_count==0){
			_previous_MA_X= X;
			_previous_MA_Y = Y;
		}
		else {
			_previous_MA_X = _current_MA_X;
			_previous_MA_Y = _current_MA_Y;
		}

	_cursor=_last+1;
	if (_cursor >= _realQueueSize)
		{
		_filled=true;
		_cursor = 0;
		}
	_lastValues_X[_cursor]=X;
	_lastValues_Y[_cursor]=Y;
	_last=_cursor;

	_count++;

	_current_MA_X = _getMA(_lastValues_X);
	_current_MA_Y = _getMA(_lastValues_Y);

	_coveredDistance = sqrt(pow(_current_MA_X-_previous_MA_X,2)+pow(_current_MA_Y-_previous_MA_Y,2));


	if (_items()>=2)
		{
		_direction = atan((_current_MA_X-_previous_MA_X)/(_current_MA_Y-_previous_MA_Y));
/*
		if ((_current_MA_Y-_previous_MA_Y)>0)
			_direction-=PI;
		if (_direction>PI)
			_direction=PI-_direction;
		else
			_direction=3*PI-_direction;
		if (_direction>2*PI)
			_direction-=(2*PI);
		}

*/
		if ((_current_MA_Y-_previous_MA_Y)>0)		
			_direction-=PI;

		if (_direction<0)
			_direction+=(2*PI);

		if (_direction<=PI)
			_direction+=PI;
		else
			_direction-=PI;
		}

	else
		_direction=0;
	return 0;
	}

int MovingObject::removeCoordinates()
{
	TRACE("Rimuovo coordinate...\n");
	int i, count, items;

	double _buffer_lastValues_X[MAXQUEUESIZE];
	double _buffer_lastValues_Y[MAXQUEUESIZE];

	if (_valorized()) {
		for(i=_first(), count=0; _filled?count<_realQueueSize:i<=_last;i++, count++)
			{
			i=i%_realQueueSize;
			_buffer_lastValues_X[count]=_lastValues_X[i];
			_buffer_lastValues_Y[count]=_lastValues_Y[i];
			}

		items=_items();
		resetCount();
		for(count=1;count<items;count++)
			{
//			TRACE("   ... nel buffer[%d]: %f\n", count, _buffer_lastValues_X[count]);
			pushCoordinates(_buffer_lastValues_X[count], _buffer_lastValues_Y[count]);
			}

		}



	return 0;
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

int MovingObject::setFrameRate(int FrameRate)
	{
	if (FrameRate<=0) FrameRate=1;
	_frameRate = FrameRate;
	return 0;
	}


int MovingObject::status()
	{
	TRACE("Method: %c\n", _method);
	TRACE("First: %d - Last: %d\n", _first(), _last);
	TRACE("RealQueueSize: %d\n", _realQueueSize);
	TRACE("Filled: %s\n", _filled?"true":"false");
	TRACE("Valorized: %s\n", _valorized()?"true":"false");
	TRACE("Count: %d\n", _count);


	int i, count;
	if (_valorized()) {
		for(i=_first(), count=0; _filled?count<_realQueueSize:i<=_last;i++, count++){
			i=i%_realQueueSize;
			TRACE("   Values[%d]: %f, %f\n", i, _lastValues_X[i], _lastValues_Y[i]);
			}

		}

	TRACE("Distance: %f\n", coveredDistance());
	TRACE("Speed: %f\n", speed());
	TRACE("Direction: %f\n", direction());
	TRACE("Previous coords: %f, %f\n", getPreviousMA_X(), getPreviousMA_Y());
	TRACE("Current coords: %f, %f\n", getCurrentMA_X(), getCurrentMA_Y());


	TRACE("---\n");
	return 0;
	}

double MovingObject::getCurrentMA_X()
	{
	return _current_MA_X;
	}


double MovingObject::getCurrentMA_Y()
	{
	return _current_MA_Y;
	}

double MovingObject::getPreviousMA_X()
	{
	return _previous_MA_X;
	}


double MovingObject::getPreviousMA_Y()
	{
	return _previous_MA_Y;
	}


double MovingObject::coveredDistance()
	{
		return _coveredDistance;
	}

double MovingObject::_getMA(double *v)
	{
	double sum=0;
	int i, count;
	int coeff;
	int den=0;

	if (_valorized()) {
		for(i=_first(), count=0; _filled?count<_realQueueSize:i<=_last;i++, count++){
			i=i%_realQueueSize;
			coeff=_method=='s'?1:count+1;
			den+=coeff;
			sum+=v[i]*coeff;
			}
		return sum/den;
		}
	else
		return 0;
	}

bool MovingObject::_valorized()
	{
	return (_count>0);
	}

int MovingObject::_first()
	{
	if (_filled)
		return (_last+1)%_realQueueSize;
	if (_valorized())
		return 0;
	return -1;
	}


unsigned long MovingObject::getCount()
	{
	return _count;	
	}

int MovingObject::resetCount()
	{
	_init(_realQueueSize, _method, _frameRate);
	_count = 0;	
	return 0;
	}

int MovingObject::_items()
	{
	return _filled?_realQueueSize:_last+1;
	}

double MovingObject::speed()
	{
	return _valorized()?_coveredDistance*_frameRate:0;
	}

double MovingObject::direction()
	{
		return _direction;
	}


