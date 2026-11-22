" class definition for Dictionary "
+Collection subclass: #Dictionary variables: #( keys values ) classVariables: #( )
" class methods for Dictionary "
=Dictionary
new
	| newDictionary |
	newDictionary <- super new.
	self in: newDictionary at: 1 put: ( OrderedArray new: 0 ).
	self in: newDictionary at: 2 put: ( Array new: 0 ).
	^ newDictionary.
!
" instance methods for Dictionary "
!Dictionary
at: key
	^ self at: key ifAbsent: [ self noKey ].
!
!Dictionary
at: key ifAbsent: exceptionBlock
	| position |
	position <- keys location: key.
	( ( position <= keys size ) and: [ key = ( keys at: position ) ] )
		ifTrue: [ ^ values at: position ]
		ifFalse: [ ^ exceptionBlock value ].
!
!Dictionary
at: key put: value
	| position |
	position <- keys location: key.
	( position <= keys size and: [ key = ( keys at: position ) ] )
		ifTrue: [ values at: position put: value ]
		ifFalse: [ keys <- keys insert: key at: position.
			values <- values insert: value at: position ].
	^ value.
!
!Dictionary
binaryDo: aBlock
	1 to: keys size do:
		[ :index | aBlock value: ( keys at: index ) value: ( values at: index ) ].
!
!Dictionary
do: aBlock
	values do: aBlock.
!
!Dictionary
isEmpty
	^ keys isEmpty.
!
!Dictionary
keysAsArray
	| index result |
	result <- Array new: keys size.
	1 to: keys size do: [ :index | result at: index put: ( keys at: index ) ].
	^ result.
!
!Dictionary
keysDo: aBlock
	1 to: keys size do: [ :index | aBlock value: ( keys at: index ) ].
!
!Dictionary
noKey
	self error: 'key not found in dictionary lookup'.
!
!Dictionary
printString
	| count result |
	result <- self class printString + ' ('.
	count <- 0.
	self binaryDo: [ :key :value |
		( count = 0 ) ifFalse: [ result <- result + ', ' ].
		result <- result + ( key printString + ' -> ' + value printString ).
		count <- count + 1.
		( count >= 20 ) ifTrue: [ ^ result + ', ...)' ] ].
	^ result + ')'.
!
!Dictionary
removeKey: key
	^ self removeKey: key ifAbsent: [ self noKey ].
!
!Dictionary
removeKey: key ifAbsent: exceptionBlock
	| position |
	position <- keys location: key.
	( position <= keys size and: [ key = ( keys at: position ) ] )
		ifTrue: [ keys <- keys removeIndex: position.
			values <- values removeIndex: position ]
		ifFalse: [ ^ exceptionBlock value ].
!
