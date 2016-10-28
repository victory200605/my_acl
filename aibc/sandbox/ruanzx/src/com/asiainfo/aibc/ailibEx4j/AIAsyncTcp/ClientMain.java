package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

import java.io.IOException;

import com.asiainfo.aibc.ailibEx4j.AILib.AIRandomStringGenerater;

public class ClientMain
{
	public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException
	{
		if (args.length < 7)
		{
			System.out
					.println("the command should be contain 7 parameters: serverIP serverPort connNum timeout sendThreadNum countForPerSendThread contentSize");
			return;
		}

		String serverIP = args[0];
		int serverPort = Integer.parseInt(args[1]);
		int connNum = Integer.parseInt(args[2]);
		long timeout = Integer.parseInt(args[3]);
		int sendThreadNum = Integer.parseInt(args[4]);
		int countForPerSendThread = Integer.parseInt(args[5]);
		int contentSize = Integer.parseInt(args[6]);

		final AsyncClient ac = new AsyncClient(serverIP, serverPort, connNum, timeout);
		AIRandomStringGenerater rsg = new AIRandomStringGenerater();

		ac.connectTo();

		for (int i = 0; i < sendThreadNum; i++)
		{
			Thread.sleep(10);
			new Thread(new sendReq(ac, ((new Integer(i)).toString() + "===" + rsg.generateString(contentSize))
					.getBytes(), countForPerSendThread)).start();
		}
	}
}

class sendReq implements Runnable
{
	private byte[] s;
	private AsyncClient ac;
	private int countForPerSendThread;

	public sendReq(AsyncClient ac, byte[] s, int countForPerSendThread)
	{
		this.s = s;
		this.ac = ac;

		if(countForPerSendThread <= 0)
		{
			this.countForPerSendThread = 99999;
		}
		else
		{
			this.countForPerSendThread = countForPerSendThread;
		}
	}

	public void run()
	{
		AsyncMessage aMsg = null;
		try
		{
			for (int i = 0; i < countForPerSendThread; i++)
			{
				 ac.sendRequestAndGetResponse(s,aMsg);
				//Thread.sleep(1000);
			}
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		}

		// if (aMsg1 != null)
		// {
		// System.out.println("msgID=" + aMsg1.getAhead().getCiMsgID() +
		// "\tresult=" + aMsg1.getAhead().getCiResult()
		// + "\tlength=" + aMsg1.getAhead().getCiLength()+"content=" + new
		// String(aMsg1.getContent()));
		//
		// if (aMsg1.getAhead().getCiLength() > 0)
		// {
		// System.out.println("content=" + new String(aMsg1.getContent()));
		// }
		// }
		// else
		// {
		// System.out.println(Thread.currentThread().getName() + " is null,can't
		// find");
		// }
	}
}
