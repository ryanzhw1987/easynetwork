#ifndef _LIB_IOBUFFER_H_20120816_LIUYONGJIN
#define _LIB_IOBUFFER_H_20120816_LIUYONGJIN


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024  //缓冲区初始化大小为1k
#define MAX_BUFFER_SIZE 10*1024*1024   //缓冲区最大10M

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////                                                             ///////
///////                 IOBuffer用于读写的缓冲区                    ///////
///////       当空间不够时,会按2倍大小扩展,直到有足够大的空间       ///////
///////       或者达到最大值.                                       ///////
///////                                                             ///////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
class IOBuffer
{
public:
    IOBuffer(unsigned int size=BUFFER_SIZE)
    {
    	m_buffer = NULL;
    	m_buffer_size = 0;
       
    	m_buffer = (char *)calloc(1, size);
    	if(m_buffer != NULL)
    		m_buffer_size = size;

    	m_data = m_buffer;
        m_data_size = 0;
    }
    ~IOBuffer()
    {
        if(m_buffer != NULL)
            free(m_buffer);
        m_buffer = m_data = NULL;
        m_buffer_size = m_data_size = 0;
    }

    //开始写, 获取一个大小为size的缓冲用于写, 成功返回可写的缓冲区, 失败返回NULL,表示没有足够内存
    char* write_begin(unsigned int size)
    {
        unsigned int empty  = m_buffer_size-m_data_size; //缓冲区总的剩余空间
        unsigned int offset = m_data-m_buffer;           //缓冲区前端剩余空间
        unsigned int last   = empty-offset;              //缓冲区后端剩余空间
        if(last >= size) //缓冲区后端的空间足够
            return m_data+m_data_size;

        //移动数据到缓冲去开始处
        memcpy(m_buffer, m_data, m_data_size);
        m_data = m_buffer;

        //总的剩余空间足够
        if(empty >= size)  
            return m_data+m_data_size;

        //不够空间
        last = m_buffer_size;
        if(last < BUFFER_SIZE)
            last = BUFFER_SIZE;
        while(last-m_data_size<size && last<MAX_BUFFER_SIZE)  //按原空间2倍扩展
            last <<= 1;
        if(last >= MAX_BUFFER_SIZE)
            return NULL;

        char *temp = (char *)realloc(m_buffer, last);
        if(temp != NULL)
        {  
            m_buffer = temp;
            m_buffer_size = last;
            m_data = m_buffer;
            return m_data+m_data_size;
        }
        return NULL;
    }

    //写结束, 设置本次写入多少数据, 成功返回true, 失败返回false
    bool write_end(unsigned int size)
    {
        if(m_buffer_size-m_data_size < size)  //本次写入的长度比剩余空间还多, 错误
            return false;
        m_data_size += size;
        return true;
    }

	//将写入的数据回滚size个字节.
	//返回值:
	//true: 回滚成功
	//false: 回滚失败, 数据没有变化
	bool write_rollback(unsigned int size)
	{
		if(size > m_data_size)
			return false;
		m_data_size -= size;
		return true;
	}

    //开始读, 获取用于读的缓冲区, size返回可读取缓冲区的大小.如果没有数据可读, 返回NULL.
    char* read_begin(unsigned int *size)
    {
        if(m_data_size>0)
        {
            *size = m_data_size;
            return m_data;
        }
		*size = 0;
        return NULL;
    }

    //读结束, 设置本次读取多少数据, 成功返回true, 失败返回false
    bool read_end(unsigned int size)
    {
        if(size > m_data_size) //读的长度比数据长度还大
            return false;
        m_data+=size;
        m_data_size-=size;
        return true;
    }

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
