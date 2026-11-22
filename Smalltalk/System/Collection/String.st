" class definition for String "
+Array subclass: #String variables: #( ) classVariables: #( )
" class methods for String "
=String
input
	| value character newline |
	" read a line from input "
	value <- ''. newline <- Char newline.
	[ character <- Char input.
		character isNil ifTrue: [ ^ nil ]. character ~= newline ] whileTrue:
		[ value <- value + character asString ].
	^ value.
!
=String
new: size
	<20 self size>.
!
" instance methods for String "
!String
asNumber
	| value |
	" parse a base-10 ASCII number, return nil on failure "
	value <- 0.
	self do: [ :character |
		character isDigit ifFalse: [ ^nil ].
		value <- ( value * 10 ) + ( character value - 48 ) ].
	^value.
!
!String
asSymbol
	^ Symbol new: self.
!
!String
at: index
	^self at: index ifAbsent: [ self badIndex ].
!
!String
at: index ifAbsent: exceptionBlock
	| code |
	code <- self basicAt: index.
	( code isNil )
		ifTrue: [ ^ exceptionBlock value ]
		ifFalse: [ ^ Char new: code ].
!
!String
at: index put: aValue
	( self basicAt: index put: aValue value ) isNil ifTrue: [
		self badIndex ].
!
!String
basicAt: index
	<21 self index>
	^nil.
!
!String
basicAt: index put: value
	<22 value self index>
	^nil.
!
!String
break: separators
	| words word |
	" break string into words, using separators "
	word <- ''.
	words <- List new.
	self do: [ :character |
		( separators includes: character )
			ifTrue: [
				( word size > 0 ) " found a word "
					ifTrue: [ words addLast: word.
							word <- '' ] ]
			ifFalse: [ word <- word + character asString ] ].
	" maybe a last word "
	( word size > 0 ) ifTrue: [ words addLast: word ].
	^ words.
!
!String
collect: transformationBlock
	^ ( super collect: transformationBlock ) asString.
!
!String
copy
	" make a clone of ourself "
	<23 self String>.
!
!String
doIt
	| method |
	method <- Undefined parseMethod: 'doItCommand ^' + self.
	^ method notNil
		ifTrue: [ ^ Context new
				perform: method withArguments: ( Array new: 1 ) ].
!
!String
hash
	| size |
	size <- self size.
	( size < 2 ) ifTrue: [
		( size = 1 ) ifTrue: [ ^ ( self at: 1 ) value ].
		^ 0 ].
	^ ( self at: 1 ) value + ( self at: size ) value.
!
!String
printString
	^ self.
!
!String
printWidth: width
	| result |
	( self size >= width absolute ) ifTrue: [ ^ self ].
	result <- self.
	( width negative ) ifTrue: [
			( self size + 1 ) to: ( width negated ) do:
				[ :ignore | result <- ' ' + result ]. ]
		ifFalse: [
			( self size + 1 ) to: width do:
				[ :ignore | result <- result + ' ' ]. ].
	^result.
!
!String
reverse
	^ self asList reverse asString.
!
!String
select: testBlock
	^ ( super select: testBlock ) asString.
!
