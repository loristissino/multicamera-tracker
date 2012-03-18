#define QUEUESIZE 10
#define FRAMERATE 25
#define PI 3.14159265

class MovingObject {

	private:
		float _lastValues_X[QUEUESIZE];
		float _lastValues_Y[QUEUESIZE];

		int _last;
		int _first();
		int _cursor;
		int _realQueueSize;
		int _items();
		bool _filled;
		bool _valorized();
		char _method;  // 's' for simple, 'w' for weighted

		float _getMA(float *v);
		float _first_X();
		float _first_Y();
		float _last_X();
		float _last_Y();


	public:

		int push(float X, float Y);
		int setMethod(char m);
		int status();
		float getMA_X();
		float getMA_Y();
		float coveredDistance();
		float speed();
		float direction();
		 MovingObject();
		 MovingObject(int queueSize);
		~MovingObject();
};


