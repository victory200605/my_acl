
#ifndef __OBJECT_TYPE_H__
#define __OBJECT_TYPE_H__

#include <string>
#include <vector>

class clsObjectType
{
public:
	enum { SIMPLE = 1, TABLE };

public:
	clsObjectType() :ciType(0)  {}

	clsObjectType( const char* apcName, const char* apcOid, const char* apcTypeName, int aiType = SIMPLE ):
		coStrName(apcName),
		coStrOid(apcOid),
		coStrTypeName(apcTypeName),
		ciType(aiType)
	{
	}
	
	void SetName( const char* apcName )
	{
		coStrName = apcName;
	}
	void SetOid( const char* apcOid )
	{
		coStrOid = apcOid;
	}
	void SetTypeName( const char* apcTypeName )
	{
		coStrTypeName = apcTypeName;
	}
	void SetType( int aiType )
	{
		ciType = aiType;
	}

	const char* GetName()      { return coStrName.c_str(); }
	const char* GetOid()       { return coStrOid.c_str(); }
	const char* GetTypeName()  { return coStrTypeName.c_str(); }
	int         GetType()      { return ciType; }

protected:
	std::string coStrName;
	std::string coStrOid;
	std::string coStrTypeName;
	
	int         ciType;
};

class clsTable : public clsObjectType
{
public:
	typedef std::vector<clsObjectType*>::iterator iterator;

public:
	clsTable( const char* apcName, const char* apcOid ) :
		clsObjectType( apcName, apcOid, "table", clsObjectType::TABLE )
	{
	}
	~clsTable()
	{
		for ( iterator liIt = copColumns.begin(); liIt != copColumns.end(); liIt++ )
		{
			delete *liIt;
		}
	}

	void AddColumn( clsObjectType* apoColumn )
	{
		copColumns.push_back( apoColumn );
	}

	iterator Begin() { return copColumns.begin(); }
	iterator End()   { return copColumns.end(); }

protected:
	std::vector<clsObjectType*> copColumns;
};

typedef std::vector<clsObjectType*> TObjectList;
typedef std::vector<clsTable*>      TTableList;

#endif //__OBJECT_TYPE_H__
