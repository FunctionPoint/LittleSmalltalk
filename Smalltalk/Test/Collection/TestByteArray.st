" unit tests for ByteArray "
+Test subclass: #TestByteArray variables: #( ) classVariables: #( )
!TestByteArray
test
	| bytes |
	bytes <- ByteArray new: 3.
	self assert: bytes size = 3.
	self assert: ( bytes at: 1 ) = 0.
	bytes at: 2 put: 255.
	self assert: ( bytes at: 2 ) = 255.
	self assert: ( bytes isKindOf: Array ).
	self assert: ( Array methods at: #size ) byteCodes class == ByteArray.
!
