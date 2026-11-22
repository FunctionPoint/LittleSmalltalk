" unit tests for Magnitude "
+Test subclass: #TestMagnitude variables: #( ) classVariables: #( )
!TestMagnitude
test
	self assert: 3 <= 3.
	self assert: 4 > 3.
	self assert: 3 >= 3.
	self assert: ( 3 >= 4 ) not.
	self assert: ( 3 between: 1 and: 5 ).
	self assert: ( 6 between: 1 and: 5 ) not.
	self assert: ( 3 max: 5 ) = 5.
	self assert: ( 3 min: 5 ) = 3.
	self assert: ( $c between: $a and: $z ).
	self assert: ( $a max: $b ) = $b.
!
