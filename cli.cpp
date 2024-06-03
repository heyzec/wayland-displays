#include <iostream>
#include <string>

void printUsage() {
    std::cout << "Usage: hello [name]" << std::endl;
    std::cout << "  name: Your name or any string to greet (optional)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        // No arguments provided, print usage
        printUsage();
        return 1; // Exit with error code 1
    }

    // Extract the name from the command-line arguments
    std::string name = argv[1];

    // Print the greeting
    std::cout << "Hello, " << name << "!" << std::endl;

    return 0; // Exit with success code
}

