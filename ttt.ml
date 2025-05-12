(* ttt_test - Speed test for random games of tic tac toe
   Released into the public domain by Brian Chen (differental) and Jeremy Chen (intgrah), 2025.

   This is free and unencumbered software released into the public domain.

   Anyone is free to copy, modify, publish, use, compile, sell, or distribute
   this software, either in source code form or as a compiled binary, for any
   purpose, commercial or non-commercial, and by any means.

   In jurisdictions that recognize copyright laws, the author or authors of
   this software dedicate any and all copyright interest in the software to
   the public domain. We make this dedication for the benefit of the public at
   large and to the detriment of our heirs and successors. We intend this
   dedication to be an overt act of relinquishment in perpetuity of all
   present and future rights to this software under copyright law.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   For more information, please refer to <https://unlicense.org/>
*)

let board_size = 20
let board_size_padded = board_size + 2
let board_size_squared = board_size * board_size
let win_condition = 10

type cell = {
  mutable s : int;
  mutable n : int;
  mutable e : int;
  mutable w : int;
  mutable se : int;
  mutable nw : int;
  mutable ne : int;
  mutable sw : int;
}

type board = cell array array

let make_cell () =
  { s = 1; n = 1; e = 1; w = 1; se = 1; nw = 1; ne = 1; sw = 1 }

let make_board () : board =
  let b =
    Array.make_matrix board_size_padded board_size_padded (make_cell ())
  in
  for y = 1 to board_size do
    for x = 1 to board_size do
      b.(y).(x) <- make_cell ()
    done
  done;
  b

let check_win b x y : bool =
  let q = b.(y).(x) in
  let col = q.s + q.n in
  let row = q.w + q.e in
  let diag = q.nw + q.se in
  let anti = q.ne + q.sw in

  if
    col > win_condition || row > win_condition || diag > win_condition
    || anti > win_condition
  then true
  else (
    b.(y + q.s).(x).n <- col;
    b.(y - q.n).(x).s <- col;
    b.(y).(x + q.e).w <- row;
    b.(y).(x - q.w).e <- row;
    b.(y + q.se).(x + q.se).nw <- diag;
    b.(y - q.nw).(x - q.nw).se <- diag;
    b.(y - q.ne).(x + q.ne).sw <- anti;
    b.(y + q.sw).(x - q.sw).ne <- anti;
    false)

let seed : int ref = ref 1729163

let rand max : int =
  seed := !seed lxor (!seed lsl 13);
  seed := !seed lxor (!seed lsr 17);
  seed := !seed lxor (!seed lsl 5);
  seed := !seed land 0xffffffff;
  (!seed * max) lsr 32

type player = Circle | Cross

exception Win of player

let do_game () : player option =
  let circle = make_board () in
  let cross = make_board () in
  let free_x = Array.make board_size_squared 0 in
  let free_y = Array.make board_size_squared 0 in
  let i = ref 0 in
  for y = 1 to board_size do
    for x = 1 to board_size do
      let j = rand (!i + 1) in
      free_x.(!i) <- free_x.(j);
      free_y.(!i) <- free_y.(j);
      free_x.(j) <- x;
      free_y.(j) <- y;
      incr i
    done
  done;

  try
    for i = 0 to board_size_squared - 1 do
      let x = free_x.(i) in
      let y = free_y.(i) in
      if i land 1 = 0 then (
        if check_win circle x y then raise_notrace (Win Circle))
      else if check_win cross x y then raise_notrace (Win Cross)
    done;
    None
  with Win p -> Some p

let () =
  let n = 10000 in
  let o, x, draw = (ref 0, ref 0, ref 0) in
  let start = Sys.time () in
  for _ = 0 to n - 1 do
    match do_game () with
    | Some Circle -> incr o
    | Some Cross -> incr x
    | None -> incr draw
  done;
  let duration = Sys.time () -. start in
  Format.printf "O/X/Draw: %d/%d/%d\n" !o !x !draw;
  Format.printf "Time taken: %d ms\n" (int_of_float (duration *. 1000.0))
