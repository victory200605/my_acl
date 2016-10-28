package com.asiainfo.aibc.ailibEx4j.BillQuery;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Collections;
import java.util.Vector;

import com.asiainfo.aibc.ailibEx4j.AIAsyncTcp.*;

public class MainQuery {
    public static String BILLMANA_CONFIG = "config/BillMana.ini";
    public static String BILLQUERY_CONFIG = "config/BillQuery.ini";

    private static int MAX_REV_BUFF = 40960;
    private static int MAX_SEND_BUFF = 10240;

    public static void main(String[] args) throws IOException, InterruptedException, ClassNotFoundException {
	String serverIP = "127.0.0.1";
	int serverPort = 0;
	String lcDel = " ";
	int liOrderBy = 0;
	boolean lbIsSearchAppendBill = false;
	ClsRequestQuery loQuery = new ClsRequestQuery();
	Vector loRs = new Vector();

	AIGetOpt go = new AIGetOpt("s:p:x:y:z:i:b:e:o:n:t:r:va");
	char liOptChar;
	while ((liOptChar = go.getopt(args)) != 0) {
	    switch (liOptChar) {
	    case 's':
		serverIP = go.optarg();
		break;
	    case 'p':
		serverPort = Integer.parseInt(go.optarg());
		break;
	    case 't':
		lcDel = go.optarg();
		break;
	    case 'r':
		liOrderBy = Integer.parseInt(go.optarg());
		break;
	    case 'x':
		loQuery.setNumberIndex(0, go.optarg().getBytes());
		break;
	    case 'y':
		loQuery.setNumberIndex(1, go.optarg().getBytes());
		break;
	    case 'z':
		loQuery.setNumberIndex(2, go.optarg().getBytes());
		break;
	    case 'i':
		loQuery.setNumberIndex(3, go.optarg().getBytes());
		break;
	    case 'b':
		loQuery.setStartTime(go.optarg());
		break;
	    case 'e':
		loQuery.setEndTime(go.optarg());
		break;
	    case 'o':
		loQuery.setOffNum(Integer.parseInt(go.optarg()));
		break;
	    case 'n':
		loQuery.setMaxRecordCnt(Integer.parseInt(go.optarg()));
		break;
	    case 'a':
		lbIsSearchAppendBill = true;
		break;
	    case 'v':
		System.err.println("s - server ip");
		System.err.println("p - port");
		System.err.println("t - out put field separator");
		System.err.println("r - order by field index start 1");
		System.err.println("x - Number Index 0");
		System.err.println("y - Number Index 1");
		System.err.println("z - Number Index 2");
		System.err.println("i - Number Index 3");
		System.err.println("b - begin time exp:20080301010101");
		System.err.println("e - end time exp:20080301010101");
		System.err.println("o - result set offset");
		System.err.println("n - result set max return count");
		break;
	    }
	}

	if (args.length <= 1) {
	    System.out.println("-v for help");
	    return;
	}

	if (liOrderBy < 0) {
	    System.out.println("Error:please input valid order by field start 1\n ");
	    return;
	}

	if (loQuery.getStartTime().length() != 14 || loQuery.getEndTime().length() != 14
		|| Long.parseLong(loQuery.getStartTime()) > Long.parseLong(loQuery.getEndTime())) {
	    System.out.println("Error:please input valid query time value, exp:20080301010101\n ");
	    return;
	}

	AsyncClient loAsyncClient = null;
	try {
	    loAsyncClient = new AsyncClient(serverIP, serverPort, 1, 50000);
	} catch (IOException e) {
	    e.printStackTrace();
	}

	// ///////////////////////////BILLQUERY CONFIG///////////
	String lsOutFormat;
	int liSearchAppendBill = 0;
	int liAlignment = 0; // 0 : left, 1 : right

	lsOutFormat = AIConfig.getProfileString(BILLQUERY_CONFIG, "BillQuery", "OutFormat", "default");
	liSearchAppendBill = Integer.parseInt(AIConfig.getProfileString(BILLQUERY_CONFIG, "BillQuery",
		"SearchAppendBill", "0"));
	liAlignment = Integer.parseInt(AIConfig.getProfileString(BILLQUERY_CONFIG, "BillQuery", "Alignment", "0"));

	// ///////////////////////////BILLMANA CONFIG/////////////
	String lcFieldWidth;
	String lcFieldSeparator = null;
	int liFieldFormat;

	liFieldFormat = Integer.parseInt(AIConfig.getProfileString(BILLMANA_CONFIG, "BillMana", "FieldFormat", "0"));
	lcFieldWidth = AIConfig.getProfileString(BILLMANA_CONFIG, "BillMana", "FieldWidth", "");

	if (liFieldFormat != 0) {
	    lcFieldSeparator = AIConfig.getProfileString(BILLMANA_CONFIG, "BillMana", "FieldSeparator", "");
	}

	String[] lsTmpSpliter;
	lsTmpSpliter = lcFieldWidth.split(",");

	for (int i = 0; i < ClsBody.MAX_NUMBERINDEX_NUM; i++) {
	    if (loQuery.isSetNum(i) == 0) {
		continue;
	    }

	    int liFieldID;
	    liFieldID = Integer.parseInt(AIConfig.getProfileString(BILLMANA_CONFIG, "BillMana", "NumberIndex"
		    + Integer.toString(i), "-1"));
	    if (liFieldID == -1) {
		continue;
	    }

	    int liFieldWidth = Integer.parseInt(lsTmpSpliter[liFieldID - 1]);

	    loQuery.setNumberIndex(i, FixString(loQuery.getNumber(i), liFieldWidth, " ", liAlignment));
	}
	//
	// ////////////////////////////////////////////////////////

	Vector lvOutFormat = new Vector();
	String[] loSpliterTmp = lsOutFormat.split(",");

	for (int i = 0; i < loSpliterTmp.length; i++) {
	    lvOutFormat.add(i, new Integer(loSpliterTmp[i]));
	}

	if (lbIsSearchAppendBill == true || liSearchAppendBill == 1) {
	    loQuery.setSearchAppendBill();
	}

	// ///////////////////////////////////////////////////////////////
	ClsHeader loHeader = new ClsHeader(1, loQuery);

	ByteBuffer bbSendBlock = ByteBuffer.allocate(MAX_SEND_BUFF);
	bbSendBlock.clear();
	bbSendBlock.order(ByteOrder.BIG_ENDIAN);

	// encode
	loHeader.Encode(bbSendBlock);

	byte[] loTmpChunk = new byte[bbSendBlock.limit()];
	bbSendBlock.get(loTmpChunk, 0, loTmpChunk.length);

	loAsyncClient.connectTo();

	AsyncMessage aMsg = new AsyncMessage();
	aMsg = loAsyncClient.sendRequestAndGetResponse(loTmpChunk);

	if (aMsg != null) {
	    ClsHeader loRepHeader = new ClsHeader();
	    // decode
	    ByteBuffer bbRecvBlock = ByteBuffer.allocate(MAX_REV_BUFF);
	    bbRecvBlock.put(aMsg.getContent());

	    loRepHeader.Decode(bbRecvBlock);

	    switch (loRepHeader.GetCmd()) {
	    case 3: {
		ClsResponseError loRepError = new ClsResponseError();
		loRepError.Decode(bbRecvBlock);

		System.out.println("Error:" + loRepError.getMsg() + " - StatCode:" + loRepError.getStat());
		break;
	    }
	    case 2: {
		ClsResponseQuery loResponseQuery = new ClsResponseQuery();
		loResponseQuery.Decode(bbRecvBlock);

		if (liFieldFormat != 0) {
		    Prepare1(lcFieldSeparator, loResponseQuery, loRs, liOrderBy - 1);
		} else {
		    prepare2(lsTmpSpliter, loResponseQuery, loRs, liOrderBy - 1);
		}

		PrintOut(loRs, lvOutFormat, lcDel);

		System.out.println("--------------------------------------------------------------------------\n");
		System.out.println("Query Max Record Count   = " + loQuery.getMaxRecordCnt());
		System.out.println("Return Record Count      = " + loResponseQuery.GetRecordCnt());
		System.out.println("Total Record Count       = " + loResponseQuery.GetTotalRecordCnt());
		System.out.println("Real Query Used time(s)  = " + loResponseQuery.GetUsedSec());
		break;
	    }
	    default: {
		System.out.println(("Error : Receive response data exception\n"));
	    }
	    }
	    System.exit(0) ;
	}
	 System.exit(0);
    }

