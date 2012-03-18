#pragma once

#define MAXQUEUESIZE 250
#define DEFAULTQUEUESIZE 12
#define DEFAULTMETHOD 's'
#define DEFAULTFRAMERATE 25
#define PI 3.14159265

class MovingObject {

	private:
		double _lastValues_X[MAXQUEUESIZE];
		double _lastValues_Y[MAXQUEUESIZE];
//		double _lastValues_dir[MAXQUEUESIZE];

		double _current_MA_X;
		double _current_MA_Y;

		double _previous_MA_X;
		double _previous_MA_Y;

		double _coveredDistance;
		double _direction;

		unsigned long _count;

		int _frameRate;

		int _init(int queueSize, char method, int frameRate);
		int _last;
		int _first();
		int _cursor;
		int _realQueueSize;
		int _items();
		bool _filled;
		bool _valorized();
		char _method;  // 's' for simple, 'w' for weighted

		double _getMA(double *v);


	public:

		int pushCoordinates(float X, float Y);
		int removeCoordinates();
		int setMethod(char m);
		int setFrameRate(int frameRate);
		int status();
		unsigned long getCount();
		int resetCount();
		double getCurrentMA_X();
		double getCurrentMA_Y();
		double getPreviousMA_X();
		double getPreviousMA_Y();
		double coveredDistance();
		double speed();
		double direction();
		 MovingObject();
		 MovingObject(int queueSize);
		~MovingObject();
};


