#include <iostream>
#include <stack>

int main() {
    // Create a stack of integers
    std::stack<int> stack;

    // Add elements to the stack
    stack.push(10);
    stack.push(20);
    stack.push(30);

    // Access and print the top element
    std::cout << "Top element: " << stack.top() << std::endl;

    // Remove the top element
    stack.pop();

    // Print the new top element
    std::cout << "Top element after pop: " << stack.top() << std::endl;

    // Check if stack is empty
    if (!stack.empty()) {
        std::cout << "Stack is not empty. Size: " << stack.size() << std::endl;
    } else {
        std::cout << "Stack is empty." << std::endl;
    }

    return 0;
}
