import java.io.*;
import java.util.ArrayList;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class WavePara {
    public long t;
    public float freq;
    public float mag;
    public float freq_deriv;

    public WavePara() {
        t = 0;
        freq = 0;
        mag = 0;
        freq_deriv = 0;
    }
};

class BatteryStatus {
    public long t;
    public short voltage;
    public short current;
    public byte capacity;

    public BatteryStatus() {
        t = 0;
        voltage = 0;
        current = 0;
        capacity = 0;
    }
};

public class ProtocolParser {

    public static final int PACKET_BUFFER_SIZE = 32; // This constant should be slightly greater than
                                                     // MAX_PACKET_PAYLOAD_SIZE
    public static final float deltaf_400Hz_freq = (float) 7.8125;
    public static final float deltaf_100Hz_freq = (float) 0.390625;
    public static final float deltaf_35Hz_freq = (float) 0.09765625;

    enum ParsingState {
        NoMessage, MessageIncoming, DecodeHead, ReceivingColon, ReceivingPayload, Checksum,
    };

    private ParsingState sMessageFlags;
    private int ucBufferIndex;
    private byte ucPacketHead;

    private RandomAccessQueue<Byte> cReparsingBuffer;

    private byte calculateChecksum(byte[] msg, int len) {
        byte checksum = msg[0];

        for (int i = 1; i < len; i++)
            checksum ^= msg[i]; // XOR of all bytes.

        return checksum;
    }

    private byte calculateChecksum(RandomAccessQueue<Byte> buffer, int len) {
        byte checksum = buffer.get(0);

        for (int i = 1; i < len; i++)
            checksum ^= buffer.get(i); // XOR of all bytes.

        return checksum;
    }

    private void send_packet(byte head, byte[] payload) {
        byte[] msg = new byte[payload.length + 3];

        msg[0] = 0x5A;
        msg[1] = head;
        for (int i = 0; i < payload.length; i++) {
            msg[i + 2] = payload[i];
        }
        msg[2 + payload.length] = calculateChecksum(msg, payload.length + 2);

        putc_callback(msg);
    }

    private void recv_packet(byte head, byte[] payload) {
        WavePara freq = new WavePara();
        BatteryStatus Batt = new BatteryStatus();
        boolean[] channelEnableBytes;

        ByteBuffer buf = ByteBuffer.wrap(payload);
        buf.order(ByteOrder.LITTLE_ENDIAN);

        switch (head) {
            case 0x51:
            case 0x52:
            case 0x53:
                freq.t = buf.getLong();
                freq.freq = buf.getFloat();
                freq.mag = buf.getFloat();
                freq.freq_deriv = buf.getFloat();

                frequency_callback(head & 0x0F, freq);
                break;
            case 0x59:
                Batt.t = buf.getLong();
                Batt.voltage = buf.getShort();
                Batt.current = buf.getShort();
                Batt.capacity = buf.get();

                battery_callback(Batt);
                break;
            case 0x5F:
                channelEnableBytes = new boolean[3];
                for (int i = 0; i < 3; i++)
                    channelEnableBytes[i] = (payload[i] > 0) ? true : false;

                channel_enable_callback(channelEnableBytes);
                break;
            default:
                break;
        }
    }

    private void recv_packet(byte head, RandomAccessQueue<Byte> buffer, int payload_len) {
        byte[] payload = new byte[payload_len];

        for (int i = 0; i < payload_len; i++)
            payload[i] = buffer.get(i + 2);

        recv_packet(head, payload);
    }

    public ProtocolParser() {
        cReparsingBuffer = new RandomAccessQueue<Byte>(PACKET_BUFFER_SIZE);
        sMessageFlags = ParsingState.NoMessage;
        ucBufferIndex = 0;
    }

