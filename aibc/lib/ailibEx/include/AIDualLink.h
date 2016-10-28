/** 
 *  @file AIDualLink.h
 */

#ifndef  __AILIBEX__AIDUALLINK_H__
#define  __AILIBEX__AIDUALLINK_H__

#include "AILib.h"

///start namespace
AIBC_NAMESPACE_START

///////////////////////////////////////////////////////////////////
//
typedef struct LISTHEAD
{
	LISTHEAD	*cpNext;   ///<the next node
	LISTHEAD	*cpPrev;   ///<the preview node
} LISTHEAD;

///////////////////////////////////////////////////////////////////
//
typedef int (*pCOMPFunc)( LISTHEAD*, LISTHEAD* );

///////////////////////////////////////////////////////////////////
//
/**
 * @brief Intialize the head of the list.
 *
 * @param pHead    IN - the head of the list
 */
extern void ai_init_list_head( LISTHEAD *pHead );

/**
 * @brief Insert pNew in the middle of pPrev and pNext.
 *
 * @param pNew    IN - the insert item
 * @param pPrev   IN - the previous item
 * @param pNext   IN - the next item
 */
extern void ai_list_add_mid( LISTHEAD *pNew, LISTHEAD *pPrev, LISTHEAD *pNext );

/**
 * @brief Insert pNew after pHead.
 *
 * @param pNew    IN - the insert item
 * @param pHead   IN - the head of list
 */
extern void ai_list_add_head( LISTHEAD *pNew, LISTHEAD *pHead );

/**
 * @brief Insert pNew before pHead.
 *
 * @param pNew    IN - the insert item
 * @param pHead   IN - the head of list
 */
extern void ai_list_add_tail( LISTHEAD *pNew, LISTHEAD *pHead );

/**
 * @brief Get the first item of list into pRetHead.
 *
 * @param pRetHead    OUT - store the first item of list
 * @param pHead       IN - the head of the list
 */
extern void ai_list_peek_head( LISTHEAD *&pRetHead, LISTHEAD *pHead );

/**
 * @brief Get the last item of list into pRetHead.
 *
 * @param pRetHead    OUT - store the last item of list
 * @param pHead       IN - the last item of the list
 */
extern void ai_list_peek_tail( LISTHEAD *&pRetHead, LISTHEAD *pHead );

/**
 * @brief Delete the item between pPrev and pNext.
 *
 * @param pPrev    IN - the previous item
 * @param pNext    IN - the next item
 */
extern void ai_list_del_mid( LISTHEAD *pPrev, LISTHEAD *pNext );

/**
 * @brief Delete the specified item from the list.
 * 
 * @param pEntry    IN - the item you want to delete
 */
extern void ai_list_del_any( LISTHEAD *pEntry );

/**
 * @brief Delete the item afte pHead.
 *
 * @param pRetHead    IN/OUT - save the pointer of the item after pHead into it
 * @param pHead       IN - the specified item
 */
extern void ai_list_del_head( LISTHEAD *&pRetHead, LISTHEAD *pHead );

/**
 * @brief Delete the item before pHead.
 *
 * @param pRetHead    IN/OUT - save the pointer of the item before pHead into it
 * @param pHead       IN - the specified item
 */
extern void ai_list_del_tail( LISTHEAD *&pRetHead, LISTHEAD *pHead );

/**
 * @brief Move pEntry to the position after pHead.
 *
 * @param pEntry    IN - the item will be moved to the next position of pHead
 * @param pHead     IN - the specified item
 */
extern void ai_list_moveto_head( LISTHEAD *pEntry, LISTHEAD *pHead );

/**
 * @brief Move pEntry to the position before pHead.
 *
 * @param pEntry    IN - the item will be moved to the provious position of pHead
 * @param pHead     IN - the specified item
 */
extern void ai_list_moveto_tail( LISTHEAD *pEntry, LISTHEAD *pHead );

/**
 * @brief test the list is empty or not
 *
 * @param pHead    IN - the head of the list
 *
 * @return 1:empty,0:not empty
 */
extern bool ai_list_is_empty( LISTHEAD const *pHead );

/**
 * @brief Use pDestHead replace pSrcHead of the list,keep the old first item first of the src list.
 *
 * @param pSrcHead    IN - the old head of the list
 * @param pDestHead   OUT - the new head of the list
 */
extern void ai_list_join_head( LISTHEAD *pSrcHead, LISTHEAD *pDestHead );

/**
 * @brief use pDestHead replace pSrcHead of the list,keep the old last item last of the src list
 *
 * @param pSrcHead    IN - the old tail of the list
 * @param pDestHead   OUT - the new tail of the list
 */
extern void ai_list_join_tail( LISTHEAD *pSrcHead, LISTHEAD *pDestHead );

/**
 * @brief swap two head for the two lists
 * 
 * @param pHead1    IN - the head of one list
 * @param pHead2    IN - the head of the other list
 */
extern void ai_list_swap( LISTHEAD* pHead1, LISTHEAD* pHead2 );

/**
 *
 */ 
extern void ai_list_mergesort( LISTHEAD *apoListHead, pCOMPFunc CompareFunc );

/***************************************************************************
 * for (FOO* lpFoo = AI_FIRST_OF_LIST(lpFooList, FOO, coFooNode);
 *      lpFoo != NULL;
 *      lpFoo = AI_NEXT_OF_LIST(lpFoo, lpFooList, FOO, coFooNode))
 * {
 *      DO_SOMETHING_WITH(lpFoo); // no side effect to lpFooList and lpFoo->coFooNode
 * }
 ***************************************************************************/
#define AI_FIRST_OF_LIST(list, stru, member) \
    (ai_list_is_empty(list) ? NULL : AI_GET_STRUCT_PTR((list)->cpNext, stru, member))

#define AI_LAST_OF_LIST(list, stru, member) \
    (ai_list_is_empty(list) ? NULL : AI_GET_STRUCT_PTR((list)->cpPrev, stru, member))

#define AI_NEXT_OF_LIST(iter, list, stru, member) \
    (iter->member.cpNext == (list) ? NULL : AI_GET_STRUCT_PTR(iter->member.cpNext, stru, member))

#define AI_PREV_OF_LIST(iter, list, stru, member) \
    (iter->member.cpPrev == (list) ? NULL : AI_GET_STRUCT_PTR(iter->member.cpPrev, stru, member))

/***************************************************************************
 * AI_FOR_EACH_IN_LIST(lpFoo, lpFooList, FOO, coFooNode)
 * {
 *     DO_SOMETHING_WITH(lpFoo); // no side effect to lpFooList and lpFoo->coFooNode
 * }
 ***************************************************************************/
#define AI_FOR_EACH_IN_LIST(iter, list, stru, member) \
    for(stru* iter = AI_FIRST_OF_LIST(list, stru, member); \
        iter != NULL; \
        iter = AI_NEXT_OF_LIST(iter, list, stru, member)) 

#define AI_FOR_EACH_IN_LIST_R(iter, list, stru, member) \
    for(stru* iter = AI_LAST_OF_LIST(list, stru, member); \
        iter != NULL; \
        iter = AI_PREV_OF_LIST(iter, list, stru, member)) 

///end namespace
AIBC_NAMESPACE_END

#endif // __AIDUALLINK_H__

