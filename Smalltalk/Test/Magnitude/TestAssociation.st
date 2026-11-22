" unit tests for Association "
+Test subclass: #TestAssociation variables: #( ) classVariables: #( )
!TestAssociation
test
	| association |
	association <- Association key: #k value: 1.
	self assert: association key = #k.
	self assert: association value = 1.
	association value: 5.
	self assert: association value = 5.
	self assert: ( Association key: 1 value: 2 ) = ( Association key: 1 value: 3 ).
	self assert: ( Association key: 1 value: 2 ) < ( Association key: 2 value: 0 ).
	self assert: ( Association key: 1 value: 2 ) printString = '(1 -> 2)'.
!
