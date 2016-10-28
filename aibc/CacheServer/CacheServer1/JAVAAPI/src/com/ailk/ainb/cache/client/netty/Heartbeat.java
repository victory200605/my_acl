/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.netty;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.handler.timeout.IdleState;
import org.jboss.netty.handler.timeout.IdleStateAwareChannelHandler;
import org.jboss.netty.handler.timeout.IdleStateEvent;

import com.ailk.ainb.cache.client.impl.ChannelBufferValue;
/**
 * 空闲发送心跳
 * @author Administrator
 *
 */
public class Heartbeat extends IdleStateAwareChannelHandler{
	private static final Log log = LogFactory.getLog(Heartbeat.class);
	@Override
	public void channelIdle(ChannelHandlerContext ctx, IdleStateEvent e)
			throws Exception {

		super.channelIdle(ctx, e);
		//System.out.println("================================"+e.getState());
		 if( e.getState() == IdleState.ALL_IDLE){
			 ctx.getChannel().write(ChannelBufferValue.getHeartBeat());
			 if(log.isDebugEnabled()){
				 log.debug("sent heartbeat message:"+ChannelBufferValue.getHeartBeat());
			 }
		 }
	}

}
