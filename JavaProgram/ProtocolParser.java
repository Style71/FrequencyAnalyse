import java.io.*;
import java.util.ArrayList;

public class ProtocolParser {

    public static final int PACKET_BUFFER_SIZE = 20; // This constant should be slightly greater than
                                                     // MAX_PACKET_PAYLOAD_SIZE
    public static final float deltaf_400Hz_freq = 7.8;
    public static final float deltaf_100Hz_freq = 1.25;
    public static final float deltaf_35Hz_freq = 0.1;

    enum ParsingState {
        NoMessage, MessageIncoming, DecodeHead, ReceivingColon, ReceivingPayload, Checksum,
    };

    private ParsingState sMessageFlags;
    private int ucBufferIndex;
    private byte ucPacketHead;

    private RandomAccessQueue<Byte> cReparsingBuffer;

    private byte calculateChecksum(char[] msg, int len) {
        byte checksum = msg[0];

        for (int i = 1; i < len; i++)
            checksum ^= msg[i]; // XOR of all bytes.

        return checksum;
    }

    private byte calculateChecksum(RandomAccessQueue<Byte> buffer, int len) {
        char checksum = buffer.get(0);

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
        msg[2 + payload.length] = calculateChecksum(msg, payload_len + 2);

        putc_callback(msg);
        // USART_Putchars(&huart1, (const char *)msg, payload_len + 3);
    }

    private void recv_packet(byte head, byte[] payload)
    {
        WavePara freq;
        BatteryStatus Batt;
        boolean[] channelEnableBytes;

        switch (head)
        {
        case 0x51:
        case 0x52:
        case 0x53:
            memcpy(&freq.t, &payload[0], 4);
            memcpy(&freq.freq, &payload[4], 4);
            memcpy(&freq.mag, &payload[8], 4);

            frequency_callback(head & 0x0F, freq);
            break;
        case 0x59:
            memcpy(&Batt.t, &payload[0], 4);
            memcpy(&Batt.voltage, &payload[4], 2);
            memcpy(&Batt.current, &payload[6], 2);
            Batt.capacity = payload[8];

            battery_callback(Batt);
            break;
        case 0x5F:
            channelEnableBytes=new boolean[3];
            for (int i = 0; i < 3; i++)
                channelEnableBytes[i] = (payload[i]>0) ? true : false;

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

    public ProtocolStream(put_chars_callback &func_put_chars, recv_channel_enable_info_callback &func_channel_enable, recv_battery_info_callback &func_battery, recv_frequency_info_callback &func_frequency){
        cReparsingBuffer = new RandomAccessQueue<Byte>[PACKET_BUFFER_SIZE];
        sMessageFlags = NoMessage;
        ucBufferIndex = 0;
    }

    public byte ParsingMessage(byte[] msg, int len) {
        byte ret = 0x00;
        byte c;
        int ucPayloadLen;
        while (i < len) {
            if (ucBufferIndex == cReparsingBuffer.length)
                cReparsingBuffer.offer(msg[i++]);

            c = cReparsingBuffer.get(ucBufferIndex++);
            switch (sMessageFlags) {
                case NoMessage: // While the state machine is in idle state.
                    if (c == 0x5A) {
                        // Get a possible packet head, change to incoming state.
                        sMessageFlags = MessageIncoming;
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
                            sMessageFlags = ReceivingPayload;
                            break;
                        default:
                            // Reset to idle state.
                            sMessageFlags = NoMessage;
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
                        if (ucPayloadLen == 16)
                            sMessageFlags = Checksum; // Set to checksum state.
                    }
                    if (ucPacketHead == 0x59) {
                        if (ucPayloadLen == 9)
                            sMessageFlags = Checksum; // Set to checksum state.
                    }
                    if (ucPacketHead == 0x5F) {
                        if (ucPayloadLen == 3)
                            sMessageFlags = Checksum; // Set to checksum state.
                    }
                    break;
                case Checksum: // While the state machine is in checksum state.
                    // Get the packet message index.
                    ucPayloadLen = ucBufferIndex - 1;
                    if (calculateChecksum(cReparsingBuffer, ucPayloadLen) == c) {
                        recv_packet(ucPacketHead, cReparsingBuffer, ucPayloadLen);
                        cReparsingBuffer.take(ucBufferIndex);
                        ret = ucPacketHead;
                    } else {
                        cReparsingBuffer.take();
                    }
                    // In both case, reset to idle state.
                    sMessageFlags = NoMessage;
                    ucBufferIndex = 0;
                    break;
                default:
                    break;
            }
        }
        return ret;
    }

    public void send_battery_info(BatteryStatus &battery)
    {
        byte temp;
        byte[] payload= new byte[9];
        byte head = 0x59;

        memcpy(&payload[0], &battery.t, 4);
        memcpy(&payload[4], &battery.voltage, 2);
        memcpy(&payload[6], &battery.current, 2);
        temp = (byte)battery.capacity;
        memcpy(&payload[8], &temp, 1);

        send_packet(head, payload);
    }

    public void send_frequency_info(int channel, WavePara wave)
    {
        float temp;
        byte[] payload= new byte[16];
        byte head;
        switch (channel)
        {
        case 1:
            head = 0x51;
            temp = 1.0 / deltaf_400Hz_freq;
            memcpy(&payload[12], &temp, 4);
            break;
        case 2:
            head = 0x52;
            temp = 1.0 / deltaf_100Hz_freq;
            memcpy(&payload[12], &temp, 4);
            break;
        case 3:
            head = 0x53;
            temp = 1.0 / deltaf_35Hz_freq;
            memcpy(&payload[12], &temp, 4);
            break;
        default:
            head = 0x50;
            break;
        }
        memcpy(&payload[0], &wave.t, 4);
        memcpy(&payload[4], &wave.freq, 4);
        memcpy(&payload[8], &wave.mag, 4);

        send_packet(head, payload);
    }

    public void send_channel_enable_info(boolean[] channelEnable) {
        byte[] payload = new byte[3];
        byte head = 0x5F;

        for (int i = 0; i < 3; i++)
            payload[i] = channelEnable[i] ? 1 : 0;
        send_packet(head, payload);
    }

    public static void main(String[] args) {
        try {
            InputStream fStream = new FileInputStream("Data.txt");
            int size = fStream.available();

            for (int i = 0; i < size; i++) {
                System.out.print((char) fStream.read());
            }
            fStream.close();

            // System.out.print(reader.readLine());
        } catch (Exception e) {
            // TODO: handle exception
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