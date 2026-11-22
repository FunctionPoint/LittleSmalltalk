" unit tests for Collection "
+Test subclass: #TestCollection variables: #( ) classVariables: #( )
!TestCollection
testEnumerating
	self assert: ( #( 1 2 3 4 ) select: [ :number | ( number rem: 2 ) = 0 ] ) asArray = #( 2 4 ).
	self assert: ( #( 1 2 3 4 ) reject: [ :number | ( number rem: 2 ) = 0 ] ) asArray = #( 1 3 ).
	self assert: ( #( 1 2 3 ) collect: [ :number | number * number ] ) asArray = #( 1 4 9 ).
	self assert: ( #( 1 2 2 3 ) occurencesOf: 2 ) = 2.
	self assert: #( 4 5 ) anyOne = 4.
!
!TestCollection
testTesting
	self assert: #( ) isEmpty.
	self assert: #( 1 ) isEmpty not.
	self assert: ( #( 1 2 ) includes: 2 ).
	self assert: ( List new isKindOf: Collection ).
!
!TestCollection
testConverting
	self assert: ( #( 3 1 2 ) asList isKindOf: List ).
	self assert: #( 3 1 2 ) asList first = 3.
	self assert: ( 1 to: 3 ) asArray = #( 1 2 3 ).
	self assert: ( List with: $a ) asString = 'a'.
!
