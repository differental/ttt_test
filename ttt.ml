let board_size = 20
let board_size_squared = board_size * board_size
let num_diagonals = (2 * board_size) - 1
let win_condition = 10

type board = {
  board : bool array array;
  cols : int array;
  rows : int array;
  diags : int array;
  anti_diags : int array;
}

let make_board () : board =
  {
    board = Array.make_matrix board_size board_size false;
    cols = Array.make board_size 0;
    rows = Array.make board_size 0;
    diags = Array.make num_diagonals 0;
    anti_diags = Array.make num_diagonals 0;
  }

let update_board board x y =
  board.board.(y).(x) <- true;
  board.cols.(x) <- board.cols.(x) + 1;
  board.rows.(y) <- board.rows.(y) + 1;
  board.diags.(x - y + board_size - 1) <-
    board.diags.(x - y + board_size - 1) + 1;
  board.anti_diags.(x + y) <- board.anti_diags.(x + y) + 1

let check_win b x y : unit =
  let acc = ref 0 in
  let xa = ref x in
  let ya = ref y in

  (* Column *)
  if b.cols.(x) >= win_condition then (
    acc := 1;
    ya := y + 1;
    while !ya < board_size && b.board.(!ya).(x) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      incr ya
    done;
    ya := y - 1;
    while 0 <= !ya && b.board.(!ya).(x) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      decr ya
    done);

  (* Row *)
  if b.rows.(y) >= win_condition then (
    acc := 1;
    xa := x + 1;
    while !xa < board_size && b.board.(y).(!xa) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      incr xa
    done;
    xa := x - 1;
    while 0 <= !xa && b.board.(y).(!xa) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      decr xa
    done);

  (* Diagonal *)
  if b.diags.(x - y + board_size - 1) >= win_condition then (
    acc := 1;
    xa := x + 1;
    ya := y + 1;
    while !xa < board_size && !ya < board_size && b.board.(!ya).(!xa) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      incr xa;
      incr ya
    done;
    xa := x - 1;
    ya := y - 1;
    while 0 <= !xa && 0 <= !ya && b.board.(!ya).(!xa) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      decr xa;
      decr ya
    done);

  (* Anti-diagonal *)
  if b.anti_diags.(x + y) >= win_condition then (
    acc := 1;
    xa := x + 1;
    ya := y - 1;
    while !xa < board_size && 0 <= !ya && b.board.(!ya).(!xa) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      incr xa;
      decr ya
    done;
    xa := x - 1;
    ya := y + 1;
    while 0 <= !xa && !ya < board_size && b.board.(!ya).(!xa) do
      incr acc;
      if !acc >= win_condition then raise Exit;
      decr xa;
      incr ya
    done)

let shuffle a =
  (* Fisher-Yates *)
  let n = Array.length a in
  for i = n - 1 downto 1 do
    let k = Random.int (i + 1) in
    let x = a.(k) in
    a.(k) <- a.(i);
    a.(i) <- x
  done

type player = Circle | Cross

exception Win of player

let do_game () : player option =
  let circle = make_board () in
  let cross = make_board () in
  let free = Array.init board_size_squared Fun.id in
  shuffle free;

  try
    for i = 0 to board_size_squared - 1 do
      let x = free.(i) / board_size in
      let y = free.(i) mod board_size in
      let player = if i mod 2 = 0 then Circle else Cross in
      let board = match player with Circle -> circle | Cross -> cross in
      update_board board x y;
      try check_win board x y with Exit -> raise (Win player)
    done;
    None
  with Win player -> Some player

let () =
  Random.init 1729;
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
