package com.ailk.ainb.cache.client.server;

import java.util.TimerTask;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.ailk.ainb.cache.client.netty.CacheClientNettyConnection;

/**
 * 重连线程
 * 
 * @author Administrator
 * 
 */
public class CacheClientReconnectTimerTask extends TimerTask {
	private static final Log log = LogFactory.getLog(CacheClientReconnectTimerTask.class);
	private final CacheClientNettyConnection connection;
	private final CacheClientContext ctx;

	public CacheClientReconnectTimerTask(CacheClientNettyConnection conn, CacheClientContext ctx) {
		this.connection = conn;
		this.ctx = ctx;
	}

	@Override
	public void run() {
		log.info("try reconnect...");
		if (!ctx.close) {
			connection.connect();
		}
	}

}
