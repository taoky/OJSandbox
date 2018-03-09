import java.io.*;
import java.util.*;

public class Main {
	static int tmp[][] = new int[10000][10000];
    public static void main(String[] args) throws IOException {
		for (int i = 0; i < 10000; i++) {
			for (int j = 0; j < 10000; j++) {
				tmp[i][j] = i + j;
			}
		}
    }
}

