" class definition for StringBuffer "
+List subclass: #StringBuffer variables: #( ) classVariables: #( )
" class methods for StringBuffer "
" instance methods for StringBuffer "
!StringBuffer
add: anObject
	^ super add: ( anObject printString ).
!
!StringBuffer
addLast: anObject
	^ super addLast: ( anObject printString ).
!
!StringBuffer
asString
	| size index result |
	size <- self size.

	result <- String new: size.

	index <- 1.

	self do: [ :entry | ( entry asString ) do: [ :char | result at: index put: char. index <- index + 1 ] ].

	^ result.
!
!StringBuffer
printString
	^ ( self asString ).
!
!StringBuffer
size
	| total |
	total <- 0.
	self do: [ :entry | total <- total + ( entry size ) ].

	^ total.
!
!StringBuffer
write: anObject
	self addLast: anObject.

	^ self.
!
