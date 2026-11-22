" eight queens exercise "
+Object subclass: #NullQueen variables: #( ) classVariables: #( )
!NullQueen
first
	^ true.
!
!NullQueen
next
	^ false.
!
!NullQueen
checkRow: row column: column
	^ false.
!
!NullQueen
result
	^ List new.
!
+Object subclass: #Queen variables: #( row column neighbor ) classVariables: #( )
!Queen
setColumn: aNumber neighbor: aQueen
	column <- aNumber.
	neighbor <- aQueen.
!
!Queen
checkRow: testRow column: testColumn
	| columnDifference |
	columnDifference <- testColumn - column.
	( ( ( row = testRow ) or:
		[ row + columnDifference = testRow ] ) or:
		[ row - columnDifference = testRow ] )
			ifTrue: [ ^ true ].
	^ neighbor checkRow: testRow column: testColumn.
!
!Queen
first
	neighbor first.
	row <- 1.
	^ self testPosition.
!
!Queen
next
	^ self advance and: [ self testPosition ].
!
!Queen
advance
	( row = 8 )
		ifTrue: [ neighbor next ifFalse: [ ^ false ].
			row <- 0 ].
	row <- row + 1.
	^ true.
!
!Queen
testPosition
	[ neighbor checkRow: row column: column ]
		whileTrue: [ self advance ifFalse: [ ^ false ] ].
	^ true.
!
!Queen
result
	| list |
	" was: ^ neighbor result; addLast: row -- the value of a cascade differs between the two compilers "
	list <- neighbor result.
	list addLast: row.
	^ list.
!
