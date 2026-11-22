" unit tests for Interval "
+Test subclass: #TestInterval variables: #( ) classVariables: #( )
!TestInterval
test
	| sum |
	self assert: ( 1 to: 5 ) low = 1.
	self assert: ( 1 to: 5 ) high = 5.
	self assert: ( ( 1 to: 5 ) includes: 3 ).
	self assert: ( ( 1 to: 5 ) includes: 6 ) not.
	self assert: ( 1 to: 0 ) isEmpty.
	self assert: ( 1 to: 10 by: 3 ) asArray = #( 1 4 7 10 ).
	self assert: ( ( 1 to: 4 ) collect: [ :number | number * 2 ] ) asArray = #( 2 4 6 8 ).
	sum <- 0.
	( 1 to: 4 ) do: [ :number | sum <- sum + number ].
	self assert: sum = 10.
	self assert: ( ( 1 to: 10 ) atRandom between: 1 and: 10 ).
	self assert: ( 1 to: 5 ) printString = 'Interval <1..5>'.
!
