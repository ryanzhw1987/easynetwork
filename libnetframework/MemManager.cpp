#include "MemManager.h"

MemPage::MemPage(unsigned int slot_size, unsigned int slot_total/*=32*/)
{
    m_slot_size = slot_size;

    m_used = 0;
    m_total = 0;
    m_ptr = NULL;
    if((m_ptr=(char*)calloc(slot_total, m_slot_size)) != NULL)
        m_total = slot_total;
}

MemPage::~MemPage()
{
    if(m_ptr != NULL)
        free(m_ptr);
    m_ptr = NULL;
    m_total = 0;       
}

void* MemPage::get_slot()
{
    char* slot = NULL;
    if(m_used < m_total)
    {
        slot = m_ptr + m_used*m_slot_size;
        ++m_used;
    }

    return (void*)slot;
}
