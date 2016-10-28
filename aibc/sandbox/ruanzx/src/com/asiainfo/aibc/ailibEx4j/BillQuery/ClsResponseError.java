package com.asiainfo.aibc.ailibEx4j.BillQuery;

import java.nio.ByteBuffer;

public class ClsResponseError implements ClsBody {
    private int ciStat;
    private String coStrMsg;

    public int Decode(ByteBuffer aoBlock) {
	int liLen;
	ciStat = aoBlock.getInt();
	liLen = aoBlock.getInt();

	byte[] bTmpBuf = new byte[liLen];
	aoBlock.get(bTmpBuf, 0, liLen);

	coStrMsg = new String(bTmpBuf);

	return 0;
    }

    public ByteBuffer Encode(ByteBuffer aoBlock) {
	return null;
    }

    public void set(int aiStat, String apsMsg) {
	ciStat = aiStat;
	coStrMsg = apsMsg;
    }

    public String getMsg() {
	return coStrMsg;
    }

    public int getStat() {
	return ciStat;
    }
}
