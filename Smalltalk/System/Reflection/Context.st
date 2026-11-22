" class definition for Context "
+Object subclass: #Context variables: #( method arguments temporaries stack bytePointer stackTop previousContext ) classVariables: #( )
" class methods for Context "
" instance methods for Context "
!Context
backtrace
	| argumentCount |
	" backtrace context calls "
	argumentCount <- 0.
	method name print.
	'(' print.
	arguments do: [ :argument |
		( argumentCount > 0 ) ifTrue: [ ', ' print ].
		argument class print.
		argumentCount <- argumentCount+1 ].
	')' printNl.
	previousContext notNil
		ifTrue: [ previousContext backtrace ].
!
!Context
perform: aMethod withArguments: argumentArray
	| process |
	self setup: aMethod withArguments: argumentArray.
	process <- Process new.
	process context: self.
	^ process execute.
!
!Context
setup: aMethod withArguments: argumentArray
	method <- aMethod.
	arguments <- argumentArray.
	bytePointer <- 0.
	stack <- Array new: method stackSize.
	stackTop <- 0.
	temporaries <- Array new: method temporarySize.
!