    private static void PrintOut(Vector loRs, Vector lvOutFormat, String acDel) {
	for (int i = 0; i < loRs.size(); i++) {
	    boolean lbFirst = true;

	    Vector lsRecord = (Vector) loRs.get(i);

	    for (int j = 1; j <= lvOutFormat.size(); j++) {
		if (lsRecord.size() <= ((Integer) lvOutFormat.get(j - 1)).intValue() - 1) {
		    System.out.println("Error:Format output out of range\n");
		    return;
		}

		if (!lbFirst) {
		    System.out.print(acDel);
		}

		System.out.print(lsRecord.get(j));
		lbFirst = false;
	    }
	    System.out.println();
	}
    }

    private static void prepare2(String[] asSplit, ClsResponseQuery aoResponseQuery, Vector aoRs, int aiOrderBy) {
	for (int i = 0; i < aoResponseQuery.getCoRecords().size(); i++) {
	    Vector loTmp = new Vector();

	    ByteBuffer bbBufTmp = ByteBuffer.allocate(((String) aoResponseQuery.getCoRecords().get(i)).length());
	    bbBufTmp = ByteBuffer.wrap(((String) aoResponseQuery.getCoRecords().get(i)).getBytes());

	    for (int j = 0; j < asSplit.length; j++) {
		byte[] bTmp = new byte[(new Integer(asSplit[j])).intValue()];

		bbBufTmp.get(bTmp, 0, bTmp.length);

		loTmp.add(new String(bTmp));
	    }
	    aoRs.add(loTmp);

	    if (aiOrderBy > 0) {
		Collections.sort(aoRs, new AIVectorComparator(aiOrderBy));
	    }
	}
    }

