#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include "log4cpp/Category.hh"
#include "log4cpp/PropertyConfigurator.hh"
int main(int argc, char* argv[]) 
{
    try
    {
        log4cpp::PropertyConfigurator::configure("./log4cpp.config");
    }
    catch (log4cpp::ConfigureFailure& f)
    {
        std::cout << "Configure Problem " << f.what() << std::endl;
        return -1;
    }

    //2    实例化category对象
    //    这些对象即使配置文件没有定义也可以使用，不过其属性继承其父category
    //    通常使用引用可能不太方便，可以使用指针，以后做指针使用
    log4cpp::Category& root = log4cpp::Category::getRoot();
    log4cpp::Category& sub1 = log4cpp::Category::getInstance(std::string("sub1"));


	char buf[100];
	memset(buf, 'C', 100);
	buf[99] = '\0';
	//    sub1 has appender A1 and rootappender. since the additivity property is set true by default
    int i, j;
    struct timeval start, end;

	printf("start...\n");
    gettimeofday(&start, NULL);
    for(i=0; i<100; ++i)
            for(j=0; j<10000; ++j)
                    sub1.info(buf);
    gettimeofday(&end, NULL);
    printf("end...\n");

    int us = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
    printf("use time:%0.4f(s)\n", us/1000000.0);
	
    return 0;
}
