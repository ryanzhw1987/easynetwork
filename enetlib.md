**一、 enetlib工具的使用**

> enetlib随EasyNetwork库一起安装。enetlib工具可以帮你快速生成框架类代码。其使用非常简单，安装之后，在命令行输入enetlib可以获取如下帮助信息：

```
Usage: enetlib -[s|m|p] ClassName [-main]
 	-s : Generate a SINGLE thread server class
 	-m : Generate a MULTIPLE thread server class
 	-p : Use ProtocolFamily class, generate protocol_family class if not exist
 	-main : Generate a Main-cpp file for the ClassName 

说明:
1.参数的出现顺序不固定:
  "-s AAA -main"和"-main -s AAA" 都是合法的

2.参数s,m,p最好不要同时/多次使用, 同时/多次使用以最后出现的一个为有效值,其他忽略
  "-m MMM -m MMM -s SSS" 等价于 "-s SSS", 只生成单线程的SSS类
  "-m MMM -p PPP -p DDD" 等价于 "-m MMM  -p DDD"

3.参数s或m与p结合时, 表示在框架类代码中使用协议族
  "-m MMM -p PPP"或者"-p PPP -m MMM" 在框架类代码MMM中使用协议族PPP

4.参数main只对s或m有效
  "-main -m MMM" 或者 "-s SSS -main" 将生成对应的Mian文件

5.所有参数组合
  "-main -s SSS -p PPP"或者"-m MMM -p PPP -main"或者"-p PPP -main -m MMM"等都是合法的
```

<br><br>
<b>二、 生成单线程框架</b>
<pre><code>enetlib –s SingleServer [-p TestProtocolFamily] [-main]<br>
</code></pre>
将在当前目录生成如下文件：<br>
<pre><code>SingleServer.cpp <br>
SingleServer.h <br>
TestProtocolFamily.h  [使用-p参数,SingleServer使用该协议族]<br>
TestProtocolFamily.cp [使用-p参数,SingleServer使用该协议族]<br>
SingleServerMain.cpp  [使用-main参数]<br>
</code></pre>

<br><br>
<b>三、 生成多线程框架</b>
<pre><code>enetlib –m MultipleServer [-main]<br>
</code></pre>
将在当前目录生成如下文件：<br>
<pre><code>MultipleServer.cpp <br>
MultipleServer.h <br>
MultipleServerMain.cpp  [使用-main参数]<br>
</code></pre>

<br><br>
<b>四、 生成协议族类代码<br>
<pre><code>enetlib –p TestProtocolFamily<br>
</code></pre>
将在当前目录生成如下文件：<br>
<pre><code>TestProtocolFamily.cpp <br>
TestProtocolFamily.h<br>
</code></pre></b>

<br><br>
<b>五、 其他</b>
<blockquote>enetlib生成的类代表NetInterface的一个实例。开发者通过创建自己的协议族类(ProtocolFamily)，并在生成的框架类代码中实现相应的接口方法就可以完成自己的应用层业务逻辑。</blockquote>

