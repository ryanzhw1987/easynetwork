#ifndef _LIB_IOBUFFER_H_20120816_LIUYONGJIN
#define _LIB_IOBUFFER_H_20120816_LIUYONGJIN
#include <stdio.h>

#define BUFFER_SIZE 1024  //缓冲区初始化大小为1k

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///////                                                                   ///////
///////                 IOBuffer用于读写的缓冲区                          ///////
///////       当空间不够时,会按2倍大小扩展,直到有足够大的空间             ///////
///////       或者达到最大值.                                             ///////
///////                                                                   ///////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class IOBuffer
{
public:
	IOBuffer(unsigned int size=BUFFER_SIZE);
	~IOBuffer();

	//开始写, 获取一个大小为size的缓冲用于写, 成功返回可写的缓冲区, 失败返回NULL,表示没有足够内存
	char* get_write_buffer(unsigned int size);

	//写结束, 设置本次写入多少数据, 成功返回true, 失败返回false
  	bool set_write_size(unsigned int size);

	//将写入的数据回滚size个字节.
	//返回值:
	//true: 回滚成功
	//false: 回滚失败, 数据没有变化
	bool write_rollback(unsigned int size);

	//开始读, 获取用于读的缓冲区, size返回可读取缓冲区的大小.如果没有数据可读, 返回NULL.
	char* get_read_buffer(unsigned int *data_size);

	//读结束, 设置本次读取多少数据, 成功返回true, 失败返回false
	bool set_read_size(unsigned int size);

	//返回可读数据的大小
	unsigned int get_data_size(){return m_data_size;}

	//获取有效数据区offset位置的buffer
	char* seek(unsigned int offset){return offset>m_data_size?NULL:m_data+offset;}
private:
	char *m_buffer; //缓冲区
	unsigned int m_buffer_size;  //缓冲区大小

	char *m_data;
	unsigned int m_data_size;
};

/*sample
int main()
{
	char data[] = "hello buffer";
	unsigned int size = sizeof(data);

	IOBuffer io_buffer;
	char *buffer = io_buffer.write_begin(size);
	if(buffer != NULL)
	{
		memcpy(buffer, data, size);
		io_buffer.write_end(size);
	}

	buffer = io_buffer.read_begin(&size);
	if(buffer != NULL)
	{
		memcpy(data, buffer, size);
		io_buffer.read_end(size);
	}

	return 0;
}
*/

#endif //_LIB_BUFFER_H_20120816_LIUYONGJIN
