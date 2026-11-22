" the while example: loops, without classes of its own. lst runs Main>>start, then exits "
+Application subclass: #Main variables: #( ) classVariables: #( )
!Main
start
	1 to: 15 do: [ :number | number factorial print. Char newline print ].
	( 5 to: 1 by: -1 ) do: [ :number | number printNl ].
!
