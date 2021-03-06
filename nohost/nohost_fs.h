#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>

#include "nohost_global_file_table.h"

#ifdef ENABLE_LIBFTL

#include "bdbm_drv.h"
#include "umemory.h"
#include "params.h"
#include "ftl_params.h"
#include "debug.h"
#include "userio.h"
#include "ufile.h"
#include "devices.h"

extern bdbm_drv_info_t* _bdi;

#endif

namespace rocksdb{

struct OpenFileEntry{
	Node* node;
	off_t r_offset;
	off_t w_offset;
	std::list<Node*>::iterator entry_list_iter;
	OpenFileEntry(Node* node_, off_t r_offset_, off_t w_offset_){
		this->node =node_;
		this->r_offset =r_offset_;
		this->w_offset =w_offset_;
		if(!node->isfile)
			this->entry_list_iter = node->children->begin();
	}
};

class NoHostFs{
private:
	std::vector<OpenFileEntry*>* open_file_table;
	int flash_fd;
	uint64_t segment_size;
	uint64_t GetFreeBlockAddress();

public:
	GlobalFileTableTree* global_file_tree; // it must be private!!!!

	NoHostFs(uint64_t assign_size);
	~NoHostFs();
	int Open(std::string name, char type);
//	ssize_t Write(int fd, const char* buf, size_t size);
	ssize_t Write(int fd, const char* buf, size_t size);
//	long int Read(int fd, char* buf, size_t size);
	ssize_t ReadHelper(int fd, char* buf, size_t size);
	size_t SequentialRead(int fd, char* buf, size_t size);
	ssize_t Pread(int fd, char* buf, size_t size, uint64_t absolute_offset);
	ssize_t BufferWrite(OpenFileEntry* entry, FileSegInfo* finfo, const char* buf, uint64_t dsize, uint64_t offset, size_t page_unit);
	ssize_t BufferRead(OpenFileEntry* entry, FileSegInfo* finfo, char* buf, uint64_t dsize, uint64_t offset, size_t page_unit, bool ispread);

	int Close(int fd);
	int Rename(std::string old_name, std::string name);
//	int Access(std::string name);
//	Node* ReadDir(int fd);
	int DeleteFile(std::string name);
	int DeleteDir(std::string name);
//	int CreateFile(std::string name);
	int CreateDir(std::string name);
//	bool DirExists(std::string name);
	uint64_t GetFileSize(std::string name);
	time_t GetFileModificationTime(std::string name);
	int Link(std::string src, std::string target);
	int IsEof(int dfd);
	off_t Lseek(int fd, off_t n);
	int Free(Node* node);
//	int Lock(std::string name, bool lock);
	std::string GetAbsolutePath();
    int  GetFd();
};


}// namespace rocksdb
