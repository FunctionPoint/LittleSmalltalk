" the startup code of the eight queens example: lst runs Main>>start, then exits "
+Application subclass: #Main variables: #( ) classVariables: #( )
!Main
start
	| lastQueen |
	lastQueen <- NullQueen new.
	1 to: 8 do: [ :column | lastQueen <- Queen new;
		setColumn: column neighbor: lastQueen ].
	lastQueen first.
	'got a result' print.
	lastQueen result do: [ :row | row print. ' ' print ].
	Char newline print.
	[ lastQueen next ] whileTrue:
		[
	lastQueen result do: [ :row | row print. ' ' print ].
	Char newline print ].
!
