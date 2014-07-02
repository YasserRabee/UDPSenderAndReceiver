
#ifndef WINDOW_H
#define WINDOW_H

#include <deque>
#include"Segment.h"

class Window
{
public:
	Window(unsigned int);
	~Window();

	bool Add(const Segment*);
	bool Ack(unsigned short);


	Segment first()
	{ return sendWindow.front(); }

	Segment at(size_t pos)
	{ return sendWindow.at(pos); }

	void Replace(size_t index, const Segment* seg)
	{ sendWindow.at(index) = *seg; }

	unsigned int count()
	{ return _count; }

	unsigned int maxLength()
	{ return _maxLength; }

	bool isFull()
	{ return _maxLength == _count;	}

	bool hasElems()
	{ return _count > 0; }

	void sendSeg()
	{ _firstUnsent++; }

	unsigned int firstUnsent()
	{ return _firstUnsent; }

	void resendAll()
	{ _firstUnsent = 0;	}

private:
	std::deque<Segment> sendWindow;
	unsigned int _maxLength;
	unsigned int _count;
	unsigned int _firstUnsent;
};

Window::Window(unsigned int length)
{
	_count = 0;
	_maxLength = length;
	_firstUnsent = 0;
}

Window::~Window()
{
}

bool Window::Add(const Segment* seg)
{
	if (_count == _maxLength)
		return false;

	sendWindow.push_back(*seg);
	_count++;
	return true;
}

bool Window::Ack(unsigned short seqNum)
{
	for (size_t i = 0; i < _count; i++)
	{
		if (seqNum == sendWindow.at(i).seqNum)
		{
			for (size_t j = 0; j <= i; j++)
			{
				sendWindow.pop_front();
				_count--;
				_firstUnsent--;
			}
			return true;
		}
	}
	return false;
}

#endif