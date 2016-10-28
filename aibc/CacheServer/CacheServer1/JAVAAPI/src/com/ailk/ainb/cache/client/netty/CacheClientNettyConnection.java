/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.netty;

import java.net.SocketAddress;
import java.util.Timer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffers;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;
import org.jboss.netty.util.HashedWheelTimer;

import com.ailk.ainb.cache.client.biz.ResponseMsg;
import com.ailk.ainb.cache.client.server.CacheClientContext;
import com.ailk.ainb.cache.client.server.CacheClientReconnectTimerTask;
import com.ailk.ainb.cache.client.server.DeleteTimeOutTimerTask;
import com.ailk.ainb.cache.client.server.ServerSocketAddress;

public class CacheClientNettyConnection {
	private static final Log log = LogFactory
			.getLog(CacheClientNettyConnection.class);
	private final CacheClientContext ctx;
	private final ServerSocketAddress address;
	private final Timer reconnectTimer = new Timer("ReconnectTimer");
	private final Timer deleteTimeoutTimer = new Timer("DeleteTimeoutTimer");
	private final ExecutorService unpackThreadPool = Executors
			.newFixedThreadPool(Runtime.getRuntime().availableProcessors() + 1,
					new UnpackThreadFactory());

	private final ClientBootstrap clientBootstrap;
	private Channel channel;
	private final int HEADER_LEN = 12;
	private ChannelBuffer addBuffer = ChannelBuffers.dynamicBuffer();
	private final HashedWheelTimer wheelTimer = new HashedWheelTimer();
	private final DeleteTimeOutTimerTask deleteTimeOutTimerTask;

	public CacheClientNettyConnection(CacheClientContext ctx,
			ServerSocketAddress address) {
		this.ctx = ctx;
		this.address = address;

		ChannelFactory factory = new NioClientSocketChannelFactory();
		clientBootstrap = new ClientBootstrap(factory);
		clientBootstrap
				.setPipelineFactory(new PipelineFactory(wheelTimer, ctx,this));

		clientBootstrap.setOption("reuseAddress", true);
		clientBootstrap.setOption("tcpNoDelay", true);
		clientBootstrap.setOption("keepAlive", true);
		deleteTimeOutTimerTask = new DeleteTimeOutTimerTask(this, ctx);
		deleteTimeoutTimer.schedule(deleteTimeOutTimerTask,
				ctx.deleteDataTimeout * 1000, ctx.deleteDataTimeout * 1000);
	}

	public SocketAddress getRemoteAddress() {
		return this.address.getSocketAddress();
	}

	/**
	 * 客户端连接到cacheServer
	 *
	 */
	public void connect() {
		ChannelFuture future = clientBootstrap.connect(this.address
				.getSocketAddress());

//		CacheClientConnectFutureListener listener = new CacheClientConnectFutureListener(this);
//
//		future.addListener(listener);

		future.awaitUninterruptibly();

		channel = future.getChannel();

		channel.setAttachment(this);
	}

	/**
	 * 连接失败，失败后重连
	 *
	 * @param cancelled
	 * @param cause
	 */
	public void onConnectFailure(Throwable cause) {
		if (!ctx.close) {
			log.warn("connect to '" + this.address.getSocketAddress()
					+ "' failed, and will retry in " + ctx.reconnectInterval
					+ " seconds later");
			this.reconnectTimer.schedule(new CacheClientReconnectTimerTask(
					this, ctx), ctx.reconnectInterval * 1000);
		}
	}

	/**
	 * 连接成功，成功后发送deleteTimeOut
	 */
	public void onConnected() {
		log.info("connect to '" + this.address.getSocketAddress()
				+ "' success...");
	}
	public String getAddr(){
		return this.address.getSocketAddress().toString();
	}
	/**
	 * 接收消息
	 *
	 * @param message
	 */
	public void onMessageReceived(ChannelBuffer buff) {

		int messageSize = buff.readableBytes();
		ChannelBuffer combinedMessage = ChannelBuffers.buffer(messageSize
				+ addBuffer.readableBytes());
		if (addBuffer.readable()) {
			messageSize += addBuffer.readableBytes();
			combinedMessage.writeBytes(addBuffer, 0, addBuffer.readableBytes());
			addBuffer.clear();
			addBuffer = ChannelBuffers.dynamicBuffer();
		}
		combinedMessage.writeBytes(buff);

		read(combinedMessage, messageSize,0);

	}

	public void read(ChannelBuffer combinedMessage, int messageSize,int index) {
		ChannelBuffer headerBf = ChannelBuffers.buffer(HEADER_LEN);

		if (messageSize < HEADER_LEN) {
			addBuffer.writeBytes(combinedMessage, index, messageSize);
//			combinedMessage.readBytes(addBuffer, Math.min(combinedMessage
//					.readableBytes(), HEADER_LEN));
		} else {

			combinedMessage.getBytes(index, headerBf);
			// 取得一个包的长度
			int packageLen = 0;
			try {
				ResponseMsg h = new ResponseMsg();
				h.decHread(headerBf);
				packageLen = (h.getMsgLen()) + HEADER_LEN;
			} catch (Exception ex) {
				log.error("package error");
			}
			if (messageSize == packageLen) {
				ChannelBuffer message = ChannelBuffers.buffer(packageLen);
				combinedMessage.getBytes(index, message);

				unpackInPool(message);

				message = null;

			} else if (messageSize > packageLen) {
				ChannelBuffer message = ChannelBuffers.buffer(packageLen);
				combinedMessage.getBytes(index, message);
				unpackInPool(message);
				index += packageLen;
				messageSize = messageSize-packageLen;
//				int len = messageSize - packageLen;
//				ChannelBuffer buff = ChannelBuffers.buffer(len);
//				buff.writeBytes(combinedMessage, packageLen, len);
				read(combinedMessage, messageSize,index);
			} else if (messageSize < packageLen) {
				addBuffer.writeBytes(combinedMessage, index, messageSize);
			}
		}
	}

	public synchronized boolean write(ChannelBuffer msg) {
		boolean isConnected = this.isConnected();
		if (isConnected) {
			this.channel.write(msg);
		}
		return isConnected;
	}

	/**
	 * 调用回调函数
	 *
	 * @param message
	 */
	private void unpackInPool(ChannelBuffer message) {
		this.unpackThreadPool.execute(new UnMessage(message, ctx,
				this.deleteTimeOutTimerTask));
	}

	public synchronized boolean isConnected() {
		return this.channel != null && this.channel.isConnected();
	}

	/**
	 * 关闭系统
	 */
	public synchronized void close() {
		reconnectTimer.cancel();
		reconnectTimer.purge();

		wheelTimer.stop();

		deleteTimeoutTimer.purge();
		deleteTimeoutTimer.cancel();

		if (channel != null) {
			channel.close();
			channel.getCloseFuture().awaitUninterruptibly();
		}

		this.clientBootstrap.releaseExternalResources();

		if (unpackThreadPool != null) {
			this.unpackThreadPool.shutdown();
			try {
				this.unpackThreadPool.awaitTermination(Long.MAX_VALUE,
						TimeUnit.SECONDS);
			} catch (InterruptedException e) {
			}
		}
		log.info("addr:"+getAddr()+" close!");
	}

	private class UnpackThreadFactory implements ThreadFactory {
		private final AtomicInteger idGen = new AtomicInteger();

		@Override
		public Thread newThread(Runnable r) {
			String name = "unpack-task-" + idGen.addAndGet(1);
			Thread t = new Thread(r, name);
			return t;
		}
	}
}