    public byte ParsingMessage(byte[] msg, int len) {
        byte ret = 0x00;
        byte c;
        int ucPayloadLen;
        int i = 0;

        while (i < len) {
            if (ucBufferIndex == cReparsingBuffer.length)
                cReparsingBuffer.offer(msg[i++]);

            c = cReparsingBuffer.get(ucBufferIndex++);
            switch (sMessageFlags) {
                case NoMessage: // While the state machine is in idle state.
                    if (c == 0x5A) {
                        // Get a possible packet head, change to incoming state.
                        sMessageFlags = ParsingState.MessageIncoming;
                    } else // Discard the character if it is not '0x5A'.
                    {
                        ucBufferIndex = 0;
                        cReparsingBuffer.take();
                    }
                    break;
                case MessageIncoming: // While the state machine is in incoming state.
                    switch (c) {
                        case 0x51:
                        case 0x52:
                        case 0x53:
                        case 0x59:
                        case 0x5F:
                            // Get a packet head, change to new state and record the payload.
                            sMessageFlags = ParsingState.ReceivingPayload;
                            break;
                        default:
                            // Reset to idle state.
                            sMessageFlags = ParsingState.NoMessage;
                            // If we fail to parse the packet, discard the first byte and repase the input
                            // string from the buffer.
                            ucBufferIndex = 0;
                            cReparsingBuffer.take();
                            break;
                    }
                    break;
                case ReceivingPayload: // While the state machine is in payload receiving state.
                    // Get the packet message index.
                    ucPayloadLen = ucBufferIndex - 2;
                    // Get the packet head.
                    ucPacketHead = cReparsingBuffer.get(1);
                    if ((ucPacketHead == 0x51) || (ucPacketHead == 0x52) || (ucPacketHead == 0x53)) {
                        if (ucPayloadLen == 20)
                            sMessageFlags = ParsingState.Checksum; // Set to checksum state.
                    }
                    if (ucPacketHead == 0x59) {
                        if (ucPayloadLen == 13)
                            sMessageFlags = ParsingState.Checksum; // Set to checksum state.
                    }
                    if (ucPacketHead == 0x5F) {
                        if (ucPayloadLen == 3)
                            sMessageFlags = ParsingState.Checksum; // Set to checksum state.
                    }
                    break;
                case Checksum: // While the state machine is in checksum state.
                    // Get the packet message index.
                    ucPayloadLen = ucBufferIndex - 1;// This ucPayloadLen contains 2 bytes head and actual payload
                                                     // bytes, exclude the checksum byte.
                    if (calculateChecksum(cReparsingBuffer, ucPayloadLen) == c) {
                        recv_packet(ucPacketHead, cReparsingBuffer, ucPayloadLen - 2);// The length here is the payload
                                                                                      // bytes length so we minus 2.
                        cReparsingBuffer.take(ucBufferIndex);
                        ret = ucPacketHead;
                    } else {
                        cReparsingBuffer.take();
                    }
                    // In both case, reset to idle state.
                    sMessageFlags = ParsingState.NoMessage;
                    ucBufferIndex = 0;
                    break;
                default:
                    break;
            }
        }
        return ret;
    }

    public void send_battery_info(BatteryStatus battery) {
        byte[] payload = new byte[13];
        ByteBuffer buf = ByteBuffer.wrap(payload);
        buf.order(ByteOrder.LITTLE_ENDIAN);

        byte head = 0x59;

        buf.putLong(battery.t);
        buf.putShort(battery.voltage);
        buf.putShort(battery.current);
        buf.put(battery.capacity);

        send_packet(head, payload);
    }

    public void send_frequency_info(int channel, WavePara wave) {
        byte[] payload = new byte[20];
        ByteBuffer buf = ByteBuffer.wrap(payload);
        buf.order(ByteOrder.LITTLE_ENDIAN);

        byte head;
        switch (channel) {
            case 1:
                head = 0x51;
                break;
            case 2:
                head = 0x52;
                break;
            case 3:
                head = 0x53;
                break;
            default:
                head = 0x50;
                break;
        }

        buf.putLong(wave.t);
        buf.putFloat(wave.freq);
        buf.putFloat(wave.mag);
        buf.putFloat(wave.freq_deriv);

        send_packet(head, payload);
    }

    public void send_channel_enable_info(boolean[] channelEnable) {
        byte[] payload = new byte[3];
        byte head = 0x5F;

        for (int i = 0; i < 3; i++)
            payload[i] = (channelEnable[i]) ? (byte) 1 : 0;
        send_packet(head, payload);
    }

