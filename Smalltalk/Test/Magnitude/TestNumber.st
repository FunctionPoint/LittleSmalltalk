" unit tests for Number "
+Test subclass: #TestNumber variables: #( ) classVariables: #( )
!TestNumber
testArithmetic
	self assert: 5 factorial = 120.
	self assert: 3 negated = -3.
	self assert: -3 absolute = 3.
	self assert: -3 negative.
	self assert: 3 negative not.
	self assert: ( 10 atRandom between: 1 and: 10 ).
!
!TestNumber
testBits
	self assert: ( 12 bitAnd: 10 ) = 8.
	self assert: ( 12 bitOr: 3 ) = 15.
	self assert: ( 1 bitShift: 4 ) = 16.
	self assert: ( 16 bitShift: -2 ) = 4.
!
!TestNumber
testConversion
	self assert: 65 asChar = $A.
	self assert: 7 asDigit = '7'.
	self assert: 11 asDigit = 'B'.
	self assert: 3 printString = '3'.
	self assert: -5 printString = '-5'.
	self assert: ( 255 printWidth: 5 ) = '00255'.
	self assert: ( 255 printWidth: 1 base: 16 ) = 'FF'.
	self assert: ( 10 printWidth: 1 base: 2 ) = '1010'.
!
!TestNumber
testLoops
	| sum |
	sum <- 0.
	1 to: 4 do: [ :number | sum <- sum + number ].
	self assert: sum = 10.
	sum <- 0.
	1 to: 10 by: 3 do: [ :number | sum <- sum + number ].
	self assert: sum = 22.
	self assert: ( 1 to: 3 ) class == Interval.
!
