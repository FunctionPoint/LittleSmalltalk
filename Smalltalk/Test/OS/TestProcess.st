" unit tests for Process "
+Test subclass: #TestProcess variables: #( ) classVariables: #( )
!TestProcess
test
	| context process |
	context <- Context new.
	context setup: ( Undefined parseMethod: 'x ^ 6 * 7' ) withArguments: ( Array new: 1 ).
	process <- Process new.
	process context: context.
	self assert: process context == context.
	self assert: process execute = 42.
!
