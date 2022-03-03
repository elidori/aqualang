#ifndef _PROGRESS_INFO_RECEIVER_HEADER_
#define _PROGRESS_INFO_RECEIVER_HEADER_

class IProgressInfoReceiver
{
protected:
	virtual ~IProgressInfoReceiver() {}
public:
	virtual void ProgressReport(int BytesDownloaded, int TotalBytesToDownload) = 0;
};

#endif // _PROGRESS_INFO_RECEIVER_HEADER_