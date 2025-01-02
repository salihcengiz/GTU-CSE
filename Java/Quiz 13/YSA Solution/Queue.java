// File: Queue.java
public class Queue extends Container {

    public Queue() {
        super();
		data = new int[INITSIZE];
		size = 0;
    }

    @Override
    public boolean empty() {
        return size == 0;
    }

    @Override
    public void add(int value) throws Exception {
//same as stack
    }

    @Override
    public int remove() throws Exception {
        if (!empty()) {
            int value = data[0];
            for (int i = 1; i < size; i++) {
                data[i - 1] = data[i];
            }
            size--;
            return value;
        } else {
            throw new Exception("Queue is empty!");
        }
    }
}