from sympy import *

xx = Symbol('x.x')
xy = Symbol('x.y')
xz = Symbol('x.z')

yx = Symbol('y.x')
yy = Symbol('y.y')
yz = Symbol('y.z')

zx = Symbol('z.x')
zy = Symbol('z.y')
zz = Symbol('z.z')

xe = Symbol('-xe')
ye = Symbol('-ye')
ze = Symbol('-ze')

A = Matrix(
    [
        [xx, xy, xz, xe],
        [yx, yy, yz, ye],
        [zx, zy, zz, ze],
        [ 0,  0,  0,  1],
    ]
    )

tax = Symbol('t.a.x')
tay = Symbol('t.a.y')
taz = Symbol('t.a.z')

tbx = Symbol('t.b.x')
tby = Symbol('t.b.y')
tbz = Symbol('t.b.z')

tcx = Symbol('t.c.x')
tcy = Symbol('t.c.y')
tcz = Symbol('t.c.z')

B = Matrix(
    [
        [tax],
        [tay],
        [taz],
        [  1],
    ]
    )

#out = (A*B).applyfunc(simplify)
#for a in out:
#    print(a)

out = (Transpose(A.inv()) * B).applyfunc(simplify)
for a in out:
    print(a)
