// File: Stack.java
public class Stack extends Container {

    public Stack() {
        super();
		data = new int[INITSIZE];
		size = 0;
    }

    @Override
    public boolean empty() {
        return size == 0;
    }

    @Override
    public void add(int value) {
        if (size < data.length) {
            data[size] = value;
            size++;
        } else {
            int[] temp = new int[data.lenght * 2];
			for(int i = 0; i < data.lenght; ++i)
				temp[i] = data[i];
			data = temp;
			data[size++] = value;
        }
    }

    @Override
    public int remove() {
        if (!empty()) {
            int value = data[size - 1];
            size--;
            return value;
        } else {
            throw new Exception("error");
            return -1; // Indicates an error
        }
    }
}