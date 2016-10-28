package com.ailk.ainb.cache.client.server;

import java.net.SocketAddress;

public class ServerSocketAddress {
    private final SocketAddress socketAddress;
    /**
     * 最小的值
     */
    private final int minRange;
    /**
     * 最大的值
     */
    private final int maxRange;

    public ServerSocketAddress(SocketAddress socketAddress,int minRange,int maxRange) {
        this.socketAddress = socketAddress;
        this.minRange=minRange;
        this.maxRange=maxRange;
    }
    /**
     * 根据设置的值生成socketAddress的key值
     * @return
     */
    public String getKey(){
    	return ""+minRange+""+maxRange;
    }
    public SocketAddress getSocketAddress() {
        return socketAddress;
    }

    public int getMinRange() {
		return minRange;
	}

	public int getMaxRange() {
		return maxRange;
	}

	@Override
    public int hashCode() {
        return socketAddress.hashCode();    //To change body of overridden methods use File | Settings | File Templates.
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof ServerSocketAddress)) {
            return false;
        }
        ServerSocketAddress address = (ServerSocketAddress) obj;
        return socketAddress.equals(address.getSocketAddress());    //To change body of overridden methods use File | Settings | File
        // Templates.
    }

    @Override
    public String toString() {
        return socketAddress.toString();    //To change body of overridden methods use File | Settings | File Templates.
    }

}
