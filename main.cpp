#include <iostream>
#include <memory>
#include <cassert>

//this shows how hard implementing doubly linked list really is
//even with the help of unique pointer to handle ownership of pointers it is still (and maybe because of that)
//difficult not to screw things up.

//Two major challenges:
// 1. Dtor of LinkedList written naively causes stack overflow for big lists (tested for around 15000).
// 2. Pointer juggling in node removal has to be crafted really gently because of the possibility to screw things up
// (current node can be destroyed, and after it is done it shouldn't be used anymore, but still we have manage other pointers )

class LinkedList
{
public:
    LinkedList() : tail(nullptr) {}
    class Node {
        friend class LinkedList;
        friend std::ostream & operator<<(std::ostream &os, const LinkedList& p);
    public:
        static int instanceCount; //number of instances still alive
        ~Node() {instanceCount--;} // keep public so unique_ptr can access it

    private:
        Node(int i) : content(i), previous(nullptr) { instanceCount++;} //keep private so nobody outside can create it

        std::unique_ptr<Node> next;     //pointer to next node
        Node* previous;                 //pointer to previous node
        int content;                    //value of the node
    };

    //adds new node with value i to the end of the list
    void addToEnd(int i) {
        std::unique_ptr<Node> toAdd{new Node(i)};
        appendToEnd(std::move(toAdd));
    }

    void appendToEnd(std::unique_ptr<Node>&& toAdd) {
        if (tail) {
            tail->next = std::move(toAdd);
            tail->next->previous = tail;
            tail = tail->next.get();
        } else {
            tail = toAdd.get();
            head = std::move(toAdd);
        }
    }

    Node *find(int i) {
        Node* current = head.get();
        while(current && current->content != i)
        {
            current = current->next.get();
        }
        return current; //it will be null if it was not found or i
    }

    //adds new node with value i after node pNode
    void addToPos(Node *pNode, int i) {
        assert(!pNode || pNode->previous == nullptr || pNode->previous->next.get() == pNode);
        assert(!pNode || pNode->next == nullptr || pNode->next->previous == pNode);
        std::unique_ptr<Node> toAdd{new Node(i)};
        if(!pNode || !pNode->next)
            //if no node supplied or we are the last node
            appendToEnd(std::move(toAdd));
        else {
            //this is somewhat tricky code, maybe it is possible to code it a little bit simpler
            // pNode->next is the rest of the list (after the pNode), we want to keep it
            std::unique_ptr<Node> restOfList(std::move(pNode->next));
            toAdd->previous = pNode;
            restOfList->previous = toAdd.get();
            toAdd->next = std::move(restOfList);
            pNode->next = std::move(toAdd);
        }
    }
    void removeNode(Node* n) {
        assert(n != nullptr && "We don't allow nullptr here");
        assert(n->previous == nullptr || n->previous->next.get() == n);
        assert(n->next == nullptr || n->next->previous == n);

        if (n == head.get() && n == tail) {
            tail = nullptr;
            head.reset();
            return;
        } else if (n == head.get()) {
            head = std::move(n->next);
        } else if (n == tail) {
            tail = n->previous;
        }

        Node* x = n->next.get();
        Node* p = n->previous;

        if (p) { //we want to     p - n - x    to update p so it next points to to x
            p->next = std::move(n->next); //to still keep next of the elem to be removed n->next
        }

        if (x) {
            x->previous = p;
        }
    }
    ~LinkedList() {
        while(tail) {
            removeNode(tail);
        }
    }
    friend std::ostream & operator<<(std::ostream &os, const LinkedList& p);

private:
    std::unique_ptr<Node> head;
    Node* tail;
};
int LinkedList::Node::instanceCount = 0;

std::ostream & operator<<(std::ostream &os, const LinkedList& p)
{
    os << "<";
    LinkedList::Node* current = p.head.get();
    while(current) {
        os << current->content;
        os << ",";
        current = current->next.get();
    }
    os << ">";
    return os;
}


int main() {
    {
        LinkedList l;
        l.addToEnd(1);
        l.addToEnd(2);
        l.addToEnd(3);
        l.addToEnd(4);
        LinkedList::Node *n = l.find(2);
        l.addToPos(n, 7);
        std::cout << "test 1: should be <1,2,7,3,4> is" << l << std::endl;
    }
    {
        LinkedList l;
        LinkedList::Node *n = l.find(2);
        l.addToPos(n, 7);
        std::cout << "test 2: should be <7> is" << l << std::endl;
    }
    {
        LinkedList l;
        l.addToEnd(2);
        LinkedList::Node *n = l.find(2);
        l.addToPos(n, 7);
        std::cout << "test 3: should be <2,7> is" << l << std::endl;
    }
    {
        LinkedList l;
        l.addToEnd(3);
        LinkedList::Node *n = l.find(2);
        l.addToPos(n, 7);
        std::cout << "test 4: should be <3,7> is" << l << std::endl;
    }
    {
        //stack overflow scenario:
        LinkedList l;
        for(int i = 0; i < 15000; i++)
            l.addToEnd(i);
        //LinkedList::Node* n = l.find(400); //so the block won't be optimized out
        //l.addToPos(n, 5000);
        //std::cout << "Freaking huge list " << l << std::endl;
    }
    std::cout << "Instance count at the end of the test " << LinkedList::Node::instanceCount << std::endl;
    return 0;
}