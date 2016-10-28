//=============================================================================
/**
 * \file    ModuleMgr.h
 * \brief 
 *
 * Copyright (c) 2007 Asiainfo Technologies(China),Inc.
 * RCS: $Id: ModuleMgr.h,v 1.2 2010/12/20 08:19:04 daizh Exp $
 *
 * History
 * 2008.04.04 first release by DZH
 */
//=============================================================================
#ifndef AI_GFQ_MODULEMGR_2008
#define AI_GFQ_MODULEMGR_2008

#include "gfq/GFQUtility.h"

// Start namespace
AI_GFQ_NAMESPACE_START

// Module head, for module name hash array
struct stModuleHead
{
	stModuleHead( const char* apsModuleName, int32_t aiModuleID, const char* apsIpAddr, int32_t aiPort );
	~stModuleHead();
	void Unlink();
	
	char ccIpAddr[AI_IP_ADDR_LEN]; // module ip address
	int32_t ciPort; // module port
	clsAsyncClient* cpoAsyncClient; // point to asyncserver object
	int32_t ciLastHeartBeatTime;

	char ccModuleName[AI_MAX_NAME_LEN]; // module name, only one
	int32_t ciModuleID; // module ID, dynamic distributed by master
	int32_t ciUsingSegmentCnt;
	int32_t ciFreeSegmentCnt; // free segment number
	
	int32_t ciErrorCnt;

	LISTHEAD coModuleChild; // for module head hash link
	LISTHEAD coFreeMoudleChild; // for free module link
	LISTHEAD coMoudleListChild; // for all module link
	LISTHEAD coUsingSegHead; // using segment link
	LISTHEAD coFreeSegHead; // free segment link
};

// Module name hash link head
struct stModuleNameHash
{
    stModuleNameHash();
    
    LISTHEAD coModuleHead;
};

// Module handle define
typedef stModuleHead* TModuleHandle;
typedef stModuleHead& TModuleRef;
#define INVALID_MODULE_HANDLE NULL

// Access mdoule head single iterator
// This iterator can't random assert
class clsModuleIter
{
public:
	// Construct funcion
	clsModuleIter( LISTHEAD* apoNode, LISTHEAD* apoHead ) : cpoNode(apoNode), cpoHead(apoHead)
	{
		assert( apoNode != NULL && apoHead != NULL );
	}
	
	// ++ Operator overwrite
	void operator ++ ( int )
	{
		if ( cpoNode != NULL && cpoNode != cpoHead )
		{
			cpoNode = cpoNode->cpNext;
		}
	}
	
	// == Operator overwrite
	bool operator == ( const clsModuleIter& apoIter )
	{
		return cpoNode == apoIter.cpoNode && cpoHead == apoIter.cpoHead;
	}
	
	// -> Operator overwrite
	TModuleHandle operator -> ()
	{
		return &(**this);
	}
	
	// * Operator overwrite
	TModuleRef operator * ()
	{
		stModuleHead* lpoModuleHead = AI_GET_STRUCT_PTR( cpoNode , stModuleHead, coMoudleListChild );
		return *lpoModuleHead;
	}
	
	// TModuleHandle Type convert
	operator TModuleHandle ()
	{
		return &(**this);
	}
protected:
	LISTHEAD* cpoNode;
	LISTHEAD* cpoHead;
};

// sgement module, do for segment allocate and recycle
template< class TSegment >
class clsModuleMgr
{
public:
	typedef clsModuleIter TIterator;
	
public:
	// segment node, element of module or queue
	struct stSegmentNode : public TSegment
	{
		stSegmentNode();
		
		void UnlinkModule();
		
		stModuleHead* cpoModuleHead;
		LISTHEAD coSegmentChild;
	};

public:
    enum { SG_RETRY = 0, SG_SWITCH, SG_RETURN }; // Define for segment error
    
public:
//	static clsModuleMgr* Instance();
	virtual ~clsModuleMgr();
    
    // Set error Limit value, if error count out of it, and the module will be delete
    void SetErrorLimit( int32_t aiErrorLimit )           { ciErrorLimit = aiErrorLimit; }
    
    // Add module
    int32_t AddModule( const char* apsModuleName, const char* apsIpAddr, int32_t aiPort );
    
    // Delete module by module name
    int32_t DelModule( const char* apsModuleName );
    
    // Delete module by module handle
    int32_t DelModule( TModuleHandle atModuleHandle );
    
    // Search module by module name
    TModuleHandle SearchModule( const char* apsModuleName );
    
    // Create segment in module, all segment node create must use this function
    TSegment* CreateSegment( TModuleHandle atModuleHandle );
    
    // Destory segment, all segment node delete must use this function
    void DestorySegment( TSegment*& apoSegment );
	
	// Put free segment to module, for load segment to module
	int32_t PutFreeSegment( TSegment* apoSegment );
	
	// Put suing segment to module, for load segment to module
	int32_t PutUsingSegment( TSegment* apoSegment );
	
	// Allocate segment
	TSegment* AllocSegment();
	int32_t FreeSegment( TSegment* apoSegment );
	
	// Start module get operator
	// Get module handle by segment pointer
	TModuleHandle GetModuleHandle( const TSegment* apoSegment );
	// Get module segment server ip address by segment pointer
	const char* GetModuleIpAddr( const TSegment* apoSegment );
	// Get module id by segment pointer
	int32_t GetModuleID( const TSegment* apoSegment );
	// Get module segment server port
	int32_t GetModulePort( const TSegment* apoSegment );
	
	size_t GetModuleCount();
	size_t GetSegmentCount( TModuleHandle atModuleHandle );
	size_t GetUsingSegmentCount( TModuleHandle atModuleHandle );
	size_t GetFreeSegmentCount( TModuleHandle atModuleHandle );
	clsAsyncClient* GetModuleServer( TModuleHandle atModuleHandle );
	void UpdateHeartBeatTime( TModuleHandle atModuleHandle );
	int32_t GetLastHeartBeatTime( TModuleHandle atModuleHandle );
	// End module get operator
	
	clsModuleIter ModuleBegin();
	clsModuleIter ModuleEnd();

	// Error observer, module error process function
	int32_t ApiErrorObserver( TModuleHandle atModuleHandle, int32_t aiErrno );
	void MasterErrorObserver( TModuleHandle atModuleHandle, int32_t aiErrno );
	void MasterFreeErrorObserver( TModuleHandle atModuleHandle, int32_t aiErrno );
	
	void Close();

//protected:
	clsModuleMgr();

protected:
    stModuleNameHash* cpoModuleHash;
    
    LISTHEAD coFreeModule;
    LISTHEAD coModuleList;
    size_t ciMuduleNum;
    
    int32_t ciModuleID;
    
    int32_t ciErrorLimit;
    
    static clsModuleMgr* cpoInstance;
};

#include "ModuleMgr.cpp"

//end namespace
AI_GFQ_NAMESPACE_END

#endif // AI_AFQ_MODULE_2008
