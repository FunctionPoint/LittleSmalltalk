" unit tests for Link "
+Test subclass: #TestLink variables: #( ) classVariables: #( )
!TestLink
test
	| link values |
	link <- Link value: 1.
	self assert: link value = 1.
	self assert: link next isNil.
	link addLast: 2.
	self assert: link next value = 2.
	values <- List new.
	link do: [ :value | values addLast: value ].
	self assert: values asArray = #( 1 2 ).
	self assert: ( Link value: 1 next: ( Link value: 2 ) ) next value = 2.
	link value: 5.
	self assert: link value = 5.
!
