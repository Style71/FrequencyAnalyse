import java.util.ArrayList;
import java.util.Scanner;

public class Welcome {
	public static void main(String[] args) {

		int size;
		Scanner input = new Scanner(System.in);
		size = input.nextInt();
		input.close();
		char[] outString = new char[size];

		System.out.println("Welcome to java!\n");
		System.out.println("The size of \n");
		RandomAccessQueue<Character> queue = new RandomAccessQueue<Character>(50);
		for (char i = 0x41; i < 0x5B; i++) {
			queue.offer(i);
		}
		System.out.printf("Before take(), queue has %d elements: ", queue.length);
		for (int i = 0; i < queue.length; i++) {
			System.out.printf("%c ", queue.get(i));
		}
		for (int i = 0; i < 10; i++) {
			queue.take();
		}
		System.out.printf("\nAfter take() invoke 10 times, queue has %d elements: ", queue.length);
		for (int i = 0; i < queue.length; i++) {
			System.out.printf("%c ", queue.get(i));
		}

		for (char i = '0'; i <= '9'; i++) {
			queue.offer(i);
		}
		System.out.printf("\nAfter adding 10 more elements, queue has %d elements: ", queue.length);
		for (int i = 0; i < queue.length; i++) {
			System.out.printf("%c ", queue.get(i));
		}
		System.out.print('\n');
	}
}

/**
 * @author 没事啦 为支持队列的随机访问，自定义一个简单长度固定的队列的队列 内部用ArrayList实现来实现循环队列 规则：从队尾进，从队首出
 * @param <T> 数组元素类型不固定
 *
 */
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