import java.util.ArrayList;
import java.util.Collections;
import java.util.Random;

public class ttt {
    private static final int BOARD_SIZE = 20;
    private static final int BOARD_SIZE_SQUARED = BOARD_SIZE * BOARD_SIZE;
    private static final int WIN_CONDITION = 10;

    private static class Board {
        private boolean[][] board;
        private int[] cols;
        private int[] rows;
        private int[] diag;
        private int[] anti;

        public Board() {
            board = new boolean[BOARD_SIZE][BOARD_SIZE];
            cols = new int[BOARD_SIZE];
            rows = new int[BOARD_SIZE];
            diag = new int[2 * BOARD_SIZE - 1];
            anti = new int[2 * BOARD_SIZE - 1];
        }

        public void update(int x, int y) {
            board[y][x] = true;
            cols[x]++;
            rows[y]++;
            diag[x - y + BOARD_SIZE - 1]++;
            anti[x + y]++;
        }

        public boolean checkWin(int x, int y) {
            // Column
            if (cols[x] >= WIN_CONDITION) {
                int acc = 1;
                for (int ya = y; ++ya < BOARD_SIZE && board[ya][x]; acc++)
                    ;
                for (int ya = y; 0 <= --ya && board[ya][x]; acc++)
                    ;
                if (acc >= WIN_CONDITION)
                    return true;
            }

            // Row
            if (rows[y] >= WIN_CONDITION) {
                int acc = 1;
                for (int xa = x; ++xa < BOARD_SIZE && board[y][xa]; acc++)
                    ;
                for (int xa = x; 0 <= --xa && board[y][xa]; acc++)
                    ;
                if (acc >= WIN_CONDITION)
                    return true;
            }

            // Diagonal
            if (diag[x - y + BOARD_SIZE - 1] >= WIN_CONDITION) {
                int acc = 1;
                for (int xa = x, ya = y; ++xa < BOARD_SIZE && ++ya < BOARD_SIZE && board[ya][xa]; acc++)
                    ;
                for (int xa = x, ya = y; 0 <= --xa && 0 <= --ya && board[ya][xa]; acc++)
                    ;
                if (acc >= WIN_CONDITION)
                    return true;
            }

            // Anti-diagonal
            if (anti[x + y] >= WIN_CONDITION) {
                int acc = 1;
                for (int xa = x, ya = y; ++xa < BOARD_SIZE && 0 <= --ya && board[ya][xa]; acc++)
                    ;
                for (int xa = x, ya = y; 0 <= --xa && ++ya < BOARD_SIZE && board[ya][xa]; acc++)
                    ;
                if (acc >= WIN_CONDITION)
                    return true;
            }

            return false;
        }
    }

    private static enum Result {
        CIRCLE,
        CROSS,
        DRAW
    }

    private static Result doGame(Random rng) {
        ArrayList<Integer> freeCells = new ArrayList<>(BOARD_SIZE_SQUARED);

        for (int i = 0; i < BOARD_SIZE_SQUARED; i++) {
            freeCells.add(i);
        }

        Collections.shuffle(freeCells, rng);

        Board circle = new Board();
        Board cross = new Board();

        for (int i = 0; i < BOARD_SIZE_SQUARED; i++) {
            int x = freeCells.get(i) % BOARD_SIZE;
            int y = freeCells.get(i) / BOARD_SIZE;

            if (i % 2 == 0) {
                circle.update(x, y);
                if (circle.checkWin(x, y))
                    return Result.CIRCLE;
            } else {
                cross.update(x, y);
                if (cross.checkWin(x, y))
                    return Result.CROSS;
            }
        }

        return Result.DRAW;
    }

    public static void main(String[] args) {
        Random rng = new Random(1729);

        final int n = 10000;
        int o = 0, x = 0, draw = 0;

        long start = System.nanoTime();

        for (int i = 0; i < n; i++) {
            switch (doGame(rng)) {
                case CIRCLE:
                    o++;
                    break;
                case CROSS:
                    x++;
                    break;
                case DRAW:
                    draw++;
                    break;
            }
        }

        long end = System.nanoTime();
        long duration = (end - start) / 1000000;

        System.out.printf("O/X/Draw: %d/%d/%d\n", o, x, draw);
        System.out.printf("Time taken: %d ms\n", duration);
    }
}
