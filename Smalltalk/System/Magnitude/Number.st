" class definition for Number "
+Magnitude subclass: #Number variables: #( ) classVariables: #( )
" class methods for Number "
=Number
new
	" can't create this way, return zero "
	^ 0.
!
" instance methods for Number "
!Number
absolute
	( self negative ) ifTrue: [ ^ self negated ].
!
!Number
asChar
	^ Char new: ( self asSmallInt ).
!
!Number
asDigit
	( self < 10 ) ifTrue:
		[ ^( Char new: ( self asSmallInt + 48 ) ) asString ].
	^( Char new: ( self asSmallInt + 55 ) ) asString.
!
!Number
atRandom
	" Return random number from 1 to self "
	( self < 2 ) ifTrue: [ ^ self ].
	^ ( ( 1 to: self ) atRandom ).
!
!Number
bitAnd: aNumber
	^ ( self asSmallInt bitAnd: aNumber ).
!
!Number
bitOr: aNumber
	^ ( self asSmallInt bitOr: aNumber ).
!
!Number
bitShift: aNumber
	^ ( self asSmallInt bitShift: aNumber ).
!
!Number
factorial
	self <= 1 ifTrue: [ ^ 1 ]
	ifFalse: [ ^ ( self - 1 ) factorial * self ].
!
!Number
negated
	^0-self.
!
!Number
negative
	^self < 0.
!
!Number
overflow
	self error: 'Numeric overflow'.
!
!Number
printString
	^self printWidth: 1 base: 10.
!
!Number
printWidth: width
	^self printWidth: width base: 10.
!
!Number
printWidth: width base: base
	| result remaining digit wasNegative digitWidth |
	result <- ''.
	( self negative ) ifTrue: [
		wasNegative <- true.
		digitWidth <- width-1.
		remaining <- self negated ] ifFalse: [
		wasNegative <- false.
		digitWidth <- width.
		remaining <- self ].
	[ true ] whileTrue: [
		result <- ( ( remaining rem: base ) asDigit ) + result.
		remaining <- remaining quo: base.
		( remaining = 0 ) ifTrue: [
			( ( result size )+1 ) to: digitWidth do: [ :ignore |
				result <- '0' + result ].
			wasNegative ifTrue: [ result <- '-' + result ].
			^result ] ].
!
!Number
to: limit
	^ Interval from: self to: limit step: 1.
!
!Number
to: limit by: step
	^ Interval from: self to: limit step: step.
!
!Number
to: limit by: step do: aBlock
	| index |
	index <- self.
	[ index <= limit ] whileTrue: [ aBlock value: index. index <- index + step ].
!
!Number
to: limit do: aBlock
	| index |
	" optimize arithmetic loops "
	index <- self.
	[ index <= limit ] whileTrue: [ aBlock value: index. index <- index + 1 ].
!
