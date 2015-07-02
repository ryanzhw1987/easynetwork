<h2>ByteBuffer类</h2>

<br><br>
<b>ByteBuffer</b> 类是对字符数组的一个封装，能够自动管理字符数组的空间分配，使开发者不需要关心内存的分配和释放，能够有效地提高开发效率。另外，在框架中,ByteBuffer在协议数据的序列化和反序列化、数据的接收和发送也起到重要的作用。<br>
<br>
为什么不直接使用标准库的string呢？因为在某些地方我们可能需要直接使用字符数组，ByteBuffer允许这种使用方法。<br>
<br>
<br><br>
1. 成员方法<br>
<pre><code> //注意: 通过ByteBuffer的方法(比如get_append_buffer)获得的buffer指针,最好不要长久保存使用.<br>
 //因为ByteBuffer内部可能重新分配,导致保存的指针失效!!!!<br>
 //比如下面使用是危险的,因为调用append方法可能导致内存重新分配.<br>
 //ByteBuffer byte_buffer;<br>
 //char *temp_buffer = byte_buffer.get_append_buffer(1024);<br>
 //byte_buffer.append("test",4);<br>
 //memcpy(temp_buffer, "this operation is dangerous!!!", 30);<br>
 class ByteBuffer<br>
 {<br>
 public:<br>
 	//初始化一个空间大小为capacity的buffer<br>
 	ByteBuffer(int capacity=INIT_SIZE);<br>
 	~ByteBuffer();<br>
<br>
 	//返回buffer的空间大小<br>
 	int capacity(){return m_capacity;}<br>
<br>
 	//返回buffer中数据的大小<br>
 	int size(){return m_size;}<br>
<br>
 	//获取从偏移offset开始的大小为size的有效数据(size=-1时表示offset后所有的有效数据)<br>
 	char* get_data(int offset=0, int size=-1);<br>
 <br>
 	//从有效数据缓冲区结尾开始获取一个大小为size的buffer(供调用者直接使用)<br>
 	//对获取到的buffer,最多只能写入size个字节的数据;<br>
 	//如果往获取到的buffer中写入数据,而没有调用set_append_size,则写入无效,不改变数据的大小(可能改变buffer的capacity)<br>
 	char* get_append_buffer(int size);<br>
<br>
 	//设置实际添加到结尾的数据长度<br>
 	//必须先调用get_append_buffer<br>
 	void set_append_size(int append_size);<br>
 <br>
 	//从数据结尾开始占用size字节的空间.成功返回该空间的偏移位置,失败返回-1<br>
 	int reserve(int size);<br>
 <br>
 	//将长度为size的buf添加到有效数据缓冲区的末尾.成功返回true,失败返回false<br>
 	bool append(const char *buf, int size);<br>
<br>
 	//添加以'\0\为结尾的字符串string到buffer的末尾(不包括string的'\0')<br>
 	bool append(const char *str);<br>
<br>
 	//在结尾添加count个字符c<br>
 	bool append(const char c, int count=1);<br>
 <br>
 	//容量增加size个字节(默认扩展为原来的2倍),成功返回true,失败返回false.<br>
 	bool expand(int size=-1);<br>
<br>
 	//清空全部有效数据<br>
 	void clear(){m_size = 0;}<br>
<br>
 	//清空末尾大小为size的有效数据(size超过数据长度的话, 等效于clear)<br>
 	void truncate(int size){size&gt;m_size?m_size=0:m_size-=size;}<br>
<br>
 public://以下四个运算符功能与append方法一样(对str操作都不包括'\0')<br>
 	ByteBuffer&amp; operator +=(const char *str);<br>
 	ByteBuffer&amp; operator +=(const char c);<br>
 	ByteBuffer&amp; operator &lt;&lt;(const char *str);<br>
 	ByteBuffer&amp; operator &lt;&lt;(const char c);<br>
 private:<br>
 	char *m_buffer;<br>
 	int m_capacity;<br>
 	int m_size;<br>
 };<br>
</code></pre>

<br><br>
2. 使用例子<br>
<br>
<blockquote>(1) 创建一个ByteBuffer实例<br>
<pre><code>ByteBuffer byte_buffer;<br>
ByteBuffer ptr_byte_bbuffer = new ByteBuffer(1024);<br>
</code></pre></blockquote>

<blockquote>(2) 在有效数据后面获取一个缓冲区用于写<br>
<pre><code>char *buffer = byte_buffer.get_append_buffer(100);<br>
memcpy(buffer, "hello byte_buffer", 16);<br>
byte_buffer.set_append_size(16);//此时byte_buffer的内容为"hello byte_buffer",注意:不包含'\0'字符!!!<br>
</code></pre>
get_append_buffer和set_append_size方法比较特殊,主要用来获取一个可以直接操作的缓冲区用来写数据,避免内存的多次copy而导致效率降低.如果是小量的数据赋值,可以使用append方法</blockquote>

<blockquote>(3) 直接在有效数据末尾添加数据<br>
<pre><code>byte_buffer.append("aaaaa"); //直接添加C风格的字符串.注意:不包含'\0'字符!!!<br>
<br>
char temp[] = "bbbbbbb";<br>
byte_buffer.append(temp, 2); //添加2个字符<br>
<br>
byte_buffer.append('c', 10); //添加10个'c'字符<br>
</code></pre></blockquote>

<blockquote>(4) 预留空间<br>
<pre><code>int offset = byte_buffer.reserve(10); //在数据末尾开始占用10个字符空间(这时数据的size增加了10),这段预留的空间内容是不确定的.<br>
</code></pre>
该方法主要用来先写后面的内容，然后再调回来往预留的空间写入数据，这些数据值可能依赖于后面的内容。</blockquote>

3. 流操作符<br>
<blockquote>为了使用方便，ByteBuffer重载了4个操作符：<br>
<pre><code>ByteBuffer&amp; operator +=(const char *str);<br>
ByteBuffer&amp; operator +=(const char c);<br>
ByteBuffer&amp; operator &lt;&lt;(const char *str);<br>
ByteBuffer&amp; operator &lt;&lt;(const char c);<br>
</code></pre>
使用例子:<br>
<pre><code>byte_buffer.clear();<br>
byte_buffer += "abc"; //byte_buffer内容为"abc"<br>
byte_buffer += 'd';   //byte_buffer内容为"abcd"<br>
byte_buffer &lt;&lt; "efg"; //byte_buffer内容为"abcdefg"<br>
byte_buffer &lt;&lt; 'h';   //byte_buffer内容为"abcdefgh"<br>
</code></pre></blockquote>

<br><br>