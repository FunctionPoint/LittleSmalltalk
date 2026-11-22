" unit tests for Node "
+Test subclass: #TestNode variables: #( ) classVariables: #( )
!TestNode
test
	| node values |
	node <- Node new: 4.
	self assert: node value = 4.
	node add: 2.
	node add: 6.
	self assert: node first = 2.
	self assert: ( node at: 6 ifAbsent: [ nil ] ) = 6.
	self assert: ( node at: 5 ifAbsent: [ 0 ] ) = 0.
	values <- List new.
	node do: [ :value | values addLast: value ].
	self assert: values asArray = #( 2 4 6 ).
!
