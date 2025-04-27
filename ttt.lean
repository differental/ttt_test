def boardSize : Nat := 20
def boardPadded : Nat := boardSize + 2
def boardSizeSquared : Nat := boardSize * boardSize
def winCondition : Nat := 10
def numDiagonals : Nat := 2 * boardSize - 1

inductive Player
| circle
| cross

inductive Result
| win : Player → Result
| draw : Result

structure Cell where
  n : Nat
  s : Nat
  e : Nat
  w : Nat
  ne : Nat
  se : Nat
  sw : Nat
  nw : Nat
deriving Inhabited

abbrev Board := Vector (Vector Cell boardPadded) boardPadded

namespace Board

def checkWin (x y : Nat) (b : Board) : Bool := Id.run do
  let q := b[y]![x]!
  let col := q.s + 1 + q.n
  let row := q.w + 1 + q.e
  let diag := q.nw + 1 + q.se
  let anti := q.ne + 1 + q.sw
  if col >= winCondition || row >= winCondition || diag >= winCondition || anti >= winCondition then
    return true
  b.set (y + q.s + 1) (x) {n := col}
  b.set (y - q.n - 1) (x) {s := col}
  b.set (y) (x + q.e + 1) {w := row}
  b.set (y) (x - q.w - 1) {e := row}
  b.set (y + q.se + 1) (x + q.se + 1) {nw := diag}
  b.set (y - q.nw - 1) (x - q.nw - 1) {se := diag}
  b.set (y - q.ne - 1) (x + q.ne + 1) {sw := anti}
  b.set (y + q.sw + 1) (x - q.sw - 1) {ne := anti}
  return false

end Board

abbrev Random := StateT UInt64 Id

-- XorShift
def rand : Random Nat := do
  let seed ← get
  let seed := seed.xor (seed.shiftLeft 13)
  let seed := seed.xor (seed.shiftRight 17)
  let seed := seed.xor (seed.shiftLeft 5)
  let seed := seed.land 0xffffffff
  set seed
  return seed.toNat

-- Fisher-Yates
def shuffle [Inhabited α] (v : Vector α n) : Random (Vector α n) := do
  let mut v := v
  let mut i := n - 1
  while i > 0 do
    let k ← rand
    let k := k % (i + 1)
    v := v.swap i k (by sorry) (by sorry)
    i := i - 1
  return v

def doGame : Random Result := do
  let freeSquares : Vector (Nat × Nat) boardSizeSquared :=
    Vector.ofFn fun i => (i % boardSize, i / boardSize)
  let freeSquares ← shuffle freeSquares

  let mut circleBoard : Board := default
  let mut crossBoard : Board := default
  for ((x, y), i) in freeSquares.zipIdx do
    if i % 2 == 0 then
      circleBoard := @circleBoard.update x y sorry sorry
      if @circleBoard.checkWin x y sorry sorry then
        return .win .circle
    else
      crossBoard := @crossBoard.update x y sorry sorry
      if @crossBoard.checkWin x y sorry sorry then
        return .win .cross
  return .draw


def main : IO Unit := do
  let n := 10000
  let seed := 1729163
  let startTime ← IO.monoMsNow

  let (o, x, d) :=
    List.replicate n doGame
    |>.mapM id
    |>.run seed
    |> Prod.fst
    |>.foldl
    (fun (o, x, d) result =>
      match result with
      | .win .circle => (o + 1, x, d)
      | .win .cross => (o, x + 1, d)
      | .draw => (o, x, d + 1))
    default

  let endTime ← IO.monoMsNow
  let duration := endTime - startTime
  println! "O/X/Draw: {o}/{x}/{d}"
  println! "Time taken: {duration} ms"
