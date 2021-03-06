#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

namespace rocksdb{

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
time_t GetCurrentTime();

class FileSegInfo{
public:
	uint64_t start_address;
	uint64_t size;
	int link_count;
	FileSegInfo(uint64_t start_address_, uint64_t size_) {
		this->start_address =start_address_;
		this->size =size_;
		link_count = 1;
	}

	uint64_t GetStartAddress(){
		return start_address;
	}
};

class FileBuffer{
public:
	char* buffer;
	uint64_t b_size;
	uint64_t start_address;
	uint64_t offset;
	FileBuffer(){
		buffer = new char[8192];
		b_size = 0;
		start_address = 0;
		offset = 0;
	}
	~FileBuffer(){
		delete [] buffer;
	}
};

class Node{
public:
	Node(std::string* name_, bool isfile_, Node* parent_) :
		children(NULL), file_info(NULL){
		this->name =name_;
		this->isfile = isfile_;
		this->parent = parent_;
		if(isfile_){
			file_info = new std::vector<FileSegInfo*>();
			file_info->clear();
		}
		else{
			children = new std::list<Node*>();
			children->clear();
		}
		last_modified_time = GetCurrentTime();
		lock = false;
		link_count = 1;
		size = 0;
		file_buf = new FileBuffer();
	}
	~Node(){
		if(isfile){
			if(file_info->at(0)->link_count == 0){
				for(unsigned int i = 0; i < file_info->size(); i++){
					delete file_info->at(i);
				}
				delete file_info;
				delete file_buf;
			}
		}
		else{
			if(link_count == 0) delete children;
		}
		delete name;
	}
	Node* parent;
	std::string* name;
	bool isfile;
	time_t last_modified_time;
	std::list<Node*>* children;
	std::vector<FileSegInfo*>* file_info;
	FileBuffer* file_buf;
	uint64_t GetSize();
	bool lock;
	int link_count;
	uint64_t size;
};

class GlobalFileTableTree{
public:
	std::vector<unsigned char>* free_page_bitmap; // until now, it associates a char with a page

	GlobalFileTableTree(uint64_t page_size_){
		free_page_bitmap = new std::vector<unsigned char>();
		free_page_bitmap->push_back(0);
		root = new Node(new std::string(""), false, NULL);
		cwd = "/";
		page_size = page_size_;
		CreateDir("tmp");
        CreateDir("proc");
        CreateDir("proc/sys");
        CreateDir("proc/sys/kernel");
        CreateDir("proc/sys/kernel/random");
        CreateFile("proc/sys/kernel/random/uuid");
	}
	~GlobalFileTableTree(){
		delete free_page_bitmap;
		RecursiveRemoveDir(root);
	}

	Node* CreateDir(const std::string &name);
	Node* CreateFile(const std::string &name);
	int Link(const std::string &src, const std::string &target);
	int DeleteDir(const std::string &name);
	int DeleteFile(const std::string &name);
	int Lock(const std::string &name, bool lock);

	Node* GetNode(const std::string &name);
	int FreeAllocatedPage(Node* node);
	// go to the target directory. and return the target directory node.
	Node* DirectoryTraverse(const std::string &path, bool isCreate);
	Node* FindChild(Node* dir, const std::string &name);
	bool RecursiveRemoveDir(Node* cur);

	bool print(Node* cur, std::string indent);
	void printAll();

	std::string cwd;
private:
	Node* root;
	uint64_t page_size;
};

} // namespace rocksdb
