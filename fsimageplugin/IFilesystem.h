#pragma once
#include "../Interface/Object.h"

class IReadOnlyBitmap : public IObject
{
public:
	virtual int64 getBlocksize() = 0;
	virtual bool hasBlock(int64 block) = 0;
	virtual bool hasError(void) = 0;
};

class IFsNextBlockCallback
{
public:
	virtual int64 nextBlock(int64 curr_block) = 0;
	virtual void slowReadWarning(int64 passed_time_ms, int64 curr_block) = 0;
	virtual void waitingForBlockCallback(int64 curr_block) = 0;
};

const int64 fs_error_read_timeout = -1;

class IFilesystem : public IReadOnlyBitmap
{
public:
	virtual int64 getBlocksize(void)=0;
	virtual int64 getSize(void)=0;
	virtual const unsigned char *getBitmap(void)=0;

	virtual bool hasBlock(int64 pBlock)=0;
	
	class IFsBuffer
	{
	public:
		virtual char* getBuf() = 0;
	};
	
	virtual IFsBuffer* readBlock(int64 pBlock, bool* p_has_error = NULL)=0;
	virtual std::vector<int64> readBlocks(int64 pStartBlock, unsigned int n,
		const std::vector<char*>& buffers, unsigned int buffer_offset=0)=0;
	virtual bool hasError(void)=0;
	virtual int64 calculateUsedSpace(void)=0;
	virtual void releaseBuffer(IFsBuffer* buf)=0;

	virtual void shutdownReadahead()=0;

	virtual int64 getOsErrorCode() = 0;

	virtual void logFileChanges(std::string volpath, int64 min_size, char* fc_bitmap) = 0;
};

class FsShutdownHelper
{
public:
	FsShutdownHelper(IFilesystem* fs)
		: fs(fs) {}

	FsShutdownHelper()
		: fs(NULL) {}

	void reset(IFilesystem* pfs) {
		fs=pfs;
	}

	~FsShutdownHelper() {
		if(fs!=NULL) fs->shutdownReadahead();
	}	
private:
	IFilesystem* fs;
};

class fs_buffer
{
public:
	fs_buffer(IFilesystem* fs, IFilesystem::IFsBuffer* buf)
		: fs(fs), buf(buf)
	{ }
	
	~fs_buffer();

	IFilesystem::IFsBuffer* get() {
		return buf;
	}

private:

	fs_buffer(const fs_buffer& other) 
		: fs(other.fs), buf(other.buf)
	{
	}

	fs_buffer& operator=(const fs_buffer& other) {
		fs_buffer temp(other);
		temp.swap(*this);
		return *this;
	}

	void swap(fs_buffer& other)
	{
		std::swap(this->fs, other.fs);
		std::swap(this->buf, other.buf);
	}


	IFilesystem* fs;
	IFilesystem::IFsBuffer* buf;
};

inline fs_buffer::~fs_buffer()
{
	if(buf!=NULL)
	{
		fs->releaseBuffer(buf);
	}
}
