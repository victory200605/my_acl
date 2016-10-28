package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.LinkedList;

public class ConnectionPool
{
	private LinkedList llValidConnList = new LinkedList();
	private LinkedList llPendingConnList = new LinkedList();

	private String serverHost;
	private int serverPort;
	private int maxConnCount;
	private Window ww;
	private long timeout;
	private boolean isShutdown;
	private InetSocketAddress isa;

	private Selector selector = null;

	public ConnectionPool(AsyncClient asyncClient) throws IOException
	{
		this.serverHost = asyncClient.getServerHost();
		this.serverPort = asyncClient.getServerPort();
		this.maxConnCount = asyncClient.getConnNum();
		this.ww = asyncClient.getWw();
		this.timeout = asyncClient.getTimeout();
		this.isShutdown = asyncClient.getShutdown();

		isa = new InetSocketAddress(serverHost, serverPort);
		selector = Selector.open();
	}

	public void makeConnection() throws IOException
	{
		for (int i = 0; i < maxConnCount; i++)
		{
			createSocketChannel();
		}
		
		//目前仍未能实现，可以指定接收线程数，多个线程同时监控一个selector会出错
		new Thread(new RecvThread(this)).start();
	}

	public void createSocketChannel() throws IOException
	{
		SocketChannel socketChannel = SocketChannel.open();
		socketChannel.configureBlocking(false);
		socketChannel.connect(getIsa());

		socketChannel.register(selector, SelectionKey.OP_CONNECT);
		putConnToPendingList(socketChannel);
	}

	public void dispatch() throws InterruptedException
	{
		synchronized (llPendingConnList)
		{
			for (int i = 0; i < llPendingConnList.size(); i++)
			{
				if (!((SocketChannel) llPendingConnList.get(i)).isConnectionPending())
				{
					putConnToValidList(((SocketChannel) llPendingConnList.get(i)));
					llPendingConnList.remove(i);
					i--;
				}
			}
		}
		synchronized (llValidConnList)
		{
			if (llValidConnList.size() <= 0)
			{
				llValidConnList.wait(timeout);
			}
		}
	}

	public SocketChannel getValidConnection() throws InterruptedException
	{
		synchronized (llValidConnList)
		{
			if (llValidConnList.size() > 0)
			{
				// 无法达到效果！
				if (((SocketChannel) llValidConnList.getFirst()).socket().isConnected()
						&& !((SocketChannel) llValidConnList.getFirst()).socket().isClosed())
				{
					return (SocketChannel) llValidConnList.removeFirst();
				}
				else
				{
					llValidConnList.removeFirst();
					return null;
				}
			}
			else
			{
				return null;
			}
		}
	}

	public void putConnToValidList(SocketChannel socketChannel)
	{
		synchronized (llValidConnList)
		{
			llValidConnList.addLast(socketChannel);
			llValidConnList.notifyAll();
		}
	}

	public void putConnToPendingList(SocketChannel socketChannel)
	{
		synchronized (llPendingConnList)
		{
			llPendingConnList.addLast(socketChannel);
			llPendingConnList.notifyAll();
		}
	}

	public void removeFromValidList(SocketChannel socketChannel)
	{
		synchronized (llValidConnList)
		{
			llValidConnList.remove(socketChannel);
		}
	}

	public void removeFromPendingList(SocketChannel socketChannel)
	{
		synchronized (llPendingConnList)
		{
			llPendingConnList.remove(socketChannel);
		}
	}

	public void closeAllConnection() throws IOException
	{
		synchronized (llValidConnList)
		{
			while (llValidConnList.size() > 0)
			{
				SocketChannel socketChannel = (SocketChannel) llValidConnList.removeFirst();
				socketChannel.close();
			}
		}
	}

	// ///////////////////////////////////////////////////
	public Window getWw()
	{
		return ww;
	}

	public long getTimeout()
	{
		return timeout;
	}

	public boolean getShutdown()
	{
		return isShutdown;
	}

	public InetSocketAddress getIsa()
	{
		return isa;
	}

	public int getMaxConnCount()
	{
		return maxConnCount;
	}

	public LinkedList getValidConnList()
	{
		return llValidConnList;
	}

	public Selector getSelector()
	{
		return selector;
	}

}
