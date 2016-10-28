#ifndef __AILIBEX__AISPLAYTREE_H__
#define __AILIBEX__AISPLAYTREE_H__

///start namespace
AIBC_NAMESPACE_START

/*
 * SPLAY TREE OPERATE MACROS.
 */

/** 
 * @brief 在二叉树mpRootNode内找到新节点mpKeyNode可以插入而不破坏树顺序的地方
 *
 * @param mpNodeType    节点类型
 * @param mpID          用来于cp连接组成该节点的一个字段,代表一维顺序
 * @param mpComp        比较节点大小顺序的传入函数
 * @param mpRootNode    进行比较的根节点,从此可以遍历根节点连接的所有节点
 * @param mpKeyNode     新节点
 * @param mpFlag        标志,掩码,一般留用
 *
 * @return 可以插入而不破坏顺序的临界地方
 */
#define  AISPLAY_ACCESSNODE( mpNodeType, mpID, mpComp, mpRootNode, mpKeyNode, mpFlag ) \
{ \
	int				nRetCode; \
	mpNodeType		stHeadNode; \
	mpNodeType		*pTmpNode = NULL; \
	mpNodeType		*pMaxNode, *pMinNode; \
	stHeadNode.cp##mpID[0] = stHeadNode.cp##mpID[1] = NULL; \
	pMaxNode = pMinNode = &stHeadNode; \
	while( (nRetCode = mpComp( mpKeyNode, mpRootNode, mpFlag )) ) \
	{ \
		if( nRetCode < 0 ) \
		{ \
			if( mpRootNode->cp##mpID[0] == NULL ) \
				break; \
			if( mpComp( mpKeyNode, mpRootNode->cp##mpID[0], mpFlag ) < 0 ) \
			{ \
				pTmpNode = mpRootNode->cp##mpID[0]; \
				mpRootNode->cp##mpID[0] = pTmpNode->cp##mpID[1]; \
				pTmpNode->cp##mpID[1] = mpRootNode; \
				mpRootNode = pTmpNode; \
				if( mpRootNode->cp##mpID[0] == NULL ) \
					break; \
			} \
			pMinNode->cp##mpID[0] = mpRootNode, pMinNode = mpRootNode; \
			mpRootNode = mpRootNode->cp##mpID[0]; \
		} \
		else if( nRetCode > 0 ) \
		{ \
			if( mpRootNode->cp##mpID[1] == NULL ) \
				break; \
			if( mpComp( mpKeyNode, mpRootNode->cp##mpID[1], mpFlag ) > 0 ) \
			{ \
				pTmpNode = mpRootNode->cp##mpID[1]; \
				mpRootNode->cp##mpID[1] = pTmpNode->cp##mpID[0]; \
				pTmpNode->cp##mpID[0] = mpRootNode; \
				mpRootNode = pTmpNode; \
				if( mpRootNode->cp##mpID[1] == NULL ) \
					break; \
			} \
			pMaxNode->cp##mpID[1] = mpRootNode, pMaxNode = mpRootNode; \
			mpRootNode = mpRootNode->cp##mpID[1]; \
		} \
		else \
		{ \
			break; \
		} \
	} \
	pMaxNode->cp##mpID[1] = mpRootNode->cp##mpID[0]; \
	pMinNode->cp##mpID[0] = mpRootNode->cp##mpID[1]; \
	mpRootNode->cp##mpID[0] = stHeadNode.cp##mpID[1]; \
	mpRootNode->cp##mpID[1] = stHeadNode.cp##mpID[0]; \
	return  (mpRootNode); \
}


/** 
 * @brief 在二叉树 mpRootNode内插入 新节点mpKeyNode
 *
 * @param mpNodeType    节点类型
 * @param mpID          用来于cp连接组成该节点的一个字段,代表一维顺序
 * @param mpComp        比较节点大小顺序的传入函数
 * @param mpRootNode    进行比较的根节点,从此可以遍历根节点连接的所有节点
 * @param mpKeyNode     新节点
 * @param mpFlag        标志或掩码,一般留用
 *
 * @return NULL         失败, 
 *         <other>      做为新的 mpRootNode节点返回.
 */
#define  AISPLAY_ADDNEWNODE( mpNodeType, mpID, mpComp, mpRootNode, mpNewNode, mpBalanceFunc, mpFlag ) \
{ \
	int			nRetCode = 0; \
	if( mpNewNode == (mpNodeType *)NULL ) \
		return  (NULL); \
	if( mpRootNode == NULL ) \
	{ \
		mpNewNode->cp##mpID[0] = mpNewNode->cp##mpID[1] = NULL; \
		mpRootNode = mpNewNode; \
		return  (mpRootNode); \
	} \
	mpRootNode = mpBalanceFunc( mpRootNode, mpNewNode ); \
	if( (nRetCode = mpComp( mpNewNode, mpRootNode, mpFlag )) < 0 ) \
	{ \
		mpNewNode->cp##mpID[0] = mpRootNode->cp##mpID[0]; \
		mpNewNode->cp##mpID[1] = mpRootNode; \
		mpRootNode->cp##mpID[0] = NULL; \
		mpRootNode = mpNewNode; \
		return  (mpRootNode); \
	} \
	if( nRetCode > 0 ) \
	{ \
		mpNewNode->cp##mpID[1] = mpRootNode->cp##mpID[1]; \
		mpNewNode->cp##mpID[0] = mpRootNode; \
		mpRootNode->cp##mpID[1] = NULL; \
		mpRootNode = mpNewNode; \
		return  (mpRootNode); \
	} \
	return  (NULL); \
}


/** 
 * @brief 在二叉树 mpRootNode内搜索跟mpKeyNode相等的节点然后删除它
 *
 * @param mpNodeType    节点类型
 * @param mpID          用来于cp连接组成该节点的一个字段,代表一维顺序
 * @param mpComp        比较节点大小顺序的传入函数
 * @param mpRootNode    进行比较的根节点,从此可以遍历根节点连接的所有节点
 * @param mpKeyNode     用此节点来确定删除二叉树中跟它相等的节点
 * @param mpFlag        标志或掩码,一般留用
 *
 * @return NULL     没找到需要删除的节点或根节点为空. 
 *         <other>  需要删除的节点
 */
#define  AISPLAY_DELOLDNODE( mpNodeType, mpID, mpComp, mpRootNode, mpKeyNode, mpBalanceFunc, mpFlag ) \
{ \
	mpNodeType		*pTmpNode = NULL; \
	if( mpRootNode == (mpNodeType *)NULL ) \
		return  (NULL); \
	mpRootNode = mpBalanceFunc( mpRootNode, mpKeyNode ); \
	if( mpComp( mpKeyNode, mpRootNode, mpFlag ) != 0 ) \
		return  (NULL); \
	if( mpRootNode->cp##mpID[0] == NULL ) \
	{ \
		mpNodeType		*pRetNode = NULL; \
		pTmpNode = mpRootNode->cp##mpID[1]; \
		pRetNode = mpRootNode; \
		mpRootNode = pTmpNode; \
		return  (pRetNode); \
	} \
	pTmpNode = mpRootNode; \
	mpRootNode = mpRootNode->cp##mpID[0]; \
	mpRootNode = mpBalanceFunc( mpRootNode, mpKeyNode ); \
	mpRootNode->cp##mpID[1] = pTmpNode->cp##mpID[1]; \
	return  (pTmpNode); \
}


/** 
 * @brief 在二叉树 mpRootNode内搜索跟mpKeyNode相等的节点
 *
 * @param mpNodeType    节点类型
 * @param mpID          用来于cp连接组成该节点的一个字段,代表一维顺序
 * @param mpComp        比较节点大小顺序的传入函数
 * @param mpRootNode    进行比较的根节点,从此可以遍历根节点连接的所有节点
 * @param mpKeyNode     用此节点来确定二叉树中跟它相等的节点
 * @param mpFlag        标志或掩码,一般留用
 *
 * @return NULL     没找到相等的节点或根节点为空. 
 *         <other>  相等的节点
 */
#define  AISPLAY_SEARCHNODE( mpNodeType, mpID, mpComp, mpRootNode, mpKeyNode, mpBalanceFunc, mpFlag ) \
{ \
	mpNodeType		*pTmpNode = NULL; \
	if( mpRootNode == (mpNodeType *)NULL ) \
		return  (NULL); \
	pTmpNode = mpBalanceFunc( mpRootNode, mpKeyNode ); \
	if( mpComp( pTmpNode, mpKeyNode, mpFlag ) != 0 ) \
		return  (NULL); \
	return  (pTmpNode); \
}


/** 
 * @brief 在二叉树 mpRootNode内搜索可以插入mpKeyNode的地方
 *
 * @param mpNodeType    节点类型
 * @param mpID          用来于cp连接组成该节点的一个字段,代表一维顺序
 * @param mpComp        比较节点大小顺序的传入函数
 * @param mpRootNode    进行比较的根节点,从此可以遍历根节点连接的所有节点
 * @param mpKeyNode     用此节点来确定二叉树中可以插入它的地方
 * @param mpFlag        标志或掩码,一般留用
 *
 * @return NULL     根节点为空. 
 *         <other>  可以插入新节点的地方
 */
#define  AISPLAY_SEARCHNODE2( mpNodeType, mpID, mpComp, mpRootNode, mpKeyNode, mpBalanceFunc, mpFlag ) \
{ \
	if( mpRootNode == (mpNodeType *)NULL ) \
		return  (NULL); \
	mpRootNode = mpBalanceFunc( mpRootNode, mpKeyNode ); \
	return  (mpRootNode); \
}

///end namespace
AIBC_NAMESPACE_END

#endif // __AISPLAYTREE_H__

