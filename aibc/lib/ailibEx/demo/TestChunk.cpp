#include "TestMacro.h"
#include "AIChunkEx.h"

int main()
{
	AIChunkEx loChunk0(10);
	AIChunkEx loChunk1( "0123456789", 11 );
    
	TEST_CASE( "base operator") {
		ASSERT_EQ( loChunk0.GetSize(), 10 );
		ASSERT_EQ( loChunk0.GetCapacity(), 10 );
		ASSERT_EQ_STR( loChunk0.BasePtr(), "" );
		ASSERT_EQ( loChunk0.WritePtr() - loChunk0.BasePtr(), 0 );
		ASSERT_EQ( loChunk0.ReadPtr() - loChunk0.BasePtr(), 0 );
		ASSERT_EQ( loChunk1.GetSize(), 11 );
		ASSERT_EQ( loChunk1.GetCapacity(), 11 );
		ASSERT_EQ_STR( loChunk1.BasePtr(), "0123456789" );
		ASSERT_EQ( loChunk1.WritePtr() - loChunk1.BasePtr(), 11 );
		ASSERT_EQ( loChunk1.ReadPtr() - loChunk1.BasePtr(), 0 );
	};

	TEST_CASE( "Resize" ) {
		loChunk1.Resize(100);
		ASSERT_EQ( loChunk1.GetSize(), 100 );
		ASSERT_EQ( loChunk1.GetCapacity(), 100 );
		ASSERT_EQ_STR( loChunk1.BasePtr(), "0123456789" );
		ASSERT_EQ( loChunk1.WritePtr() - loChunk1.BasePtr(), 11 );
		ASSERT_EQ( loChunk1.ReadPtr() - loChunk1.BasePtr(), 0 );
		loChunk1.Resize(5);
		ASSERT_EQ( loChunk1.GetSize(), 5 );
		ASSERT_EQ( loChunk1.GetCapacity(), 100 );
		ASSERT_EQ_STR( loChunk1.BasePtr(), "0123456789" );
		ASSERT_EQ( loChunk1.WritePtr() - loChunk1.BasePtr(), 11 );
		ASSERT_EQ( loChunk1.ReadPtr() - loChunk1.BasePtr(), 0 );
		loChunk1.Resize(100);
		memset( loChunk1.WritePtr() - 1, 'A', 10 );
		loChunk1.WritePtr(9);
		ASSERT_EQ_STR( loChunk1.BasePtr(), "0123456789AAAAAAAAAA" );
		ASSERT_EQ( loChunk1.WritePtr() - loChunk1.BasePtr(), 20 );
	}
}
