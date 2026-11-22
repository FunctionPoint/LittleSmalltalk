" unit tests for StringBuffer "
+Test subclass: #TestStringBuffer variables: #( ) classVariables: #( )
!TestStringBuffer
test
	| buffer |
	buffer <- StringBuffer new.
	buffer addLast: 'ab'.
	buffer addLast: 'cd'.
	self assert: buffer asString = 'abcd'.
	self assert: buffer size = 4.
	buffer <- StringBuffer new.
	buffer add: 3.
	self assert: buffer asString = '3'.
!
