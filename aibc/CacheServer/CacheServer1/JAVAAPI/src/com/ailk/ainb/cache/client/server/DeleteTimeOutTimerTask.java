/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.server;

import java.io.Closeable;
import java.util.TimerTask;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jboss.netty.buffer.ChannelBuffer;

import com.ailk.ainb.cache.client.impl.ChannelBufferValue;
import com.ailk.ainb.cache.client.netty.CacheClientNettyConnection;

/**
 * 删除超时线程
 *
 * @author Administrator
 *
 */
public class DeleteTimeOutTimerTask extends TimerTask implements Closeable {
	private static final Log log = LogFactory
	.getLog(DeleteTimeOutTimerTask.class);
	private final CacheClientNettyConnection conn;
	private final CacheClientContext ctx;
	private boolean closed = false;

	public DeleteTimeOutTimerTask(CacheClientNettyConnection conn, CacheClientContext ctx) {
		this.conn = conn;
		this.ctx = ctx;
	}

	@Override
	public void run() {
		closed = false;
		int count = 0;
		while (count < ctx.deleteTimeOutSize && conn.isConnected() && !closed) {
			int msgId = ctx.msgIdGen.next();
			ChannelBuffer buff = ChannelBufferValue.getDeleteOutBuffer(msgId, ctx.deleteTimeOutType);
			conn.write(buff);
			if(log.isDebugEnabled()){
				log.debug("write deleteTimeOut:"+buff);
			}

			count++;
		}
	}

	@Override
	public void close() {
		closed = true;
	}
}
