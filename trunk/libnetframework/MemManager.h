#ifndef _LIB_MEMMANAGER_H_20120826_LIUYONGJIN
#define _LIB_MEMMANAGER_H_20120826_LIUYONGJIN

#include <stdio.h>
#include <stdlib.h>
#include <new>

///////////////////////////////////
///////     Memory Cache    ///////
///////////////////////////////////
template<class T>
class MemCache
{
public:
	MemCache():m_free_list(NULL){}
	~MemCache();
	T* Alloc();
	bool Free(T *&element);
private:
	void* m_free_list;
};

#define MEM_MAGIC_NUM 0x2B2B2C2C
typedef struct _mem_block
{
	unsigned int magic_num;
	void *prt;
}MemBlock;
#define MIN_SIZE sizeof(void*)

template<class T>
MemCache<T>::~MemCache()
{
	MemBlock *temp,*next; 
	
	for(temp=(MemBlock*)m_free_list; temp != NULL; temp=next)
	{
		next = (MemBlock*)temp->prt;
		free(temp);
	}
}

template<class T>
T* MemCache<T>::Alloc()
{
	T *element = NULL;
	MemBlock *temp = NULL;

	if(m_free_list != NULL)
	{
		temp = (MemBlock*)m_free_list;
		m_free_list = temp->prt;

		element = (T*)&temp->prt;
	}
	else
	{
		unsigned int size = sizeof(T)>MIN_SIZE?sizeof(T):MIN_SIZE;
		size += sizeof(unsigned int);
		temp = (MemBlock*)calloc(1, size);
		if(temp != NULL)
		{
			temp->magic_num = MEM_MAGIC_NUM;
			element = (T*)((char*)temp + sizeof(unsigned int));
		}
	}

	if(element == NULL)
		return NULL;

	element = new(element) T;
	return element;
}

template<class T>
bool MemCache<T>::Free(T *&element)
{
	MemBlock *temp = (MemBlock *)((char*)element - sizeof(unsigned int));
	if(temp->magic_num != MEM_MAGIC_NUM)
		return false;

	element->~T();
	element = NULL;

	temp->prt = m_free_list;
	m_free_list = (void*)temp;
	return true;
}

///////////////////////////////////
///////     Memory Page     ///////
///////////////////////////////////
class MemPage
{
public:
    MemPage(unsigned int slot_size, unsigned int slot_total=32);  //������СΪslot_total��T���ڴ�ҳ
    ~MemPage();
    //��ȡһ��slot, NULL��ʾû�пռ�
    void* get_slot();
    //�ж�һ��slot�Ƿ����ڵ�ǰpage. true����,false������
    bool is_slot(void* slot){return (char*)slot>=m_ptr && (char*)slot<=m_ptr+m_total*m_slot_size;}
    //�Ƿ��п���slot.true��, falseû��
    bool have_slot(){return m_used<m_total;}
private:
    unsigned int m_slot_size;
    unsigned int m_total;
    unsigned int m_used;
    char* m_ptr;
};

///////////////////////////////////
///////     Memory Slab     ///////
///////////////////////////////////
template <class T>
class MemSlab
{
public:
    MemSlab(unsigned int init_capacity=128);
    ~MemSlab();
    T* Alloc();
    bool Free(T* element);
private:
    void *m_free_list;
    MemPage** m_pages;
    unsigned int m_total;
    unsigned int m_current;
    unsigned int m_capacity;    //ÿ��mem page��Ԫ�ظ���, ��2����չ

private:
    void* get_free_slot();
    void* get_page_slot();
    void expend_page();
    void new_page();
};

//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// public member function  //////////////////////////
template <class T>
MemSlab<T>::MemSlab(unsigned int init_capacity/*=128*/)
{
    m_free_list = NULL;

    m_total = 4; //��ʼ��������5��mem page������
    m_current = 0;
    m_capacity = init_capacity;
    m_pages = calloc(m_total, sizeof(MemPage*));
    new_page(); //����һ����ҳ    
}

template <class T>
MemSlab<T>::~MemSlab()
{
    m_free_list = NULL;

    while(m_current >= 0)
        delete m_pages[m_current--];
    free(m_pages);

    m_pages = NULL;
    m_total = 0;    
}

template <class T>
T* MemSlab<T>::Alloc()
{
    T* element = NULL;
    void* slot = get_free_slot();
    if(slot == NULL)
        slot = get_page_slot();

    if(slot != NULL)
        element = new(slot) T;
    return element;
}

template <class T>
bool MemSlab<T>::Free(T* element)
{
    int i;
    for(i=0; i<=m_current; ++i)
        if(m_pages[i]->is_slot((void*)element))
            break;
    if(i>m_current) //�����ڵ�ǰslab
        return false;

    element->~T();
    *(void**)element = m_free_list;
    m_free_list = (void*)element;
    return true;
}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// private member function  //////////////////////////
template <class T>
void MemSlab<T>::new_page()
{
    unsigned int size = sizeof(T);
    if(size < sizeof(void*))
        size = sizeof(void*);   //����Ҫ4���ֽ�

    if(m_current < m_total)
        m_pages[m_current] = new MemPage(size, m_capacity);
}

template <class T>
void MemSlab<T>::expend_page()
{
    if(m_pages[m_current]->have_slot())
        return;

    //���·���һ��page
    ++m_current;
    m_capacity<<=1;

    if(m_current == m_total)
    {
        m_total<<=1;
        MemPage** pages = realloc(m_pages, m_total*sizeof(MemPage*));
        if(pages != NULL)
            m_pages = pages;
        else
            return;
    }

    new_page();
}

template <class T>
void* MemSlab<T>::get_free_slot()
{
    void* slot = m_free_list;
    if(slot != NULL)
        m_free_list = *(void**)m_free_list;
    return slot;
}

template <class T>
void* MemSlab<T>::get_page_slot()
{
    expend_page();
    return m_pages[m_current]->get_slot();
}

#endif //_LIB_MEMMANAGER_H_20120826_LIUYONGJIN

