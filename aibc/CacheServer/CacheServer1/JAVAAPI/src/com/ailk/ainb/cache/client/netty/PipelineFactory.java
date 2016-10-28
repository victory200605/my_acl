/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.netty;


import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.handler.timeout.IdleStateHandler;
import org.jboss.netty.util.Timer;

import com.ailk.ainb.cache.client.server.CacheClientContext;

public class PipelineFactory implements ChannelPipelineFactory {
	private final Timer timer;
	private final CacheClientContext ctx;
	private final CacheClientNettyConnection conn;
	public PipelineFactory(Timer timer,CacheClientContext ctx,CacheClientNettyConnection conn){
		this.timer=timer;
		this.ctx=ctx;
		this.conn = conn;
	}
	@Override
	public ChannelPipeline getPipeline() throws Exception {

		 ChannelPipeline pipeline = Channels.pipeline();
		 pipeline.addLast("timeout", new IdleStateHandler(timer, (int)ctx.keepAliveInterval, (int)ctx.keepAliveInterval, (int)ctx.keepAliveInterval));
		 pipeline.addLast("hearbeat", new Heartbeat());
		 pipeline.addLast("handler", new CacheClientHandler(ctx,conn));

		return pipeline;
	}

}
