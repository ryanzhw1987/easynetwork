<h1>EasyNetwork框架详细介绍</h1>
EasyNetwork对网络底层、链接管理、协议编/解码、数据的接/发送等进行了封装，并且提供了一些其他工具类，由这些类构成了框架的基础模块。

而NetInterface则将这些模块进行了有机结合，为应用层提供了统一的接口。通过实现NetInterface类相应的接口，开发者可以快速搭建自己的服务器框架，并把精力集中于业务逻辑的开发。

另外ConnectThread和ConnectThreadPool则是对多线程的服务器框架的封装，每个ConnectThread是NetInterface的一个实例。同样只要实现ConnectThread的接口方法，开发者就能搭建好一个多线程的服务器框架，而不必关心链接的分配、线程的创建/管理等细节。

本节将具体地介绍一些基础模块和框架结构，以及一些类图和框架流程等内容。

<br><br>

<b>一、 主要模块介绍</b>
<ul><li>基础模块<br>
<table><thead><th> <a href='ByteBuffer.md'>ByteBuffer</a>     </th><th> <a href='ConnectAccepter.md'>ConnectAccepter</a>        </th></thead><tbody>
<tr><td> <a href='ProtocolFamily.md'>ProtocolFamily</a> </td><td> <a href='DefaultProtocolFamily.md'>DefaultProtocolFamily</a>  </td></tr>
<tr><td> <a href='EventHandler.md'>EventHandler</a>   </td><td> <a href='IODemuxer.md'>IODemuxer</a>                    </td></tr>
<tr><td> <a href='IODemuxerEpoll.md'>IODemuxerEpoll</a> </td><td> <a href='ListenHandler.md'>ListenHandler</a>            </td></tr>
<tr><td> <a href='MemManager.md'>MemManager</a>     </td><td> <a href='Queue.md'>Queue</a>                            </td></tr>
<tr><td> <a href='Socket.md'>Socket</a>             </td><td> <a href='Thread.md'>Thread</a>                          </td></tr>
<tr><td> <a href='ThreadPool.md'>ThreadPool</a>     </td><td> <a href='TransProtocol.md'>TransProtocol</a>            </td></tr>
<tr><td> <a href='WorkThread.md'>WorkThread</a>     </td><td>                                                         </td></tr></li></ul></tbody></table>

<br>
<ul><li>框架结构<br>
<table><thead><th> <a href='NetInterface.md'>NetInterface</a>        </th><th> <a href='SocketManager.md'>SocketManager</a> </th></thead><tbody>
<tr><td> <a href='PipeThread.md'>PipeThread</a>            </td><td> <a href='ConnectThread.md'>ConnectThread</a> </td></tr>
<tr><td> <a href='ConnectThreadPool.md'>ConnectThreadPool</a>   </td><td>                                              </td></tr></li></ul></tbody></table>


<br><br>
<b>二、 EasyNetwork的主要组件</b>

<img src='http://easynetwork.googlecode.com/files/Component.jpg' />

<br><br>
<b>三、 类图</b>

<img src='http://easynetwork.googlecode.com/files/Class.jpg' />

<br><br>
<b>四、 单线程开发范式流程图</b>

<img src='http://easynetwork.googlecode.com/files/procedure.jpg' />

<br><br>
<b>五、 多线程框架流程图</b>

<img src='http://easynetwork.googlecode.com/files/MTframe.jpg' />

<br><br><br>
有任何的问题或者意见和建议请联系作者：<i>xmulyj@gmail.com</i>

<br><br><br>