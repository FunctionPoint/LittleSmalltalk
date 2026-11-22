" unit tests for Undefined "
+Test subclass: #TestUndefined variables: #( ) classVariables: #( )
!TestUndefined
test
	self assert: nil isNil.
	self assert: nil notNil not.
	self assert: nil printString = 'nil'.
	self assert: Undefined new isNil.
	self assert: nil class == Undefined.
!
