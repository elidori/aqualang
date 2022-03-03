#ifndef _I_THREAD_RELEATED_OBJECT_HEADER_
#define _I_THREAD_RELEATED_OBJECT_HEADER_

class IThreadReleatedObject
{
protected:
	virtual ~IThreadReleatedObject() {}
public:
	virtual bool OpenInThreadContext() = 0;
};

#endif // _I_THREAD_RELEATED_OBJECT_HEADER_