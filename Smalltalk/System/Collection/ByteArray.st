" class definition for ByteArray "
+Array subclass: #ByteArray variables: #( ) classVariables: #( )
" class methods for ByteArray "
=ByteArray
new: size
	<20 self size>.
!
" instance methods for ByteArray "
!ByteArray
asString
	| string size |
	size <- self size.
	string <- String new: size.
	1 to: size do: [ :index | string at: index put: ( ( self at: index ) asChar ) ].
	^ string.
!
!ByteArray
at: index
	<21 self index>
	( self includesKey: index ) ifFalse: [ self badIndex ].
	self primitiveFailed.
!
!ByteArray
at: index ifAbsent: exceptionBlock
	<21 self index>
	exceptionBlock value.
!
!ByteArray
at: index put: aValue
	<22 aValue self index>
	( self includesKey: index ) ifFalse: [ self badIndex ].
	self primitiveFailed.
!
!ByteArray
basicAt: index
	<21 self index>
	^nil.
!
