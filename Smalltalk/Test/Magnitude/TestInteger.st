" unit tests for Integer "
+Test subclass: #TestInteger variables: #( ) classVariables: #( )
!TestInteger
testArithmetic
	| bigNumber |
	" the bootstrap compiler has no 64-bit literals: the REPL compiler makes them "
	bigNumber <- '12345678901234' doIt.
	self assert: bigNumber class == Integer.
	self assert: ( bigNumber * 10 ) printString = '123456789012340'.
	self assert: ( bigNumber quo: 1000 ) printString = '12345678901'.
	self assert: ( bigNumber rem: 1000 ) printString = '234'.
	self assert: ( bigNumber - '12345678901233' doIt ) printString = '1'.
	self assert: ( bigNumber + '1' doIt asInteger ) printString = '12345678901235'.
!
!TestInteger
testComparing
	| bigNumber |
	bigNumber <- '12345678901234' doIt.
	self assert: bigNumber < '12345678901235' doIt.
	self assert: bigNumber = '12345678901234' doIt.
	self assert: ( bigNumber = '12345678901235' doIt ) not.
!
!TestInteger
testConversion
	self assert: 3 asInteger class == Integer.
	self assert: 3 asInteger asSmallInt = 3.
	self assert: ( 100000 * 100000 ) class == Integer.
	self assert: ( 100000 * 100000 ) printString = '10000000000'.
	self assert: 20 factorial printString = '2432902008176640000'.
!
