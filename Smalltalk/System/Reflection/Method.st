" class definition for Method "
+Object subclass: #Method variables: #( name byteCodes literals stackSize temporarySize class ) classVariables: #( )
" class methods for Method "
=Method
flushCache
	<34>.
	self primitiveFailed.
!
=Method
name: methodName byteCodes: methodByteCodes literals: methodLiterals stackSize: methodStackSize temporarySize: methodTemporarySize class: methodClass
	| newMethod |
	newMethod <- self new.
	super in: newMethod at: 1 put: methodName.
	super in: newMethod at: 2 put: methodByteCodes.
	super in: newMethod at: 3 put: methodLiterals.
	super in: newMethod at: 4 put: methodStackSize.
	super in: newMethod at: 5 put: methodTemporarySize.
	super in: newMethod at: 6 put: methodClass.
	^ newMethod.
!
" instance methods for Method "
!Method
byteCodes
	^ byteCodes.
!
!Method
literals
	^ literals.
!
!Method
name
	^ name.
!
!Method
stackSize
	^ stackSize.
!
!Method
temporarySize
	^temporarySize.
!
