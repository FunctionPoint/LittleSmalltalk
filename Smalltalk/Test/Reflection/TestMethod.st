" unit tests for Method "
+Test subclass: #TestMethod variables: #( ) classVariables: #( )
!TestMethod
test
	| method |
	method <- Array methods at: #size.
	self assert: method class == Method.
	self assert: method name == #size.
	self assert: method byteCodes class == ByteArray.
	self assert: method byteCodes size > 0.
	self assert: method stackSize > 0.
	method <- Object parseMethod: 'foo ^ #( 1 ) , #bar'.
	self assert: method literals size = 3.
	self assert: ( method literals at: 1 ) = #( 1 ).
	self assert: ( method literals at: 2 ) == #bar.
	self assert: method temporarySize = 0.
!
