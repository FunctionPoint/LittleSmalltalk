" unit tests for Char "
+Test subclass: #TestChar variables: #( ) classVariables: #( )
!TestChar
testValues
	self assert: $a value = 97.
	self assert: ( Char new: 65 ) = $A.
	self assert: 65 asChar = $A.
	self assert: Char newline value = 10.
	self assert: Char tab value = 9.
	self assert: $a = $a.
	self assert: $a < $b.
	self assert: $a hash = 97.
!
!TestChar
testTests
	self assert: $5 isDigit.
	self assert: $a isDigit not.
	self assert: $a isAlphabetic.
	self assert: $1 isAlphanumeric.
	self assert: $  isBlank.
	self assert: $A isUpperCase.
	self assert: $a isLowerCase.
	self assert: $a isUpperCase not.
!
!TestChar
testConversion
	self assert: $a upperCase = $A.
	self assert: $Z lowerCase = $z.
	self assert: $5 upperCase = $5.
	self assert: $a asString = 'a'.
	self assert: $a printString = '$a'.
!
