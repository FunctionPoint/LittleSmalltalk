!Number
isPrime
	( ( self rem: 2 ) = 0 ) ifTrue: [ ^ false ].
	( 3 to: ( self quo: 2 ) by: 2 ) do: [ :candidate |
		( ( self rem: candidate ) = 0 ) ifTrue: [ ^ false ] ].
	^ true.
!
=Number
primes: limit
	| candidate count |
	2 print.
	candidate <- 3.
	count <- 0.
	[ count < limit ] whileTrue: [
		[ candidate isPrime not ] whileTrue: [
			candidate <- candidate + 2 ].
		' ' print. candidate print.
		count <- count + 1.
		candidate <- candidate + 2 ].
	Char newline print.
!
