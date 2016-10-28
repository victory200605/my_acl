package com.asiainfo.aibc.ailibEx4j.BillQuery;

import java.nio.ByteBuffer;

public class ClsHeader {
    public static final int INT_FIELD_SIZE = 4;
    private int ciCmd;
    private ClsBody coBody;

    public ClsHeader() {
	ciCmd = 0;
	coBody = null;
    }

    public ClsHeader(int aiCmd, ClsBody aoBody) {
	this.ciCmd = aiCmd;
	this.coBody = aoBody;
    }

    public int Decode(ByteBuffer aoBlock) {
	aoBlock.flip();
	if (aoBlock.limit() < INT_FIELD_SIZE) {
	    return -1;
	}

	ciCmd = aoBlock.getInt();

	if (coBody != null) {
	    return coBody.Decode(aoBlock);
	}
	return 0;
    }

    public ByteBuffer Encode(ByteBuffer aoBlock) {
	if (aoBlock.capacity() < INT_FIELD_SIZE) {
	    return null;
	}

	aoBlock.putInt(ciCmd);

	if (coBody != null) {
	    coBody.Encode(aoBlock);
	    return aoBlock;
	}

	return null;
    }

    public int GetCmd() {
	return ciCmd;
    }
}
