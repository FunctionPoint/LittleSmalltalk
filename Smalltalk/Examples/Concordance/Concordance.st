" Concordance program: which words occur on which lines of the input "
+Object subclass: #Concordance variables: #( dictionary ) classVariables: #( )
!Concordance
word: word occursOnLine: line
	( dictionary includes: word )
		ifFalse: [ dictionary at: word put: List new ].
	( ( dictionary at: word ) includes: line )
		ifFalse: [ ( dictionary at: word ) addLast: line ].
!
!Concordance
fileIn
	| text lineNumber words |
	lineNumber <- 0.
	[ text <- String input.  text notNil ]
		whileTrue: [
			text <- text collect: [ :character | character lowerCase ].
			lineNumber <- lineNumber + 1.
			words <- text break: ' .,'.
			words do: [ :word | self word: word
				occursOnLine: lineNumber ] ].
!
!Concordance
displayDictionary
	dictionary binaryDo: [ :word :lines |
		word print.
		lines do: [ :line | ' ' print. line print ].
		Char newline print ].
!
!Concordance
run
	dictionary <- Dictionary new.
	self fileIn.
	self displayDictionary.
!
