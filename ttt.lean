def boardSize : Nat := 20
def boardSizeSquared : Nat := boardSize * boardSize
def winCondition : Nat := 10
def numDiagonals : Nat := 2 * boardSize - 1

inductive Player
| circle
| cross

inductive Result
| win : Player → Result
| draw : Result

structure Board where
  board : Vector (Vector Bool boardSize) boardSize
  cols : Vector Nat boardSize
  rows : Vector Nat boardSize
  diag : Vector Nat numDiagonals
  anti : Vector Nat numDiagonals
deriving Inhabited

namespace Board

def update (x y : Nat) {h₁ : x < boardSize} {h₂ : y < boardSize} (b : Board) : Board :=
  let diagIdx := x - y + boardSize - 1
  let antiIdx := x + y
  have : diagIdx < numDiagonals := by sorry
  have : antiIdx < numDiagonals := by sorry
  { board := b.board.set y (b.board[y].set x true),
    cols := b.cols.set x (b.cols[x] + 1),
    rows := b.rows.set y (b.rows[y] + 1),
    diag := b.diag.set diagIdx (b.diag[diagIdx] + 1),
    anti := b.anti.set antiIdx (b.anti[antiIdx] + 1)}

partial def sumDir (x y dx dy : Int) (b : Board) : Nat :=
  if 0 ≤ x && 0 ≤ y && x < boardSize && y < boardSize && b.board[y.toNat]![x.toNat]! then
    1 + sumDir (x + dx) (y + dy) dx dy b
  else
    0

def checkWin (x y : Nat) {h1 : x < boardSize} {h2 : y < boardSize} (b : Board) : Bool := Id.run do
  -- Column
  if winCondition ≤ b.cols[x] then
    let acc := 1 + sumDir x (y + 1) 0 1 b + sumDir x (y - 1) 0 (-1) b
    if winCondition ≤ acc then
      return true

  -- Row
  if winCondition ≤ b.rows[y] then
    let acc := 1 + sumDir (x + 1) y 1 0 b + sumDir (x - 1) y (-1) 0 b
    if winCondition ≤ acc then
      return true

  -- Diagonal
  if winCondition ≤ b.diag[x - y + boardSize - 1]! then
    let acc := 1 + sumDir (x + 1) (y + 1) 1 1 b + sumDir (x - 1) (y - 1) (-1) (-1) b
    if winCondition ≤ acc then
      return true

  -- Anti-diagonal
  if winCondition ≤ b.anti[x + y]! then
    let acc := 1 + sumDir (x + 1) (y - 1) 1 (-1) b + sumDir (x - 1) (y + 1) (-1) 1 b
    if winCondition ≤ acc then
      return true

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
