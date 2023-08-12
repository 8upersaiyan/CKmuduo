#include "CommonConnectionPool.h"
#include "mysqllog.h"
#include <iostream>

// 线程安全的懒汉单例函数接口 
ConnectionPool* ConnectionPool::getConnectionPool()
{
	static ConnectionPool pool; // lock和unlock 静态的初始化第一次运行才会初始化 线程安全
	return &pool;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
	FILE* pf = fopen("mysql.ini", "r");
	if (pf == nullptr)
	{
		LOG("mysql.ini file is not exist!"); //日志信息
		return false;
	}

	while (!feof(pf)) //feof 用于查询文件指针是否以达到末尾
	{
		char line[1024] = { 0 };
		fgets(line, 1024, pf);
		string str = line;

		//找等号
		int idx = str.find('=', 0);
		if (idx == -1) {	//无效配置项
			continue;
		}

		//找回车
		//password=1234\n
		int endidx = str.find('\n', idx);
		//解析文本内容
		string key = str.substr(0, idx);
		string value = str.substr(idx + 1, endidx - idx - 1);

		if (key == "ip")
		{
			_ip = value;
		}
		else if (key == "port")
		{
			_port = atoi(value.c_str());
		}
		else if (key == "username")
		{
			_username = value;
		}
		else if (key == "password")
		{
			_password = value;
		}
		else if (key == "dbname")
		{
			_dbname = value;
		}
		else if (key == "initSize")
		{
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize")
		{
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut")
		{
			_connectionTimeout = atoi(value.c_str());
		}
	}
	return true;
}

//连接池的构造函数
ConnectionPool::ConnectionPool() {

	// 加载配置项了
	if (!loadConfigFile())
	{
		return;
	}
	// 创建初始数量的连接
	for (int i = 0; i < _initSize; ++i)
	{
		Connection* p = new Connection();
		p->connect(_ip, _port, _username, _password, _dbname);
		p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
		_connectionQue.push(p);
		_connectionCnt++;
	}

	// 绑定器
	// 启动一个新的线程，作为连接的生产者 linux thread => pthread_create 
	thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
	produce.detach();//分离线程

	// 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
	thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
	scanner.detach();//分离线程
}

//运行在独立的线程中，专门负责生产新连接     生产者线程
void ConnectionPool::produceConnectionTask() {
	for (;;) {
		unique_lock<mutex> lock(_queueMutex);
		while (!_connectionQue.empty()) {
			cv.wait(lock); //队列不为空， 此处生产线程进入等待状态
		}

		// 连接数量没有到达上限，继续创建新的连接
		if (_connectionCnt < _maxSize) {
			Connection* p = new Connection();
			p->connect(_ip, _port, _username, _password, _dbname);
			p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
			_connectionQue.push(p);
			_connectionCnt++;  //连接数量++
		}
		// 通知消费者线程， 可以消费连接了
		cv.notify_all();
	}
}

//给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<Connection> ConnectionPool::getConnection() {

	unique_lock<mutex> lock(_queueMutex);
	//队列为空
	while (_connectionQue.empty()) {
		//超时
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (_connectionQue.empty()) //超时醒来发现还是空的 
			{
				LOG("获取空间连接超时了。。。获取连接失败！");
				return nullptr;
			}
		}
	}
	/*
	* share_ptr智能指针析构时，会把connection资源直接delete掉，相当于调用connection的析构函数，connection就被close掉了
	* 这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue中
	*/
	//队列不为空 ----消费连接
	shared_ptr<Connection> sp(_connectionQue.front(), 
		[&](Connection *pcon) 
		{
			// 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
			unique_lock<mutex> lock(_queueMutex);
			pcon->refreshAliveTime(); // 刷新一下开始空闲的起始时间
			_connectionQue.push(pcon);
	    });

	_connectionQue.pop(); //消费掉头部

	if (_connectionQue.empty()) //消费掉头部后 队列变为空 
	{
		//谁消费了队列中最后一个connection，谁负责通知一下生产连接
		cv.notify_all(); 
	}
	return sp;
}

//扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
void ConnectionPool::scannerConnectionTask(){
	for (;;) {
		//通过 sleep模拟定时效果
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		//扫描整个队列，释放多余的连接
		unique_lock<mutex> lock(_queueMutex);
		while (_connectionCnt > _initSize) {
			Connection* p = _connectionQue.front(); //指向对头
			if (p->getAliveeTime() >= (_maxIdleTime * 100))
			{
				_connectionQue.pop();
				_connectionCnt--;
				delete p;
			}
			else
			{
				break; //队头的连接没有超过 _maxIdleTim 其他连接肯定没有
			}

		} 
	}
}