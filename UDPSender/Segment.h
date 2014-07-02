
#ifndef SEGMENT_H
#define SEGMENT_H

struct Segment
{
	unsigned short int seqNum;
	char* data;
	DWORD timeOutTick; // The tick when this segment timeout
	bool isSent;
};

#endif