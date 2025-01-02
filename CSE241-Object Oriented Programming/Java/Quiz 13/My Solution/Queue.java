//Start of the file Queue.java
public class Queue extends Container{
    @Override
    public void add(int value){
        if(size != data.length - 1)
            data [size++] = value;
    }

    @Override
    public int remove(){
        return data [size--];
    }

    private static int qCount;

    public static int getQCount(){return qCount;} 

    public Queue() {size = 0; qCount++;}
    
    @Override
    public bool empty(){
        if (size == 0)
            return true;
        else
            return false;
    }
}
// End of the file Queue.java