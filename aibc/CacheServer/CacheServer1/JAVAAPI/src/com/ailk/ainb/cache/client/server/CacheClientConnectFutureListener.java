/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.server;

import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.ChannelFutureListener;

import com.ailk.ainb.cache.client.netty.CacheClientNettyConnection;
/**
 * 监听是否连接成功
 * 失败则重连
 * @author Administrator
 *
 */
public class CacheClientConnectFutureListener implements ChannelFutureListener {
	private final CacheClientNettyConnection connection;

	public CacheClientConnectFutureListener(CacheClientNettyConnection conn) {
		this.connection = conn;
	}

	@Override
	public void operationComplete(ChannelFuture future) throws Exception {
		if (!future.isSuccess()) {
			 //connection.onConnectFailure(future.getCause());
		 }

	}

}
