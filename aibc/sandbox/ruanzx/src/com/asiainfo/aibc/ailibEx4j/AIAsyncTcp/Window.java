package com.asiainfo.aibc.ailibEx4j.AIAsyncTcp;

import java.util.Hashtable;

public class Window
{
	private static int siNextMsgID;
	private int msgID;
	private int liNextMsgID;
	private MatchNode matchNode;

	//private Hashtable<Integer, MatchNode> htMsgID = new Hashtable<Integer, MatchNode>();
	private Hashtable htMsgID = new Hashtable();

	public Window()
	{
		siNextMsgID = 0;
		msgID = 0;
		liNextMsgID = 0;
		matchNode = new MatchNode();
	}

	private int getNextMsgID()
	{
		msgID = siNextMsgID;
		siNextMsgID = siNextMsgID + 1;
		return msgID;
	}

	public synchronized MatchNode createMatchNode()
	{
		liNextMsgID = getNextMsgID();

		matchNode = new MatchNode();
		matchNode.setMsgID(liNextMsgID);

		htMsgID.put(new Integer(liNextMsgID), matchNode);

		return matchNode;
	}

	public synchronized MatchNode findMatchNode(int msgID)
	{
		matchNode = new MatchNode();
		matchNode = (MatchNode) htMsgID.get(new Integer(msgID));

		if (matchNode != null)
		{
			return matchNode;
		}
		else
		{
			return null;
		}
	}

	public synchronized MatchNode removeMatchNode(int msgID)
	{
		return (MatchNode) htMsgID.remove(new Integer(msgID));
	}
	
	public void WakeupPendMsg(int msgID, int result, byte[] content)
	{
		MatchNode matchNode = findMatchNode(msgID);

		if (matchNode != null)
		{
			synchronized (matchNode)
			{
				matchNode.setResult(result);

				if (content != null)
				{
					matchNode.setLength(content.length);
					matchNode.setContent(content);
				}
				else
				{
					matchNode.setLength(0);
					matchNode.setContent(null);
				}

				matchNode.notifyAll();
			}
		}
	}

}
