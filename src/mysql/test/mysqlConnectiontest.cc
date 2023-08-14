#include "mysqllog.h"
#include <iostream>
using namespace std;
#include "Connection.h"
#include "CommonConnectionPool.h"
#include <stdio.h>

int main()
{
#if 0
	Connection conn;
	conn.connect("127.0.0.1", 3306, "root", "1234", "chat");

	clock_t begin = clock();
	ConnectionPool* cp = ConnectionPool::getConnectionPool();

	//多线程
	thread t1([]() {
		ConnectionPool *cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			/*Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "1234", "chat");
			conn.update(sql);*/
		}
		});
	thread t2([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			/*Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "1234", "chat");
			conn.update(sql);*/
		}
		});
	thread t3([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			/*Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "1234", "chat");
			conn.update(sql);*/
		}
		});
	thread t4([]() {
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		for (int i = 0; i < 2500; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			/*Connection conn;
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			conn.connect("127.0.0.1", 3306, "root", "1234", "chat");
			conn.update(sql);*/
		}
		});

	t1.join();
	t2.join();
	t3.join();
	t4.join();

#endif

#if 1
	//单线程
	clock_t begin = clock();
	for (int i = 0; i < 10000; ++i) {

		Connection conn; //不使用连接池连接
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		conn.connect("127.0.0.1", 3306, "root", "1234", "chat");
		conn.update(sql);
		
		/*
		ConnectionPool* cp = ConnectionPool::getConnectionPool();
		shared_ptr<Connection> sp = cp->getConnection();
		char sql[1024] = { 0 };
		sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			"zhang san", 20, "male");
		sp->update(sql);
		*/
	}
#endif

	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;

	return 0;
}
