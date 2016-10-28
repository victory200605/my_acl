package com.ailk.ainb.cache.client.netty;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelStateEvent;
import org.jboss.netty.channel.ExceptionEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelHandler;

import com.ailk.ainb.cache.client.server.CacheClientContext;

/**
 * 处理netty的信息
 *
 * @author Administrator
 *
 */
public class CacheClientHandler extends SimpleChannelHandler

{
	private static final Log log = LogFactory.getLog(CacheClientHandler.class);
	private final CacheClientContext ctx;
	private final CacheClientNettyConnection conn;

	public CacheClientHandler(CacheClientContext ctx,CacheClientNettyConnection conn) {
		this.ctx = ctx;
		this.conn = conn;
	}

	/**
	 * 关闭线程
	 */
	@Override
	public void channelClosed(ChannelHandlerContext ctx, ChannelStateEvent e)
			throws Exception {

//		CacheClientNettyConnection conn = (CacheClientNettyConnection) ctx
//				.getChannel().getAttachment();
		log.info("addr:"+conn.getAddr()+" channel will be close!");
		conn.onConnectFailure(null);
//		conn.destroy(this.ctx.close);
	}

	/**
	 * 接收消息
	 */
	@Override
	public void messageReceived(ChannelHandlerContext ctx, MessageEvent e)
			throws Exception {

		ChannelBuffer message = (ChannelBuffer) e.getMessage();
//		CacheClientNettyConnection conn = (CacheClientNettyConnection) ctx
//				.getChannel().getAttachment();
		conn.onMessageReceived(message);

	}

	/**
	 * 连接成功
	 */
	@Override
	public void channelConnected(ChannelHandlerContext ctx, ChannelStateEvent e)
			throws Exception {

//		CacheClientNettyConnection conn = (CacheClientNettyConnection) ctx
//				.getChannel().getAttachment();
//		if(conn!=null)
		conn.onConnected();
	}

	/**
	 * 异常处理
	 */
	@Override
	public void exceptionCaught(ChannelHandlerContext ctx, ExceptionEvent e)
			throws Exception {
		//log.info(e.getCause());
		//e.getCause().printStackTrace();
		//e.getChannel().close();
//		if (e.getCause() instanceof IOException) {
//
//			CacheClientNettyConnection conn = (CacheClientNettyConnection) ctx
//					.getChannel().getAttachment();
//			if (!conn.isConnected()) {
//				conn.onConnectFailure(e.getCause());
//			} else {
//				System.out.println("already connected.");
//			}
//		}
	}
}
