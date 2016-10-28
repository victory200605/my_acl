package com.asiainfo.aibc.ailibEx4j.BillQuery;

import java.nio.ByteBuffer;

public interface ClsBody {
    public static int MAX_NUMBERINDEX_NUM = 4;
    public static int MAX_IDINDEX_NUM = 1;
    public static final int MAX_NUMBER_LEN = 25;
    public static final int MAX_BUFF_SIZE = 4096;

    public abstract ByteBuffer Encode(ByteBuffer aoBlock);

    public abstract int Decode(ByteBuffer aoBlock);

}
