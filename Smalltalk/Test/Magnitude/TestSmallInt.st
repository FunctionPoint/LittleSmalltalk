" unit tests for SmallInt "
+Test subclass: #TestSmallInt variables: #( ) classVariables: #( )
!TestSmallInt
testArithmetic
	self assert: 3 + 4 = 7.
	self assert: 3 - 4 = -1.
	self assert: 6 * 7 = 42.
	self assert: ( 7 quo: 2 ) = 3.
	self assert: ( 7 rem: 2 ) = 1.
	self assert: ( -7 quo: 2 ) = -3.
	self assert: ( -7 rem: 2 ) = -1.
!
!TestSmallInt
testComparing
	self assert: 3 < 4.
	self assert: ( 4 < 3 ) not.
	self assert: 3 = 3.
	self assert: 3 class == SmallInt.
	self assert: 3 hash = 3.
!
