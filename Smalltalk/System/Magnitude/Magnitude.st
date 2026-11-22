" class definition for Magnitude "
+Object subclass: #Magnitude variables: #( ) classVariables: #( )
" class methods for Magnitude "
" instance methods for Magnitude "
!Magnitude
<= aMagnitude
	^ self < aMagnitude or: [ self = aMagnitude ].
!
!Magnitude
> aMagnitude
	^ aMagnitude < self.
!
!Magnitude
>= aMagnitude
	^ ( self > aMagnitude ) or: [ self = aMagnitude ].
!
!Magnitude
between: low and: high
	^ low <= self and: [ self <= high ].
!
!Magnitude
max: aMagnitude
	^ self < aMagnitude ifTrue: [ aMagnitude ] ifFalse: [ self ].
!
!Magnitude
min: aMagnitude
	^ self < aMagnitude ifTrue: [ self ] ifFalse: [ aMagnitude ].
!
