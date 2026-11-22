" unit tests for Context "
+Test subclass: #TestContext variables: #( ) classVariables: #( )
!TestContext
testPerform
	| method |
	method <- Undefined parseMethod: 'x ^ 6 * 7'.
	self assert: ( Context new perform: method withArguments: ( Array new: 1 ) ) = 42.
	method <- Array methods at: #size.
	self assert: ( Context new perform: method withArguments: ( Array with: #( 1 2 3 ) ) ) = 3.
!
!TestContext
testFailedPrimitive
	" after a primitive fails, the method goes on with its own receiver and arguments "
	self assert: ( self afterFailedPrimitive: 5 ) == self.
	self assert: ( self argumentAfterFailedPrimitive: 5 ) = 5.
!
!TestContext
afterFailedPrimitive: aNumber
	" primitive 180 (Primitives) fails with one argument instead of two "
	<180 aNumber>.
	^ self.
!
!TestContext
argumentAfterFailedPrimitive: aNumber
	<180 aNumber>.
	^ aNumber.
!
