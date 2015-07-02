<h1>libslog</h1>

> libslog是一个基于linux的高性能开源C/C++多线程安全日志库。_EasyNetwork_ 服务端开源框架使用了libslog作为日志库。

> libslog的最新源码可从这里下载：[libslog.tar](http://easynetwork.googlecode.com/files/libslog.tar)

> 有任何宝贵的意见和建议请联系作者：_xmulyj@gmail.com_

**一、 特性**
> a. 多线程安全

> b. 简单的配置参数

> c. 支持热更新log参数，如从INFO级别更改为DEBUG不需要重启服务器

**二、安装与使用**
> a. 编译
```
make
```

> b. 安装
```
sudo make install
```

> c. 使用

> libslog使用非常简单，在自己的源文件中包含头文件slog.h：
```
#include "slog.h"
```

> 使用libslog时需要先进行初始化：
```
SLOG_INIT(SLOG_CONFIG_PATH);
//SLOG_INIT(NULL);
```
> > 其中SLOG\_CONFIG\_PATH是slog的配置文件路径，如果为NULL的话将使用默认的配置参数，并且log将打印到屏幕。


> libslog有5个log级别：TRACE、DEBUG、INFO、WARN、ERROR。TRACE级别最高，将打印所有级别的log。DEBUG次之，可以打印TRACE除外的其他log。ERROR最低，只打印本级别的log。slog的使用非常简单，与C语言的printf格式一致：
```
#include "slog.h"

int main()
{
  SLOG_INIT(NULL);

  SLOG_TRACE("hello slog. %d", 1);
  SLOG_DEBUG("hello slog. %d", 2);
  SLOG_INFO("hello slog. %d", 3);
  SLOG_WARN("hello slog. %d", 4);
  SLOG_ERROR("hello slog. %d", 5);

  SLOG_UNINIT();

  return;
}
```
> > 其中SLOG\_UNINIT()是对slog的反初始化。


> 最后链接slog库：
```
g++ a.cpp -lslog -o test_slog
```


**三、 性能**
> a. 测试条件
```
每次写入100w条数据,测试10次. 每条数据的格式如下:
slog:  2012-09-24 17:55:52[WARN]ccccccc...  (99个'C'字符)
log4cpp:  2012-09-24 18:03:58 [INFO]: [sub1] ccccccc...  (99个'C'字符)
```

> b. 机器负载
```
slog:  load average: 0.25, 0.32, 0.60
log4cpp:  load average: 0.29, 0.37, 0.51
```

> c. log4cpp版本
```
log4cpp-1.1rc1.tar.gz
```
> d. 测试结果
```
----------------------------
 \  |  slog     |  log4cpp  
---------------------------
 1  | 1.7867    |  10.4785  
 2  | 1.8693    |  11.6938  
 3  | 1.8035    |  9.6103   
 4  | 1.9039    |  9.6179   
 5  | 1.7574    |  10.6606 
 6  | 1.7622    |  9.7011   
 7  | 1.7441    |  9.5807 
 8  | 1.8711    |  11.1646
 9  | 1.7866    |  9.6206
10  | 1.7732    |  9.6028
----|-----------------------
avg |  1.8058   | 10.17309
----------------------------
                   (单位:s)
```
> 性能比log4cpp高5倍:)

**四、 slog的配置参数**
> slog的配置参数非常简单，并且支持热更新，即在不需要重启服务器的情况下修改配置参数，这点对线上跟踪问题非常有用。
```
### log级别
slog_level=DEBUG
#slog_level=INFO
### log文件名
slog_log_name=./log/server.log
### log 文件最大大小(单位M)
slog_log_maxsize=20M
### log文件最多个数
slog_log_maxcount=30
### log缓冲区大小(单位KB,默认512KB)
slog_flush_size=1024
### log缓冲刷新间隔(单位s,默认1s)
slog_flush_interval=2
### log动态更新配置参数的时间间隔(单位s,默认60s)
config_update_interval=30
```

**五、 测试代码(见源码)**
> a. slog\_test.c
```
char buf[100];
memset(buf, 'c', 100);
buf[99]=0;

int i,j;
struct timeval start, end;

printf("start...\n");
gettimeofday(&start, NULL);
for(i=0; i<100; ++i)
{
	for(j=0;j<10000; ++j)
		SLOG_WARN("%s", buf);
}
gettimeofday(&end, NULL);
printf("end...\n");

int us = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
printf("use time:%0.4f(s)\n", us/1000000.0);
```

> b. log4cpp\_test.cpp
```
char buf[100];
memset(buf, 'C', 100);
buf[99] = '\0';

int i, j;
struct timeval start, end;

printf("start...\n");
gettimeofday(&start, NULL);
for(i=0; i<100; ++i)
{
	for(j=0; j<10000; ++j)
		sub1.info(buf);
}
gettimeofday(&end, NULL);
printf("end...\n");

int us = (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);
printf("use time:%0.4f(s)\n", us/1000000.0);
```