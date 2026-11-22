" unit tests for Tree "
+Test subclass: #TestTree variables: #( ) classVariables: #( )
!TestTree
testAdding
	| tree |
	tree <- Tree new.
	self assert: tree isEmpty.
	tree add: 5.
	tree add: 2.
	tree add: 8.
	self assert: tree first = 2.
	self assert: tree asArray = #( 2 5 8 ).
	self assert: ( tree at: 5 ifAbsent: [ 0 ] ) = 5.
	self assert: ( tree at: 4 ifAbsent: [ 0 ] ) = 0.
	self assert: tree copy asArray = #( 2 5 8 ).
!
!TestTree
testRemoving
	| tree |
	tree <- Tree new.
	tree add: 5.
	tree add: 2.
	tree remove: 5 ifAbsent: [ nil ].
	self assert: tree asArray = #( 2 ).
	tree removeFirst.
	self assert: tree isEmpty.
!
