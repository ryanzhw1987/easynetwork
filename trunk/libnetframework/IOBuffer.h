#ifndef _LIB_IOBUFFER_H_20120816_LIUYONGJIN
#define _LIB_IOBUFFER_H_20120816_LIUYONGJIN


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024  //��������ʼ����СΪ1k
#define MAX_BUFFER_SIZE 10*1024*1024   //���������10M

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////                                                             ///////
///////                 IOBuffer���ڶ�д�Ļ�����                    ///////
///////       ���ռ䲻��ʱ,�ᰴ2����С��չ,ֱ�����㹻��Ŀռ�       ///////
///////       ���ߴﵽ���ֵ.                                       ///////
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

    //��ʼд, ��ȡһ����СΪsize�Ļ�������д, �ɹ����ؿ�д�Ļ�����, ʧ�ܷ���NULL,��ʾû���㹻�ڴ�
    char* write_begin(unsigned int size)
    {
        unsigned int empty  = m_buffer_size-m_data_size; //�������ܵ�ʣ��ռ�
        unsigned int offset = m_data-m_buffer;           //������ǰ��ʣ��ռ�
        unsigned int last   = empty-offset;              //���������ʣ��ռ�
        if(last >= size) //��������˵Ŀռ��㹻
            return m_data+m_data_size;

        //�ƶ����ݵ�����ȥ��ʼ��
        memcpy(m_buffer, m_data, m_data_size);
        m_data = m_buffer;

        //�ܵ�ʣ��ռ��㹻
        if(empty >= size)  
            return m_data+m_data_size;

        //�����ռ�
        last = m_buffer_size;
        if(last < BUFFER_SIZE)
            last = BUFFER_SIZE;
        while(last-m_data_size<size && last<MAX_BUFFER_SIZE)  //��ԭ�ռ�2����չ
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

    //д����, ���ñ���д���������, �ɹ�����true, ʧ�ܷ���false
    bool write_end(unsigned int size)
    {
        if(m_buffer_size-m_data_size < size)  //����д��ĳ��ȱ�ʣ��ռ仹��, ����
            return false;
        m_data_size += size;
        return true;
    }

	//��д������ݻع�size���ֽ�.
	//����ֵ:
	//true: �ع��ɹ�
	//false: �ع�ʧ��, ����û�б仯
	bool write_rollback(unsigned int size)
	{
		if(size > m_data_size)
			return false;
		m_data_size -= size;
		return true;
	}

    //��ʼ��, ��ȡ���ڶ��Ļ�����, size���ؿɶ�ȡ�������Ĵ�С.���û�����ݿɶ�, ����NULL.
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

    //������, ���ñ��ζ�ȡ��������, �ɹ�����true, ʧ�ܷ���false
    bool read_end(unsigned int size)
    {
        if(size > m_data_size) //���ĳ��ȱ����ݳ��Ȼ���
            return false;
        m_data+=size;
        m_data_size-=size;
        return true;
    }

private:
    char *m_buffer; //������
    unsigned int m_buffer_size;  //��������С

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
