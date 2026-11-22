" class definition for Symbol "
+Magnitude subclass: #Symbol variables: #( ) classVariables: #( symbols )
" class methods for Symbol "
=Symbol
intern: string
	<23 string Symbol>.
!
=Symbol
new: fromString
	| symbol |
	^ symbols at: fromString
		ifAbsent: [ symbols add: ( self intern: fromString ) ].
!
" instance methods for Symbol "
!Symbol
< aString
	| order |
	" works with either symbol or string arguments "
	order <- self compare: aString.
	order isNil ifTrue: [ ^ self printString < aString printString ].
	^ order < 0.
!
!Symbol
= aString
	| order |
	" works with either symbol or string arguments "
	order <- self compare: aString.
	order isNil ifTrue: [ ^ self printString = aString printString ].
	^ order = 0.
!
!Symbol
compare: aString
	" -1, 0 or 1: the bytes of self compared with those of aString, a Symbol or String "
	" (primitive 181, the order the VM sorts symbols in); nil for anything else "
	<181 self aString>.
	^ nil.
!
!Symbol
asString
	^self printString.
!
!Symbol
asSymbol
	^self.
!
!Symbol
hash
	^self printString hash.
!
!Symbol
printString
	<23 self String>.
!
