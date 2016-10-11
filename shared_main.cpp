#include <iostream>
#include <memory>

class BinarySearchTree {
public:
    class Node {
        friend class BinarySearchTree;

        friend void release(std::shared_ptr<Node>, unsigned int n);

        friend std::ostream &operator<<(std::ostream &os, const BinarySearchTree &p);

    public:
        static int instanceCount; //number of instances still alive
        ~Node() { instanceCount--; } // keep public so unique_ptr can access it

    private:
        Node(int i) : content(i) { instanceCount++; } //keep private so nobody outside can create it

        std::shared_ptr<Node> left;     //pointer to left node
        std::shared_ptr<Node> right;     //pointer to right node
        std::weak_ptr<Node> parent;                 //pointer to previous node
        int content;                    //value of the node
    };

    BinarySearchTree() : size(0) {}

    ~BinarySearchTree() {
        if (root) {
            release(root, size);
            root.reset();
        }
    }

    //adds new node with value i to the end of the list
    void addToEnd(int i) {
        std::shared_ptr<Node> toAdd{new Node(i)};
        appendToEnd(std::move(toAdd));
    }

    void appendToEnd(std::shared_ptr<Node> &&toAdd) {
        std::shared_ptr<Node> current = root;
        if (!root) {
            root = toAdd;
            size++;
            return;
        }
        std::shared_ptr<Node> previous;

        while (current) {
            if (toAdd->content < current->content) {
                previous = current;
                current = current->left;
            }
            else if (toAdd->content > current->content) {
                previous = current;
                current = current->right;
            }
            else
                break;
        }
        if (!current && previous) {
            if (toAdd->content < previous->content) {
                previous->left = toAdd;
            }
            else if (toAdd->content > previous->content) {
                previous->right = toAdd;
            }
            toAdd->parent = previous;
            size++;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const BinarySearchTree &p);

private:
    std::shared_ptr<Node> root;
    unsigned int size;
};

int BinarySearchTree::Node::instanceCount = 0;

std::ostream &operator<<(std::ostream &os, const BinarySearchTree &p) {
    os << "<";
    std::shared_ptr<BinarySearchTree::Node> current = p.root;

    unsigned int processed = 0;
    bool fromUp = true;
    bool comingFromLeft = false;
    while (processed != p.size) {
        if (fromUp) {
            if (!current->left) {
                os << current->content << ",";
                processed++;
                if (current->right) {
                    current = current->right;
                } else {
                    fromUp = false;
                    if (std::shared_ptr<BinarySearchTree::Node> parent = current->parent.lock()) { //if we are root we don't have parent so we have to be careful
                        comingFromLeft = parent->left == current;
                        current = parent;
                    }
                }
            } else {
                current = current->left;
            }
        } else {
            if (comingFromLeft) {
                os << current->content << ",";
                processed++;
                if (current->right) {
                    current = current->right;
                    fromUp = true;
                } else {
                    if (std::shared_ptr<BinarySearchTree::Node> parent = current->parent.lock()) { //if we are root we don't have parent so we have to be careful
                        comingFromLeft = parent->left == current;
                        current = parent;
                    }
                }
            } else {
                if (std::shared_ptr<BinarySearchTree::Node> parent = current->parent.lock()) { //if we are root we don't have parent so we have to be careful
                    comingFromLeft = parent->left == current;
                    current = parent;
                }
            }
        }
    }
    os << ">";
    return os;
}

void release(std::shared_ptr<BinarySearchTree::Node> current, unsigned int size) {
    unsigned int processed = 0;
    bool fromUp = true;
    bool comingFromLeft = false;
    while (processed != size - 1) { //cannot remove itself
        if (fromUp) {
            if (!current->left) {
                if (current->right) {
                    current = current->right;
                } else {
                    fromUp = false;
                    if (std::shared_ptr<BinarySearchTree::Node> parent = current->parent.lock()) { //if we are root we don't have parent so we have to be careful
                        comingFromLeft = parent->left == current;
                        current = parent;
                    }
                }
            } else {
                current = current->left;
            }
        } else {
            if (comingFromLeft) {
                if (current->right) {
                    current = current->right;
                    fromUp = true;
                } else {
                    current->left.reset();
                    processed++;

                    if (std::shared_ptr<BinarySearchTree::Node> parent = current->parent.lock()) { //if we are root we don't have parent so we have to be careful
                        comingFromLeft = parent->left == current;
                        current = parent;
                    }
                }
            } else {
                current->right.reset();
                processed++;

                if (std::shared_ptr<BinarySearchTree::Node> parent = current->parent.lock()) { //if we are root we don't have parent so we have to be careful
                    comingFromLeft = parent->left == current;
                    current = parent;
                }
            }
        }
    }
}

int main() {
    {
        BinarySearchTree l;
        l.addToEnd(1);
        l.addToEnd(2);
        l.addToEnd(3);
        l.addToEnd(4);
        std::cout << "test 1: should be <1,2,3,4> is" << l << std::endl;
    }
    {
        BinarySearchTree l;
        l.addToEnd(6);
        l.addToEnd(2);
        l.addToEnd(8);
        l.addToEnd(7);
        l.addToEnd(9);
        l.addToEnd(4);
        l.addToEnd(1);
        l.addToEnd(3);
        l.addToEnd(5);
        std::cout << "test 1: should be <1,2,3,4,5,6,7,8,9> is" << l << std::endl;
    }
    {
        //stack overflow scenario:
        BinarySearchTree l;
        for (int i = 0; i < 15000; i++)
            l.addToEnd(i);
    }
    std::cout << "Instance count at the end of the test " << BinarySearchTree::Node::instanceCount << std::endl;
    return 0;
}