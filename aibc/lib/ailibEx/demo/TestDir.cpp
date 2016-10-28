#include "TestMacro.h"
#include "AIDir.h"

int main()
{
	AIDir loDir;
	
	TEST_CASE( "base operator" )
	{
		ASSERT_EQ( AIDir::Make( "./test_dir", 0755 ), 0 );
		ASSERT_EQ( AIDir::Make( "./test_dir/test_sub_dir1", 0755 ), 0 );
		ASSERT_EQ( AIDir::Make( "./test_dir/test_sub_dir2", 0755 ), 0 );
		ASSERT_EQ( AIDir::Make( "./test_dir/test_sub_dir3", 0755 ), 0 );

		ASSERT_EQ( loDir.Open( "./test_dir" ), 0 );
		int liRetCode = 0;
		int liCounter = 0;
		while( 1 )
		{
			liRetCode = loDir.ReadNext();
			ASSERT_EXP( liRetCode == AIDir::AI_WARN_END_OF_DIR || liRetCode == AIDir::AI_NO_ERROR );
			BREAK_EXP( liRetCode == AIDir::AI_WARN_END_OF_DIR );
			liRetCode = loDir.IsDots();
			ASSERT_EXP( liRetCode == true || liRetCode == false );
			CONTINUE_EXP( liRetCode == true );
			liCounter++;
		}
		ASSERT_EQ( liCounter, 3 );
	}
}
