" unit tests for Symbol "
+Test subclass: #TestSymbol variables: #( ) classVariables: #( )
!TestSymbol
testInterning
	self assert: #abc == #abc.
	self assert: ( Symbol new: 'xyz' ) == #xyz.
	self assert: #abc asSymbol == #abc.
	self assert: 'abc' asSymbol == #abc.
!
!TestSymbol
testConversion
	self assert: #abc printString = 'abc'.
	self assert: #abc asString class == String.
	self assert: #abc hash = 'abc' hash.
	self assert: #at:put: printString = 'at:put:'.
	self assert: #+ printString = '+'.
!
!TestSymbol
testComparing
	" by their bytes (primitive 181), with a Symbol or a String "
	self assert: ( #abc compare: #abd ) = -1.
	self assert: ( #abd compare: #abc ) = 1.
	self assert: ( #abc compare: #abc ) = 0.
	self assert: ( #abc compare: 'abc' ) = 0.
	self assert: ( #ab compare: #abc ) = -1.
	self assert: ( #abc compare: 3 ) isNil.
	self assert: #abc < #abd.
	self assert: #ab < #abc.
	self assert: ( #abd < #abc ) not.
	self assert: #abc < 'abd'.
	self assert: #abc = 'abc'.
	self assert: #abc = #abc.
	self assert: ( #abc = #abd ) not.
	self assert: ( #abc = 3 ) not.
	self assert: #Zebra < #apple.
!
