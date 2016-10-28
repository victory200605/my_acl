package com.asiainfo.aibc.ailibEx4j.BillQuery;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class ClsRequestQuery implements ClsBody {
    private int ciOffNum;
    private int ciMaxRecordCnt;
    private ClsStIndexReqest[] coNumberIndex = null;
    private ClsStIndexReqest[] coIDIndex = null;
    private String ccStartTime;
    private String ccEndTime;
    private byte ciIsSearchAppendBill;

    public ClsRequestQuery() {
	ciOffNum = 0;
	ciMaxRecordCnt = 0;
	coNumberIndex = new ClsStIndexReqest[MAX_NUMBERINDEX_NUM];
	coIDIndex = new ClsStIndexReqest[MAX_IDINDEX_NUM];
	ccStartTime = null;
	ccEndTime = null;
	ciIsSearchAppendBill = (byte) 0;

	Arrays.fill(coNumberIndex, new ClsStIndexReqest());
	Arrays.fill(coIDIndex, new ClsStIndexReqest());
    }

    private class ClsStIndexReqest {
	public byte isSeted;
	public byte[] ccNumber;

	public ClsStIndexReqest() {
	    isSeted = (byte) 0;
	    ccNumber = new byte[MAX_NUMBER_LEN];
	}
    }

    public ByteBuffer Encode(ByteBuffer aoBlock) {
	if (getStartTime().length() != 14 || getEndTime().length() != 14) {
	    return null;
	}

	aoBlock.putInt(getOffNum());
	aoBlock.putInt(getMaxRecordCnt());
	for (int i = 0; i < MAX_NUMBERINDEX_NUM; i++) {
	    aoBlock.put(isSetNum(i));
	    aoBlock.put(getNumber(i));
	}

	for (int j = 0; j < MAX_IDINDEX_NUM; j++) {
	    aoBlock.put(isSetID(j));
	    aoBlock.put(getID(j));
	}

	aoBlock.put((getStartTime()).getBytes());
	aoBlock.put((byte) 0);
	aoBlock.put((getEndTime()).getBytes());
	aoBlock.put((byte) 0);

	aoBlock.put(getCiIsSearchAppendBill());
	aoBlock.put((byte) 0).put((byte) 0).put((byte) 0);

	aoBlock.flip();

	return aoBlock;
    }

    public int Decode(ByteBuffer aoBlock) {
	return 0;
    }

    public int getOffNum() {
	return ciOffNum;
    }

    public int getMaxRecordCnt() {
	return ciMaxRecordCnt;
    }

    public byte isSetNum(int aiIdx) {
	return coNumberIndex[aiIdx].isSeted;
    }

    public byte[] getNumber(int aiIdx) {
	return coNumberIndex[aiIdx].ccNumber;
    }

    public byte isSetID(int aiIdx) {
	return coIDIndex[aiIdx].isSeted;
    }

    public byte[] getID(int aiIdx) {
	return coIDIndex[aiIdx].ccNumber;
    }

    public String getStartTime() {
	return ccStartTime;
    }

    public String getEndTime() {
	return ccEndTime;
    }

    public boolean isSearchAppendBill() {
	return ciIsSearchAppendBill == (byte) 1;
    }

    public void setOffNum(int ciOffNum) {
	this.ciOffNum = ciOffNum;
    }

    public void setMaxRecordCnt(int ciMaxRecordCnt) {
	this.ciMaxRecordCnt = ciMaxRecordCnt;
    }

    public int setNumberIndex(int aiIdx, byte[] asNumber) {
	if (aiIdx >= MAX_NUMBERINDEX_NUM) {
	    return -1;
	}

	coNumberIndex[aiIdx] = new ClsStIndexReqest();

	coNumberIndex[aiIdx].isSeted = 1;
	coNumberIndex[aiIdx].ccNumber = asNumber;

	return 0;
    }

    public int setIDIndex(int aiIdx, byte[] asNumber) {
	if (aiIdx >= MAX_NUMBERINDEX_NUM) {
	    return -1;
	}

	coIDIndex[aiIdx].isSeted = 1;
	coIDIndex[aiIdx].ccNumber = asNumber;

	return 0;
    }

    public void setStartTime(String asStartTime) {
	this.ccStartTime = asStartTime;
    }

    public void setEndTime(String asEndTime) {
	this.ccEndTime = asEndTime;
    }

    public void setSearchAppendBill() {
	this.ciIsSearchAppendBill = (byte) 1;
    }

    public byte getCiIsSearchAppendBill() {
	return ciIsSearchAppendBill;
    }
}
