" class definition for Class "
+Object subclass: #Class variables: #( name parentClass methods size variables ) classVariables: #( )
" class methods for Class "
" instance methods for Class "
!Class
addMethod: text
	| method |
	method <- self parseMethod: text.
	method notNil
		ifTrue: [
			methods at: method name put: method.
			Method flushCache.
			^ 'method inserted: ' + method name printString ].
	" the method did not compile: parseMethod: has printed the error "
	^ nil.
!
!Class
allMethods
	| methodDictionary |
	parentClass isNil
		ifTrue: [ methodDictionary <- Dictionary new ]
		ifFalse: [ methodDictionary <- parentClass allMethods ].
	methods binaryDo: [ :selector :method | methodDictionary at: selector put: method ].
	^ methodDictionary.
!
!Class
allVariables
	| names |
	" return all our variable names "
	parentClass notNil
		ifTrue: [ names <- parentClass allVariables ]
		ifFalse: [ names <- Array new: 0 ].
	( variables isNil or: [ variables isEmpty ] )
		ifFalse: [ names <- names + variables ].
	^ names.
!
!Class
basicNew
	" Like new "
	<7 self size>.
!
!Class
compile: text variables: names
	" the compiler of the VM: the selector, the bytecodes, the literals, the stack size and "
	" the number of temporaries of the method text (an Array), or the message of a syntax "
	" error (a String). names: the instance variables, superclass variables first "
	<180 text names>.
	self primitiveFailed.
!
!Class
compiledLiterals: pairs
	| literals pair |
	" the literals of a method from primitive 180, each a pair: #( 1 object ), #( 2 name ) "
	" for a symbol, #( 3 pairs ) for a literal array "
	literals <- Array new: pairs size.
	1 to: pairs size do: [ :index |
		pair <- pairs at: index.
		( pair at: 1 ) = 1 ifTrue: [ literals at: index put: ( pair at: 2 ) ].
		( pair at: 1 ) = 2 ifTrue: [ literals at: index put: ( pair at: 2 ) asSymbol ].
		( pair at: 1 ) = 3 ifTrue: [ literals at: index put: ( self compiledLiterals: ( pair at: 2 ) ) ] ].
	^ literals.
!
!Class
instanceVariables
	| names |
	" return all our variable names "
	parentClass notNil
		ifTrue: [ names <- parentClass instanceVariables ]
		ifFalse: [ names <- Array new: 0 ].
	( variables isNil or: [ variables isEmpty ] )
		ifFalse: [ names <- names + variables ].
	^ names.
!
!Class
listAllMethods
	self allMethods keysDo: [ :selector | selector printNl ].
!
!Class
listMethods
	methods keysDo:
		[ :name | name printNl ].
!
!Class
methods
	" return the tree of methods "
	^ methods.
!
!Class
name
	^ name.
!
!Class
name: newName parent: newParent variables: newVariables
	" create a new class with the given characteristics "
	name <- newName.
	parentClass <- newParent.
	methods <- Dictionary new.
	size <- newVariables size + newParent size.
	variables <- newVariables.
!
!Class
new
	" return a new instance of ourselves "
	<7 self size>.
!
!Class
parseMethod: text
	| result |
	" compile the method text for this class with the compiler of the VM; nil after an error "
	result <- self compile: text variables: self instanceVariables.
	( result isKindOf: String ) ifTrue: [
		'Compile error ' print.
		result printNl.
		^ nil ].
	^ Method name: ( result at: 1 ) asSymbol
		byteCodes: ( result at: 2 )
		literals: ( self compiledLiterals: ( result at: 3 ) )
		stackSize: ( result at: 4 )
		temporarySize: ( result at: 5 )
		class: self.
!
!Class
printString
	" just return our name "
	^ name printString.
!
!Class
size
	^ size.
!
!Class
subclass: className
	^ self subclass: className variables: ( Array new: 0 ) classVariables: ( Array new: 0 ).
!
!Class
subclass: className variables: instanceVariableNames
	^ self subclass: className variables: instanceVariableNames classVariables: ( Array new: 0 ).
!
!Class
subclass: className variables: instanceVariableNames classVariables: classVariableNames
	| metaclass metaName |
	" create the meta class and the class.  Add both to the globals. "
	metaName <- ( 'Meta' + className asString ) asSymbol.
	metaclass <- Class new name: metaName
		parent: self class
		variables: classVariableNames.
	" globals at: metaName put: meta. "

	" make the actual class "
	globals at: className put: ( metaclass new name: className
		parent: self
		variables: instanceVariableNames ).
	^ 'subclass created: ' + className printString.
!
!Class
subclasses
	^ ( globals select: [ :object | ( object isKindOf: Class ) and: [ ( object superclass ) = self ] ] ).
!
!Class
subclasses: indent
	globals do: [ :object |
		( ( object isKindOf: Class ) and: [ object superclass == self ] )
			ifTrue: [
				1 to: indent do: [ :ignore | $  print ].
				object printNl.
				object subclasses: indent + 4 ] ].
!
!Class
superclass
	^ parentClass.
!
!Class
variables
	^ variables.
!
