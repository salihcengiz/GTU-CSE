//Start of the file Stack.java
public class Stack extends Container {

    @Override
    public void add(int value){
        if(size != data.length - 1)
            data[size++] = value;
    }

    private static int sCount;

    public static int getsCount() {return sCount;}

    public Stack() {size = 0; sCount++;}

    @Override
    public int remove() {
        return data[size--];
    }
    
    @Override
    public bool empty(){
        if (size == 0)
            return true;
        else
            return false;
    }
}
//End of the file Stack.java
