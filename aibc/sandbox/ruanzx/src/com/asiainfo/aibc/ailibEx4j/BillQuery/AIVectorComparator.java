package com.asiainfo.aibc.ailibEx4j.BillQuery;

import java.util.Comparator;
import java.util.Vector;

public class AIVectorComparator implements Comparator {
    private int ciOrderBy;

    public AIVectorComparator(int aiOrderBy) {
	ciOrderBy = aiOrderBy;
    }

    public int compare(Object o1, Object o2) {
	Vector v1 = (Vector) o1;
	Vector v2 = (Vector) o2;

	String s1 = (String) v1.get(ciOrderBy);
	String s2 = (String) v2.get(ciOrderBy);

	return s1.compareTo(s2);
    }

}
