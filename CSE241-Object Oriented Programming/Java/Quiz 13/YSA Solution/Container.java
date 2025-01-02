public abstract class Container {
    protected int[] data;
    protected int size;
	protected final int INITSIZE = 100;

    public Container() {
    }

    public abstract boolean empty();

    public abstract void add(int value);

    public abstract int remove();
}