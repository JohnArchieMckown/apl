
(10×2 3⍴⍳6){(⍺+⍵)⊣⎕←'LEFT: ⍺=',(⍕⍺),', ⍵=',(⍕⍵)}.{(⍺×⍵)⊣⎕←'RIGHT: ⍺=',(⍕⍺),', ⍵=',(⍕⍵)}(1000×2 3⍴⍳6)

----------------------------------

Q←10×2 3⍴⍳6
Q{(⍺+⍵)⊣⎕←'LEFT: ⍺=',(⍕⍺),', ⍵=',(⍕⍵)}.{(⍺×⍵)⊣⎕←'RIGHT: ⍺=',(⍕⍺),', ⍵=',(⍕⍵)}Q


----------------------------------

Q←10×2 3⍴⍳6

∇A LL B
(A+B)⊣⎕←'LEFT: A=',(⍕A),', B=',(⍕B)
∇

∇A RR B
(A×B)⊣⎕←'RIGHT: A=',(⍕A),', B=',(⍕B)
∇

Q LL.RR Q
