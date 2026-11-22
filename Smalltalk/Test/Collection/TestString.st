" unit tests for String "
+Test subclass: #TestString variables: #( ) classVariables: #( )
!TestString
testAccess
	| string |
	self assert: 'hello' size = 5.
	self assert: ( 'abc' at: 2 ) = $b.
	string <- 'abc' copy.
	string at: 1 put: $z.
	self assert: string = 'zbc'.
	self assert: 'it''s' size = 4.
	self assert: ( String new: 3 ) size = 3.
	self assert: '' isEmpty.
!
!TestString
testComparing
	self assert: 'abc' = 'abc'.
	self assert: ( 'abc' = 'abd' ) not.
	self assert: 'abc' < 'abd'.
	self assert: 'abc' hash = 'abc' copy hash.
!
!TestString
testCopying
	self assert: 'abc' + 'def' = 'abcdef'.
	self assert: 'abc' reverse = 'cba'.
	self assert: ( 'abc' collect: [ :character | character upperCase ] ) = 'ABC'.
	self assert: ( 'a1b2' select: [ :character | character isDigit ] ) = '12'.
	self assert: ( 'abc' printWidth: 5 ) = 'abc  '.
	self assert: ( 'hello' from: 2 to: 3 ) = 'el'.
!
!TestString
testConversion
	self assert: 'abc' asSymbol == #abc.
	self assert: '42' asNumber = 42.
	self assert: '4x' asNumber isNil.
	self assert: 'abc' printString = 'abc'.
	self assert: ( 'hello world' break: ' ' ) size = 2.
	self assert: ( 'a,b' break: ',' ) first = 'a'.
	self assert: '3 + 4' doIt = 7.
!
