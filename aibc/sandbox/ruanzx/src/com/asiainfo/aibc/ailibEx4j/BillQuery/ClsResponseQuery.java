package com.asiainfo.aibc.ailibEx4j.BillQuery;

import java.nio.ByteBuffer;
import java.util.Vector;

public class ClsResponseQuery implements ClsBody {
    private int ciStat;
    private int ciRecordCnt;
    private int ciTotalRecordCnt;
    private int ciUsedSec;
    private Vector coRecords = null;

    public ClsResponseQuery() {
	ciStat = 0;
	ciRecordCnt = 0;
	ciTotalRecordCnt = 0;
	ciUsedSec = 0;
	coRecords = new Vector();
    }

    public ByteBuffer Encode(ByteBuffer aoBlock) {
	return null;
    }

    public int Decode(ByteBuffer aoBlock) {
	ciStat = aoBlock.getInt();
	ciRecordCnt = aoBlock.getInt();
	ciTotalRecordCnt = aoBlock.getInt();
	ciUsedSec = aoBlock.getInt();

	for (int liIt = 0; liIt < ciRecordCnt; liIt++) {
	    int liLen = 0;
	    liLen = aoBlock.getInt();

	    byte[] b = new byte[liLen];
	    aoBlock.get(b, 0, liLen);

	    coRecords.add(new String(b));
	}
	return 0;
    }

    public void SetTotalRecordCnt(int aiTotalRecordCnt) {
	ciTotalRecordCnt = aiTotalRecordCnt;
    }

    public void SetUsedSec(int aiSec) {
	ciUsedSec = aiSec;
    }

    public void SetStat(int aiStat) {
	ciStat = aiStat;
    }

    public int GetRecordCnt() {
	return coRecords.size();
    }

    public int GetTotalRecordCnt() {
	return ciTotalRecordCnt;
    }

    public int GetUsedSec() {
	return ciUsedSec;
    }

    public Vector getCoRecords() {
	return coRecords;
    }

    public int getCiStat() {
	return ciStat;
    }
}
