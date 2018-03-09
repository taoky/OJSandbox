import java.io.*;
import java.util.*;

public class Main {
    public static void main(String[] args) throws IOException {
        Scanner sc = new Scanner(System.in);
		String s = sc.nextLine();
		String res = s;
		if (s == null || s.length() <= 1) {
		}
		else {
			StringBuffer sb = new StringBuffer(s);
			sb = sb.reverse();
			res = sb.toString();
		}
        System.out.println(res);
    }
}

