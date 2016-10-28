/*
*Copyright(c) 1995-2013 by Ailk(bish)
*All rights reserved.
*/
package com.ailk.ainb.cache.client.impl;

import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.buffer.ChannelBuffers;

import com.ailk.ainb.cache.client.biz.Constants;

public class ChannelBufferValue {
	/**
	 * 返回要发送删除数据的buffer值
	 * @param key
	 * @param type
	 * @param msgId
	 * @return
	 */
	public static ChannelBuffer getValue(String key, Integer type,Integer msgId) {
		byte[] keyValue = key.getBytes();
		int len = 12+keyValue.length;
		ChannelBuffer buffer = ChannelBuffers.buffer(len+12);
		//写入头部信息
		buffer.writeBytes(getHead(msgId,len));
		//写入内容信息
		buffer.writeBytes(getBody(keyValue, type, len));
		return buffer;
	}
	public static ChannelBuffer getBody(byte[] key, Integer type, int len) {
		ChannelBuffer buffer = ChannelBuffers.buffer(len);

		buffer.writeInt(len);
		buffer.writeInt(type);

		buffer.writeInt(key.length);
		buffer.writeBytes(key);

		return buffer;
	}
	/**
	 * 返回要发送插入数据的buffer值
	 * @param key
	 * @param value
	 * @param msgId
	 * @return
	 */
	public static ChannelBuffer getValue(String key,String value,Integer msgId){
		byte[] keyValue = key.getBytes();
		byte[] valueVal = value.getBytes();
		int len = 20+keyValue.length+valueVal.length;
		ChannelBuffer buffer = ChannelBuffers.buffer(len+12);
		//写入头部信息
		buffer.writeBytes(getHead(msgId,len));
		//写入内容信息
		buffer.writeBytes(getBody(keyValue, valueVal, len));
		return buffer;
	}

	public static ChannelBuffer getBody(byte key[],byte value[],int len){

		ChannelBuffer buffer = ChannelBuffers.buffer(len);

		buffer.writeInt(len);
		buffer.writeInt(Constants.REQ_INSERT);

		buffer.writeInt(1);
		buffer.writeInt(key.length);
		buffer.writeBytes(key);
		buffer.writeInt(value.length);
		buffer.writeBytes(value);

		return buffer;
	}
	public static  ChannelBuffer getHead(Integer msgId,int len){
		ChannelBuffer buffer = ChannelBuffers.buffer(12);

		buffer.writeInt(msgId);
		buffer.writeInt(0);
		buffer.writeInt(len);

		return buffer;
	}
	/**
	 * 发送心跳信息buffer值
	 * @return
	 */
	public static ChannelBuffer getHeartBeat(){
		ChannelBuffer buffer = ChannelBuffers.buffer(12);
		buffer.writeInt(-1);
		buffer.writeInt(0);
		buffer.writeInt(0);
		return buffer;
	}
	/**
	 * 返回要发送的删除超时信息buffer值
	 * @param msgId
	 * @param type
	 * @return
	 */
	public static ChannelBuffer getDeleteOutBuffer(Integer msgId,Integer type) {
		int len = 12;
		ChannelBuffer buffer = ChannelBuffers.buffer(len+12);
		buffer.writeBytes(getHead(msgId,len));

		ChannelBuffer body = ChannelBuffers.buffer(len);

		body.writeInt(len);
		body.writeInt(Constants.REQ_DELTIMEOUT);

		body.writeInt(type);

		buffer.writeBytes(body);

		return buffer;
	}
}
