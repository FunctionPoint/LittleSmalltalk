" class definition for Interval "
+Collection subclass: #Interval variables: #( low high step ) classVariables: #( )
" class methods for Interval "
=Interval
from: lowBound to: highBound step: stepSize
	| newInterval |
	newInterval <- self in: self new at: 1 put: lowBound.
	self in: newInterval at: 2 put: highBound.
	self in: newInterval at: 3 put: stepSize.
	^ newInterval.
!
" instance methods for Interval "
!Interval
atRandom
	| randomIndex |
	" Return a random element from our sequence "
	randomIndex <- ( SmallInt atRandom ) rem: ( ( high - low + 1 ) quo: step ).
	^ low + ( randomIndex * step ).
!
!Interval
do: aBlock
	| current |
	current <- low.
	( step < 0 )
		ifTrue: [
			[ current >= high ] whileTrue:
				[ aBlock value: current.
				current <- current + step ] ]
		ifFalse: [
			[ current <= high ] whileTrue:
				[ aBlock value: current.
				current <- current + step ] ].
!
!Interval
high
	^ high.
!
!Interval
high: newHigh
	high <- newHigh.
!
!Interval
includes: aNumber
	" Check within range first "
	( ( aNumber < low ) or: [ aNumber > high ] ) ifTrue: [ ^ false ].
	" Then check if in equivalence class of interval "
	^ ( ( aNumber - low ) rem: step ) = 0.
!
!Interval
low
	^ low.
!
!Interval
low: newLow
	low <- newLow.
!
!Interval
printString
	| string |
	string <- ( self class printString ) + ' <' +
		low printString + '..' + high printString.
	( step ~= 1 ) ifTrue: [ string <- string + ' by ' + step printString ].
	^ string + '>'.
!
