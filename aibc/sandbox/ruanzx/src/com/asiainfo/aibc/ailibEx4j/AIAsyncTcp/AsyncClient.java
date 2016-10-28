package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SocketChannel;

public class AsyncClient
{
	private String serverHost;
	private int serverPort;
	private int connNum;
	private long timeout;
	private boolean isShutdown;

	private Window ww = null;
	private ConnectionPool connPool = null;

	public static final int ASYNC_RECV_BUFF_SIZE = (128 * 1024);
	public static final int ASYNC_MAX_CONN_NUM = 10;
	public static final int ASYNC_SEND_FAIL = -100;
	public static final int ASYNC_RECV_FAIL = -200;

	public AsyncClient(String aServerHost, int aiServerPort, int aiConnNum, long aiTimeout) throws IOException
	{
		isShutdown = false;
		serverPort = aiServerPort;

		if (aServerHost != null)
		{
			serverHost = aServerHost;
		}
		else
		{
			aServerHost = "127.0.0.1";
		}

		if (aiConnNum > ASYNC_MAX_CONN_NUM)
		{
			this.connNum = ASYNC_MAX_CONN_NUM;
		}
		else if (aiConnNum <= 0)
		{
			this.connNum = 1;
		}
		else
		{
			this.connNum = aiConnNum;
		}

		if (aiTimeout > 10000)
		{
			this.timeout = aiTimeout;
		}
		else
		{
			this.timeout = 10000;
		}

		ww = new Window();
	}

	public void connectTo() throws IOException, InterruptedException, ClassNotFoundException
	{
		connPool = new ConnectionPool(this);
		connPool.makeConnection();
	}

	public int sendRequestAndGetResponse(byte[] absContent, AsyncMessage aMsg) throws InterruptedException
	{
		SocketChannel socketChannel = null;
		ByteBuffer lbbSendBuffer = ByteBuffer.allocate(ASYNC_RECV_BUFF_SIZE);
		lbbSendBuffer.order(ByteOrder.BIG_ENDIAN);
		AsyncHead ahead = null;

		if (absContent == null || absContent.length > ASYNC_RECV_BUFF_SIZE)
		{
			return -1;
		}
		MatchNode matchNode = ww.createMatchNode();
		while (!isShutdown)
		{
			synchronized (matchNode)
			{
				socketChannel = connPool.getValidConnection();

				if (socketChannel != null)
				{
					ahead = new AsyncHead(matchNode.getMsgID(), 0, absContent.length);

					ByteBuffer lbbBufHead = ByteBuffer.allocate(AsyncHead.HEADER_SIZE);
					lbbBufHead = ahead.buildHeader(ahead.getCiMsgID(), ahead.getCiResult(), ahead.getCiLength());
					lbbBufHead.flip();

					lbbSendBuffer.put(lbbBufHead);
					lbbSendBuffer.put(absContent);

					lbbSendBuffer.flip();

					try
					{
						while (lbbSendBuffer.hasRemaining())
						{
//							System.out.println("sending.................................");
							socketChannel.write(lbbSendBuffer);
						}
					}
					catch (IOException e)
					{
						return ASYNC_SEND_FAIL;
//						continue;
					}

					lbbSendBuffer.clear();

					// wait
//					System.out.println("waiting on... " + socketChannel.socket());
					matchNode.wait(timeout);

					// put back connection
					connPool.putConnToPendingList(socketChannel);

					if (matchNode.getLength() != 0 || matchNode.getResult() != 0)
					{
						ahead = new AsyncHead(matchNode.getMsgID(), matchNode.getResult(), matchNode.getLength());
//						aMsg = new AsyncMessage(ahead, matchNode.getContent());
						aMsg.setAsyncMsg(new AsyncMessage(ahead, matchNode.getContent()));
						
						
					}
					else
					{
						return ASYNC_RECV_FAIL;
					}

					// remove MatchNode from Hashtable
					if (ww.removeMatchNode(matchNode.getMsgID()) == null)
					{
						return -1;
					}

//					if (matchNode.getLength() != 0)
//					{
//						if (new String(absContent).equals(new String(aMsg.getContent())))
//						{
//							System.out.println("\n------send content = response content-------"
//									+ aMsg.getAhead().getCiMsgID() + "\n");
//						}
//						else
//						{
//							System.out.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
//						}
//					}
//					System.out.println("XXXXXXXXXXXXXXXXXXXXXXXXXXX");
					return 0;
				}
				else
				{
//					System.out.println("\n\ndispatch...............");
					connPool.dispatch();
				}
			}
		}
		return 0;
	}

	public void shutdown() throws IOException
	{
		isShutdown = true;
		connPool.closeAllConnection();
	}

	public String getServerHost()
	{
		return serverHost;
	}

	public int getServerPort()
	{
		return serverPort;
	}

	public int getConnNum()
	{
		return connNum;
	}

	public long getTimeout()
	{
		return timeout;
	}

	public boolean getShutdown()
	{
		return isShutdown;
	}

	public Window getWw()
	{
		return ww;
	}

}