    private void channel_enable_callback(boolean[] channelEnable) {
        for (int i = 0; i < 3; i++) {
            System.out.printf("Channel%i: %s \t", i, (channelEnable[i]) ? "Enabled" : "Disabled");
        }
        System.out.print('\n');
    }

    private void battery_callback(BatteryStatus Batt) {
        System.out.printf("t = %.3fs, Voltage = %dmV, Current = %dmA, Capacity = %d%%\n", (float) Batt.t / 1000000.0,
                Batt.voltage, Batt.current, Batt.capacity);
    }

    private void frequency_callback(int channel, WavePara freq) {
        System.out.printf("t = %.3fs, f%d = %.2f+-%.2fHz, mag = %.2fmV\n", (float) freq.t / 1000000.0, channel,
                freq.freq, freq.freq_deriv, freq.mag * 1000);
    }

    private void putc_callback(byte[] msg) {

    }

    public static void main(String[] args) {
        try {
            BufferedInputStream binInput = new BufferedInputStream(new FileInputStream("Data.dat"));
            ProtocolParser parser = new ProtocolParser();

            // int readCnt = 0;
            int byteRead;
            byte[] msg;

            while ((byteRead = binInput.available()) > 0) {
                // readCnt+=byteRead;
                msg = binInput.readNBytes(byteRead);
                parser.ParsingMessage(msg, byteRead);
            }
            // System.out.printf("\nRead %d bytes from Data.dat.\n", readCnt);
            binInput.close();
        } catch (FileNotFoundException ex) {
            System.out.println(ex.getMessage());
            System.out.println("Input file not found!\n");
        } catch (IOException ex) {
            System.out.println(ex.getMessage());
        }
    }

}

class RandomAccessQueue<T> {

    private int size; // 队列容量

    public int length; // 队列长度

    private int front, rear; // 队首序号，队尾序号

    private ArrayList<T> list;

    public RandomAccessQueue(int size) {
        list = new ArrayList<T>(size);
        for (int i = 0; i < size; i++) {
            list.add(null);
        }
        this.size = size;
        this.front = 0;
        this.rear = 0;
    }

    /**
     * 获取index所对应的元素，这种随机访问队列相对于普通队列的最重要的作用
     * 
     * @param index 需要访问的元素序号（从队首到队尾的序号，队首序号为0）
     * @return index所对应的元素，index超出队列的长度则返回null
     */
    public T get(int index) {
        if (index > length) {
            return null;
        }
        return list.get((front + index) % size);
    }

    /**
     * 队首元素出队
     * 
     * @return 出队的队首元素，如果队列为空，则返回null
     */
    public T take() {
        if (length <= 0) {
            return null;
        }
        T t = list.get(front);
        list.set(front, null);
        front = (front + 1) % size;
        length--;
        return t;
    }

    /**
     * 队首元素出队
     * 
     * @return 出队的队首元素，如果队列为空，则返回null
     */
    public void take(int num) {
        for (int i = 0; i < num; i++)
            take();
    }

    /**
     * 元素入队
     * 
     * @param t 入队元素
     * @return 如果队满，则入队失败，返回false，否则将元素插入队尾，返回true
     */
    public boolean offer(T t) {
        checkNotNull(t);
        if (length == size) {
            return false;
        }
        list.set(rear, t);
        rear = (rear + 1) % size;
        length++;
        return true;
    }

    /**
     * 获取队列的容量
     * 
     * @return 队列的容量
     */
    public int size() {
        return size;
    }

    private static void checkNotNull(Object v) {
        if (v == null)
            throw new NullPointerException();
    }

    /**
     * 清空队列
     */
    public void clear() {
        for (int i = 0; i < size; i++) {
            list.set(i, null);
        }
        this.front = 0;
        this.rear = 0;
    }

    /**
     * 判断队列是否为空
     * 
     * @return 是否为空
     */
    public boolean isEmpty() {
        if (length == 0) {
            return true;
        }
        return false;
    }

    /**
     * 判断队列是否满
     * 
     * @return 是否满
     */
    public boolean isFull() {
        if (length == size) {
            return true;
        }
        return false;
    }

}