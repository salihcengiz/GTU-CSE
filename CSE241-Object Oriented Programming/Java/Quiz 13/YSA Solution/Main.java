public class Main {
    public static void main(String[] args) {
        try {
            // Create an array of Container references
            Container[] containers = new Container[4];

            // Add Stack and Queue objects
            containers[0] = new Stack();
            containers[1] = new Stack();
            containers[2] = new Queue();
            containers[3] = new Queue();

            // Add and remove elements
/*             containers[0].add(10); // Add to Stack
            containers[1].add(20); // Add to another Stack
            containers[2].add(30); // Add to Queue
            containers[3].add(40); // Add to another Queue */
			
			for(var c : containers)
				c.add(123);

            System.out.println(containers[0].remove()); // Should print 10 (Stack)
            System.out.println(containers[1].remove()); // Should print 20 (Stack)
            System.out.println(containers[2].remove()); // Should print 30 (Queue)
            System.out.println(containers[3].remove()); // Should print 40 (Queue)
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }
}
