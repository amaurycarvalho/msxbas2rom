1 dim ax(2), ay(2)
2 aX(0)=20 : aY(0)=171
3 aX(1)=128 : aY(1)=20
4 aX(2)=235 : aY(2)=171
5 X = aX(0) : Y = aY(0)
6 SCREEN 2
7 PSET(X,Y)
8 I=INT(RND(1)*3)
9 X=(X + aX(I))*0.5 : Y=(Y + aY(I))*0.5
10 GOTO 7
