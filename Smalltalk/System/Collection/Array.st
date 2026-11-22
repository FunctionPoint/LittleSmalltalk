" class definition for Array "
+Collection subclass: #Array variables: #( ) classVariables: #( )
" class methods for Array "
=Array
new
	^ self new: 0.
!
=Array
new: elementCount
	<7 self elementCount>.
!
=Array
with: firstElement
	^ self in: ( self new: 1 ) at: 1 put: firstElement.
!
=Array
with: firstElement with: secondElement
	| result |
	result <- self new: 2.
	self in: result at: 1 put: firstElement.
	self in: result at: 2 put: secondElement.
	^ result.
!
=Array
with: firstElement with: secondElement with: thirdElement
	| result |
	result <- self new: 3.
	self in: result at: 1 put: firstElement.
	self in: result at: 2 put: secondElement.
	self in: result at: 3 put: thirdElement.
	^ result.
!
" instance methods for Array "
!Array
+ aValue
	| ownSize otherSize newValue |
	" catenate two strings together "
	ownSize <- self size.
	otherSize <- aValue size.
	newValue <- self class new: ( ownSize + otherSize ).
	newValue replaceFrom: 1 to: ownSize with: self.
	newValue replaceFrom: ownSize+1 to: ownSize+otherSize with: aValue.
	^ newValue.
!
!Array
< anArray
	| ownSize otherSize |
	ownSize <- self size. otherSize <- anArray size.
	1 to: ( ownSize min: otherSize )
		do: [ :index | ( self at: index ) ~= ( anArray at: index )
			ifTrue: [ ^ ( self at: index ) < ( anArray at: index ) ] ].
	^ ownSize < otherSize.
!
!Array
= anArray
	self size = anArray size ifFalse: [ ^ false ].
	1 to: self size do:
		[ :index | ( self at: index ) = ( anArray at: index )
			ifFalse: [ ^ false ] ].
	^ true.
!
!Array
at: index
	<24 self index>
	( self includesKey: index ) ifFalse: [ self badIndex ].
	self primitiveFailed.
!
!Array
at: index ifAbsent: exceptionBlock
	<24 self index>.
	^ exceptionBlock value.
!
!Array
at: index put: value
	<5 value self index>
	( self includesKey: index ) ifFalse: [ self badIndex ].
	self primitiveFailed.
!
!Array
badIndex
	self error: 'array indexing error'.
!
!Array
copy
	^ self asArray.
!
!Array
do: aBlock
	1 to: self size do: [ :index | aBlock value: ( self at: index ) ].
!
!Array
elementsExchangeIdentityWith: otherArray
	<35 self otherArray>.
	self primitiveFailed.
!
!Array
first
	^self at: 1.
!
!Array
from: low to: high
	| start stop size result |
	start <- low max: 0.
	stop <- high min: self size.
	size <- ( stop + 1 - start ) max: 0.
	result <- ( self species ) new: size.
	1 to: size do: [ :index |
		result at: index put: ( self at: start ).
		start <- start + 1 ].
	^ result.
!
!Array
hash
	| size |
	size <- self size.
	( size < 2 ) ifTrue: [
		( size = 1 ) ifTrue: [ ^ ( self at: 1 ) hash + size ].
		^ 0 ].
	^ ( self at: 1 ) hash + ( self at: size ) hash.
!
!Array
includes: aValue
	self do: [ :element | element = aValue ifTrue: [ ^ true ] ].
	^ false.
!
!Array
includesKey: index
	^ index between: 1 and: self size.
!
!Array
indexOf: aValue
	1 to: self size do: [ :index |
		( ( self at: index ) == aValue ) ifTrue: [ ^ index ] ].
	^ nil.
!
!Array
indexOfVal: aValue
	1 to: self size do: [ :index |
		( ( self at: index ) = aValue ) ifTrue: [ ^ index ] ].
	^ nil.
!
!Array
insert: value at: position
	| newArray newSize |
	newSize <- self size + 1.
	newArray <- self class new: newSize.
	newArray replaceFrom: 1 to: position-1 with: self.
	newArray at: position put: value.
	newArray replaceFrom: position+1 to: newSize with:
		self startingAt: position.
	^ newArray.
!
!Array
removeIndex: position
	| newArray newSize |
	newSize <- self size - 1.
	newArray <- self class new: newSize.
	newArray replaceFrom: 1 to: position-1 with: self.
	newArray replaceFrom: position to: newSize with: self
		startingAt: position+1.
	^ newArray.
!
!Array
replaceFrom: start to: stop with: replacement
	^ self replaceFrom: start to: stop with: replacement startingAt: 1.
!
!Array
replaceFrom: start to: stop with: replacement startingAt: replacementStart
	| base |
	<38 start stop replacement replacementStart self>.
	base <- replacementStart-1.
	0 to: ( stop - start ) do: [ :offset |
		self at: ( offset + start ) put:
			( replacement at: ( offset + replacementStart ) ) ].
!
!Array
size
	" compute number of elements "
	<4 self>.
!
!Array
with: newItem
	| newArray size |
	size <- self size.
	newArray <- self class new: size + 1.
	newArray replaceFrom: 1 to: size with: self.
	newArray at: size + 1 put: newItem
	^ newArray.
!
