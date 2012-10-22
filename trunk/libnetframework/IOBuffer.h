#ifndef _LIB_IOBUFFER_H_20120816_LIUYONGJIN
#define _LIB_IOBUFFER_H_20120816_LIUYONGJIN
#include <stdio.h>

#define BUFFER_SIZE 1024  //缓冲区初始化大小为1k

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///////                                                                   ///////
///////                 IOBuffer用于读写的缓冲区                         ///////
///////       当空间不够时,会按2倍大小扩展,直到有足够大的空间          ///////
///////       或者达到最大值.                                            ///////
///////                                                                   ///////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
class IOBuffer
{
public:
	IOBuffer(unsigned int init_size=BUFFER_SIZE);
	~IOBuffer();

	//打开大小为size的缓冲区用于写.
	//成功:返回缓存取到指针; 失败:返回NULL;
	char *write_open(unsigned int size);

	//关闭写缓冲区;
	//成功:返回true, 设置成功写入的字节数;
	//失败:返回false,缓冲区数据没有任何改变;
	bool write_close(unsigned int write_size);

	//打开缓冲区用于读
	//成功:返回缓冲区到指针,设置size为可读缓冲区中数据的大小
	//失败:返回NULL
	char *read_open(unsigned int &size);

	//关闭读缓冲区;
	//成功:返回true,设置成功读出的字节数
	//失败:返回false,缓冲区数据没有任何改变;
	bool read_close(unsigned int read_size);

	//返回可读数据的大小
	unsigned int get_size(){return m_data_size;}

	//返回有效数据区中偏移位置为offset,大小为size的有效数据缓冲区.
	//成功:返回缓冲区指针
	//失败:返回NULL
	char* seek(unsigned int offset, unsigned int size){return (offset+size>m_data_size?NULL:m_data+offset);}

	//从有效数据区尾部将数据截掉size字节
	//成功:返回true, 尾部的size字节无效
	//失败:返回false, 数据没有任何变化
	bool truncate(unsigned int size){return size>m_data_size?false:(m_data_size-=size,true);}
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
	char *buffer = io_buffer.write_open(size);
	if(buffer != NULL)
	{
		memcpy(buffer, data, size);
		io_buffer.write_close(size);
	}

	buffer = io_buffer.read_open(size);
	if(buffer != NULL)
	{
		memcpy(data, buffer, size);
		io_buffer.read_close(size);
	}

	return 0;
}
*/

#endif //_LIB_BUFFER_H_20120816_LIUYONGJIN
