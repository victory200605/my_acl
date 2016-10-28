/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.netty;

import org.jboss.netty.buffer.ChannelBuffer;

import com.ailk.ainb.cache.client.CacheClientListener;
import com.ailk.ainb.cache.client.biz.Constants;
import com.ailk.ainb.cache.client.biz.Response;
import com.ailk.ainb.cache.client.biz.ResponseDelete;
import com.ailk.ainb.cache.client.biz.ResponseDeleteTimeOut;
import com.ailk.ainb.cache.client.biz.ResponseFetch;
import com.ailk.ainb.cache.client.biz.ResponseInsert;
import com.ailk.ainb.cache.client.biz.ResponseMsg;
import com.ailk.ainb.cache.client.server.CacheClientContext;
import com.ailk.ainb.cache.client.server.DeleteTimeOutTimerTask;

public class UnMessage implements Runnable {
	private final ChannelBuffer message;
	private final CacheClientContext ctx;
	private final DeleteTimeOutTimerTask task;

	public UnMessage(ChannelBuffer message, CacheClientContext ctx, DeleteTimeOutTimerTask task) {
		this.message = message;
		this.ctx = ctx;
		this.task = task;
	}

	@Override
	public void run() {
		ResponseMsg msg = new ResponseMsg();
		Object obj = msg.decValue(message);
		if (obj != null) {
			for (CacheClientListener listener : ctx.listeners) {
				if (msg.getCmdId() == Constants.RESP_INSERT) {
					// ResponseInsert f=(ResponseInsert)obj;
					// System.out.println("==========i======================"+f.getMsgId()+":"+f.getResult()+":"+":"+(++i));
					listener.insert((ResponseInsert) obj);
				} else if (msg.getCmdId() == Constants.RESP_DELETE) {
					listener.delete((ResponseDelete) obj);
				} else if (msg.getCmdId() == Constants.RESP_FETCH) {
					// ResponseFetch f=(ResponseFetch)obj;
					// if(f.getResult()==0){
					// System.out.println("============j===================="+f.getMsgId()+":"+f.getResult()+":"+":"+f.getValue()+":"+(++j));
					// }
					listener.fetch((ResponseFetch) obj);
				} else if (msg.getCmdId() == Constants.RESP_DELTIMEOUT) {
					ResponseDeleteTimeOut timeOut = (ResponseDeleteTimeOut) obj;
					if (timeOut.getResult() != Constants.CORRECT) {
						task.close();
					}
					listener.deleteTimeOut(timeOut);
				} else {
					listener.errorCommunication((Response) obj);
				}
			}
		}
	}

}
