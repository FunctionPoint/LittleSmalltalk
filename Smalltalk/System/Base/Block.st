" class definition for Block "
+Context subclass: #Block variables: #( argumentLocation creatingContext oldBytePointer ) classVariables: #( )
" class methods for Block "
" instance methods for Block "
!Block
argCount
	self error: 'Incorrect argument passing to Block'.
!
!Block
backtrace
	| argumentCount |
	" backtrace context calls "
	'block from ' print. method name print.
	'(' print.
	argumentCount <- 0.
	arguments do: [ :argument |
		( argumentCount > 0 ) ifTrue: [ ', ' print ].
		argument class print.
		argumentCount <- argumentCount+1 ].
	')' printNl.
	previousContext notNil
		ifTrue: [ previousContext backtrace ].
!
!Block
value
	" start block execution "
	<8 self>
	( self argCount ).
!
!Block
value: argument
	" start block execution "
	<8 argument self>
	( self argCount ).
!
!Block
value: firstArgument value: secondArgument
	" start block execution "
	<8 firstArgument secondArgument self>
	( self argCount ).
!
!Block
whileFalse: aBlock
	self value ifFalse: [ aBlock value. ^ self whileFalse: aBlock ].
!
!Block
whileTrue: aBlock
	self value ifTrue: [ aBlock value. ^ self whileTrue: aBlock ].
!
