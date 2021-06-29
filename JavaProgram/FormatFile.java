import java.io.*;
import java.util.Scanner;

public class FormatFile {
    public static void main(String[] args) {
        try {
            Scanner input = new Scanner(new FileInputStream("Data.txt"));
            String[] bytesStr = input.nextLine().split(" ");
            input.close();

            byte[] bytes = new byte[bytesStr.length];
            System.out.printf("Get %d bytes: \n", bytes.length);

            byte oneByte;
            for (int i = 0; i < bytesStr.length; i++) {
                oneByte = 0;
                char[] chs = bytesStr[i].toCharArray();
                for (char c : chs) {
                    oneByte <<= 4;
                    if (c >= '0' && c <= '9')
                        oneByte += c - '0';
                    else if (c >= 'a' && c <= 'f')
                        oneByte += c - 'a' + 10;
                    else if (c >= 'A' && c <= 'F')
                        oneByte += c - 'A' + 10;
                }
                bytes[i] = oneByte;

                System.out.printf("0x%02X ", bytes[i]);
            }

            FileOutputStream output = new FileOutputStream("Data.dat");
            output.write(bytes);
            output.close();

            System.out.println("\nReading Data.dat......\n");
            BufferedInputStream binInput = new BufferedInputStream(new FileInputStream("Data.dat"));
            int readCnt = 0;
            int byteRead;
            while ((byteRead = binInput.read()) >= 0) {
                readCnt++;
                System.out.printf("0x%02X ", (byte) byteRead);
            }
            System.out.printf("\nRead %d bytes from Data.dat.\n", readCnt);
            binInput.close();
        } catch (Exception fileNotFounException) {
            System.out.println("Input file not found!\n");
        }
        /*
         * BufferedInputStream fin = new BufferedInputStream(new
         * FileInputStream("Data.txt")); String str = fin.
         */

    }
}
