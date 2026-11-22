" unit tests for Dictionary "
+Test subclass: #TestDictionary variables: #( ) classVariables: #( )
!TestDictionary
testAccess
	| dictionary |
	dictionary <- Dictionary new.
	self assert: ( dictionary at: #a put: 1 ) = 1.
	dictionary at: #b put: 2.
	self assert: ( dictionary at: #a ) = 1.
	self assert: ( dictionary at: #z ifAbsent: [ 0 ] ) = 0.
	dictionary at: #a put: 3.
	self assert: ( dictionary at: #a ) = 3.
	self assert: dictionary size = 2.
!
!TestDictionary
testRemoving
	| dictionary |
	dictionary <- Dictionary new.
	dictionary at: #a put: 1.
	dictionary removeKey: #a.
	self assert: dictionary isEmpty.
	self assert: ( dictionary at: #a ifAbsent: [ nil ] ) isNil.
!
!TestDictionary
testEnumerating
	| dictionary keys sum |
	dictionary <- Dictionary new.
	dictionary at: #b put: 1.
	dictionary at: #a put: 2.
	sum <- 0.
	dictionary do: [ :value | sum <- sum + value ].
	self assert: sum = 3.
	keys <- List new.
	dictionary keysDo: [ :key | keys addLast: key ].
	self assert: keys size = 2.
	self assert: ( keys asArray includes: #a ).
	sum <- 0.
	dictionary binaryDo: [ :key :value | sum <- sum + value ].
	self assert: sum = 3.
	self assert: dictionary keysAsArray size = 2.
!
