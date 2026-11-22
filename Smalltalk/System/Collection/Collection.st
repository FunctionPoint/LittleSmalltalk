" class definition for Collection "
+Magnitude subclass: #Collection variables: #( ) classVariables: #( )
" class methods for Collection "
" instance methods for Collection "
!Collection
< aCollection
	self do: [ :element | ( aCollection includes: element )
		ifFalse: [ ^ false ] ].
	^ true.
!
!Collection
= aCollection
	^ self < aCollection and: [ aCollection < self ].
!
!Collection
anyOne
	self do: [ :element | ^ element ].
	self emptyCollection.
!
!Collection
asArray
	| newArray index |
	newArray <- Array new: self size.
	index <- 1.
	self do: [ :element | newArray at: index put: element.
		index <- index + 1 ].
	^ newArray.
!
!Collection
asList
	^ List new addAll: self.
!
!Collection
asString
	| newString index |
	newString <- String new: self size.
	index <- 1.
	self do: [ :element | newString at: index put: element.
		index <- index + 1 ].
	^ newString.
!
!Collection
at: value
	^ self at: value ifAbsent: [ self noElement ].
!
!Collection
at: value ifAbsent: exceptionBlock
	self do: [ :element | element = value ifTrue: [ ^ element ] ].
	^ exceptionBlock value.
!
!Collection
collect: transformBlock
	| newList |
	newList <- List new.
	self do: [ :element | newList addLast: ( transformBlock value: element ) ].
	^ newList.
!
!Collection
do: aBlock
	self subclassResponsibility.
!
!Collection
emptyCollection
	self error: ( self class printString + ' is empty' ).
!
!Collection
from: fromIndex to: toIndex
	| result index size base low high |
	low <- fromIndex max: 1.
	high <- toIndex min: self size.
	size <- ( high - low ) + 1.
	( size < 1 ) ifTrue: [ ^ Array new: 0 ].
	result <- Array new: size.
	base <- index <- 1.
	self do: [ :element |
		( ( index >= low ) and: [ index <= high ] ) ifTrue: [
			result at: base put: element.
			base <- base + 1.
			( base > size ) ifTrue: [ ^ result ] ].
		index <- index + 1. ].
	^ result.
!
!Collection
includes: value
	self at: value ifAbsent: [ ^ false ].
	^ true.
!
!Collection
isEmpty
	" empty if there are no elements "
	^ self size = 0.
!
!Collection
noElement
	self error: 'Element not present'.
!
!Collection
occurencesOf: anObject
	| count |
	count <- 0.
	self do: [ :element | ( element = anObject ) ifTrue: [ count <- count + 1 ] ].
	^ count.
!
!Collection
printString
	| count result |
	result <- super printString.
	( self respondsTo: #do: ) ifFalse: [ ^ result ].
	count <- 0.
	result <- result + ' ('.
	self basicDo: [ :element |
		( count = 0 ) ifFalse: [ result <- result + ' ' ].
		result <- result + element printString.
		count <- count + 1.
		( count >= 20 ) ifTrue: [ ^ result + ' ...)' ] ].
	^ result + ')'.
!
!Collection
reject: testBlock
	" select the things that do not match predicate "
	^ self select: [ :element | ( testBlock value: element ) not ].
!
!Collection
select: testBlock
	| newList |
	newList <- List new.
	self do: [ :element | ( testBlock value: element ) ifTrue: [ newList addLast: element ] ].
	^ newList.
!
!Collection
size
	| tally |
	tally <- 0.
	self do: [ :element | tally <- tally + 1 ].
	^ tally.
!
