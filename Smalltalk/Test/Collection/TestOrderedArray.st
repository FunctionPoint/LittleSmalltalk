" unit tests for OrderedArray "
+Test subclass: #TestOrderedArray variables: #( ) classVariables: #( )
!TestOrderedArray
test
	| array |
	array <- OrderedArray new.
	array <- array add: 3.
	array <- array add: 1.
	array <- array add: 2.
	self assert: array asArray = #( 1 2 3 ).
	self assert: ( array includes: 2 ).
	self assert: ( array includes: 4 ) not.
	self assert: ( array isKindOf: Array ).
!
