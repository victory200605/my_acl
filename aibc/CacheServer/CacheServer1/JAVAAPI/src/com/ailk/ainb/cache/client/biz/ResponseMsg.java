/*
 *Copyright(c) 1995-2013 by Ailk(bish)
 *All rights reserved.
 */
package com.ailk.ainb.cache.client.biz;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.jboss.netty.buffer.ChannelBuffer;

public class ResponseMsg{
	private static final Log log = LogFactory.getLog(ResponseMsg.class);
	private int msgId;
	/**
	 * 服务端返回非0时表示通讯出错
	 */
	private int result;
	/**
	 * 消息总长度
	 */
	private int msgLen;

	/**
	 * 消息总长度
	 *
	 */
	private int allLen;

	/**
	 * 命令或响应类型
	 */
	private int cmdId;
	/**
	 * 获取头部12个字节信息
	 * @param hread
	 */
	public void decHread(ChannelBuffer hread) {
		this.msgId = hread.readInt();
		this.result = hread.readInt();
		this.msgLen = hread.readInt();
	}

	public int getCmdId() {
		return cmdId;
	}

	public void setCmdId(int cmdId) {
		this.cmdId = cmdId;
	}


	public int getMsgId() {
		return msgId;
	}


	public void setMsgId(int msgId) {
		this.msgId = msgId;
	}


	public int getResult() {
		return result;
	}


	public void setResult(int result) {
		this.result = result;
	}

	public int getMsgLen() {
		return msgLen;
	}

	public void setMsgLen(int msgLen) {
		this.msgLen = msgLen;
	}

	public int getAllLen() {
		return allLen;
	}

	public void setAllLen(int allLen) {
		this.allLen = allLen;
	}
	/**
	 * 处理消息
	 * @param message
	 * @return
	 */
	public Object decValue(ChannelBuffer message) {

		this.msgId = message.readInt();

		if(msgId==-1){
			if(log.isDebugEnabled()){
				 log.debug("get heartbeat message:"+message);
			 }
			return null;
		}

		this.result = message.readInt();
		this.msgLen = message.readInt();

		if(result!=0){
			Response res = new Response();
			res.setMsgId(msgId);
			res.setResult(result);
			if(log.isDebugEnabled()){
				 log.debug("response message msgId:"+msgId+",result:"+res.getResult());
			 }
			return res;
		}

		this.allLen = message.readInt();


		this.cmdId = message.readInt();
		//返回插入结果信息
		if (this.cmdId == Constants.RESP_INSERT) {
			ResponseInsert res = new ResponseInsert();
			res.setMsgId(msgId);
			res.setResult(message.readInt());
			if(log.isDebugEnabled()){
				 log.debug("response insert message msgId:"+msgId+",result:"+res.getResult());
			 }
			return res;
		}
		//返回删除结果信息
		else if (this.cmdId == Constants.RESP_DELETE) {
			ResponseDelete res = new ResponseDelete();
			res.setMsgId(msgId);
			res.setResult(message.readInt());
			if(log.isDebugEnabled()){
				 log.debug("response delete message msgId:"+msgId+",result:"+res.getResult());
			 }
			return res;
		}
		//返回获取结果信息
		else if (this.cmdId == Constants.RESP_FETCH) {

			ResponseFetch res = new ResponseFetch();
			res.setMsgId(msgId);
			int result = message.readInt();
			res.setResult(result);

			if(result == Constants.CORRECT){
				int valueSize = message.readInt();
				byte value[] = new byte[valueSize];
				message.readBytes(value);
				res.setValue(new String(value));
				if(log.isDebugEnabled()){
					 log.debug("response fetch message msgId:"+msgId+",result:"+result+",value:"+res.getValue());
				 }
			}else{
				if(log.isDebugEnabled()){
					 log.debug("response fetch message msgId:"+msgId+",result:"+result);
				 }
			}
			return res;

		}
		//返回删除超时结果信息
		else if (this.cmdId == Constants.RESP_DELTIMEOUT) {
			ResponseDeleteTimeOut res = new ResponseDeleteTimeOut();
			res.setMsgId(msgId);
			int result = message.readInt();
			res.setResult(result);

			if(result == Constants.CORRECT){
				int keySize = message.readInt();
				byte key[] = new byte[keySize];
				message.readBytes(key);
				res.setKey(new String(key));

				int valSize = message.readInt();
				byte value[] = new byte[valSize];
				message.readBytes(value);
				res.setValue(new String(value));
				if(log.isDebugEnabled()){
					 log.debug("response deleteTimeOut message msgId:"+msgId+",result:"+result+",key:"+res.getKey()+",value:"+res.getValue());
				 }
			}else{
				if(log.isDebugEnabled()){
					 log.debug("response deleteTimeOut message msgId:"+msgId+",result:"+result);
				 }
			}
			return res;
		}
		return null;
	}

}
