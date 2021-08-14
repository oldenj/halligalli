Maximize
obj: 1 g

Subject To
beta1: g - y(FWDL|TrD|0|0|4|false|1,Unbek|Unbek|35|neg1|0|false|1) - y(FWDL|TrD|0|0|4|false|2,Unbek|Unbek|35|neg1|0|false|2) = 0
beta2: b - y(Unbek|Unbek|neg2|neg1|0|false|1,FWDL|TrD|0|0|2|false|1) = 0
beta3: d - y(FWDL|TrD|0|0|3|false|1,FWDL|TrD|0|0|4|false|1) - y(FWDL|TrD|0|0|3|false|2,FWDL|TrD|0|0|4|false|2) = 0
alpha1: g <= 4
alpha2: d <= 3
alpha3: b <= 2

Bounds
g >= 0
b >= 0
d >= 0
y(FWDL|TrD|0|0|4|false|1,Unbek|Unbek|35|neg1|0|false|1) >= 0
y(FWDL|TrD|0|0|4|false|2,Unbek|Unbek|35|neg1|0|false|2) >= 0
y(Unbek|Unbek|neg2|neg1|0|false|1,FWDL|TrD|0|0|2|false|1) >= 0
y(FWDL|TrD|0|0|3|false|1,FWDL|TrD|0|0|4|false|1) >= 0
y(FWDL|TrD|0|0|3|false|2,FWDL|TrD|0|0|4|false|2) >= 0

Integers

End