    private static void Prepare1(String acFieldSeparator, ClsResponseQuery aoResponseQuery, Vector aoRs, int aiOrderBy) {
	for (int it = 0; it < aoResponseQuery.getCoRecords().size(); it++) {
	    Vector loTmp = new Vector();
	    String loRecord = (String) aoResponseQuery.getCoRecords().get(it);

	    String[] loSplit = loRecord.split(acFieldSeparator);

	    for (int i = 1; i < loSplit.length; i++) {
		loTmp.add(loSplit[i]);
	    }
	    aoRs.add(loTmp);
	    if (aiOrderBy > 0) {
		Collections.sort(aoRs, new AIVectorComparator(aiOrderBy));
	    }
	}
    }

    private static byte[] FixString(byte[] asBuff, int aiWidth, String aCh, int aiAlignment) {
	ByteBuffer bbBuf = ByteBuffer.allocate(aiWidth + 1);
	bbBuf.clear();

	int liLen = asBuff.length;

	if (aiWidth <= liLen) {
	    bbBuf = ByteBuffer.wrap(asBuff);
	    return bbBuf.array();
	}

	if (aiAlignment == 0) {
	    bbBuf.put(asBuff);

	    for (int i = liLen; i < aiWidth; i++) {
		bbBuf.put(aCh.getBytes());
	    }
	    bbBuf.put((byte) 0);
	} else {
	    for (int i = 0; i < aiWidth - liLen; i++) {
		bbBuf.put(aCh.getBytes());
	    }
	    bbBuf.put(asBuff);
	    bbBuf.put((byte) 0);
	}
	return bbBuf.array();
    }
}
