package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.Set;

public class RecvThread implements Runnable
{
	private Selector selector = null;
	private Window ww;
	private long timeout;
	private ConnectionPool connPool = null;
	private boolean isShutdown;

	public RecvThread(ConnectionPool connPool) throws IOException
	{
		this.selector = connPool.getSelector();
		connPool.getIsa();
		this.timeout = connPool.getTimeout();
		this.ww = connPool.getWw();
		connPool.getMaxConnCount();
		this.connPool = connPool;
		this.isShutdown = connPool.getShutdown();
	}

	public void run()
	{
		int num = 0;
		while (!isShutdown)
		{
			try
			{
				num = selector.select(timeout);
			}
			catch (IOException e1)
			{
				e1.printStackTrace();
			}
			if (num > 0)
			{
				Set readyKeys = selector.selectedKeys();
				Iterator it = readyKeys.iterator();

				while (it.hasNext())
				{
					SelectionKey key = null;
					try
					{

						key = (SelectionKey) it.next();
						it.remove();

						if (key.isReadable())
						{
							recvMessage(key);

						}
						else if (key.isConnectable())
						{
							SocketChannel socketChannel = (SocketChannel) key.channel();

							boolean success = false;
							success = socketChannel.finishConnect();

							socketChannel.register(selector, SelectionKey.OP_READ);

							if (success)
							{
								System.out.println("connect server:" + socketChannel.socket() + " successfully");

								connPool.putConnToValidList(socketChannel);
								connPool.removeFromPendingList(socketChannel);

								synchronized (connPool.getValidConnList())
								{
									connPool.getValidConnList().notifyAll();
								}
							}
							else
							{
								key.cancel();
							}
						}
					}
					catch (IOException e)
					{
						try
						{
							if (key != null)
							{
								SocketChannel socketCh = (SocketChannel) key.channel();

								connPool.removeFromValidList(socketCh);
								connPool.removeFromPendingList(socketCh);

								key.cancel();
								key.channel().close();
								
								//目前对断开的连接，1s后重新建立连接
								Thread t = new Thread()
								{
									public void run()
									{
										try
										{
											Thread.sleep(1000);
										}
										catch (InterruptedException e)
										{
											e.printStackTrace();
										}
										try
										{
											connPool.createSocketChannel();
										}
										catch (IOException e)
										{
											e.printStackTrace();
										}
									}
								};
								t.start();
							}
						}
						catch (Exception ex)
						{
							ex.printStackTrace();
						}
					}
					catch (ClassNotFoundException e)
					{
						e.printStackTrace();
					}
					catch (InterruptedException e)
					{
						e.printStackTrace();
					}
				}
			}
		}
	}

	private synchronized void recvMessage(SelectionKey selKey) throws IOException, ClassNotFoundException,
			InterruptedException
	{
		AsyncHead head = new AsyncHead();

		SocketChannel socketChannel = (SocketChannel) selKey.channel();

		ByteBuffer lbbReadBuffer = ByteBuffer.allocate(AsyncHead.HEADER_SIZE);
		lbbReadBuffer.clear();

		int readNum;
		synchronized (socketChannel)
		{
			readNum = socketChannel.read(lbbReadBuffer);

			if (readNum > 0)
			{
				lbbReadBuffer.order(ByteOrder.BIG_ENDIAN);
				lbbReadBuffer.flip();

				head.parseHeader(lbbReadBuffer);

				if (head.getCiLength() <= 0)
				{
					ww.WakeupPendMsg(head.getCiMsgID(), head.getCiResult(), null);
				}
				else
				{
					ByteBuffer lbbReadBufContent = ByteBuffer.allocate(head.getCiLength());
					lbbReadBufContent.clear();

					socketChannel.read(lbbReadBufContent);
					while (lbbReadBufContent.hasRemaining())
					{
						socketChannel.read(lbbReadBufContent);
					}

					lbbReadBufContent.order(ByteOrder.BIG_ENDIAN);
					lbbReadBufContent.flip();

					byte[] content = new byte[head.getCiLength()];

					lbbReadBufContent.get(content, 0, content.length);

					ww.WakeupPendMsg(head.getCiMsgID(), head.getCiResult(), content);

					lbbReadBufContent.clear();
				}

				lbbReadBuffer.clear();
			}
		}
	}

}
