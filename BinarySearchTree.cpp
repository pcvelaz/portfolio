//============================================================================
// Name        : Project Two.cpp
// Author      : Paul Velazquez
//============================================================================

#include <iostream>
#include <time.h>
#include <fstream>
#include "CSVparser.hpp"

using namespace std;

//===================
// Global definitions
//===================

// forward declarations
double strToDouble(string str, char ch);

// define a structure to hold course information
struct Course {
    string courseNum; // unique identifier
    string courseName;
    vector<string> prereqs;
    Course() {
    }
};

// Internal structure for tree node
struct Node {
    Course course;
    Node *left;
    Node *right;

    // default constructor
    Node() {
        left = nullptr;
        right = nullptr;
    }

    // initialize with a course
    Node(Course aCourse) :
            Node() {
        course = aCourse;
    }
};

// Binary Search Tree class definition

class BinarySearchTree {

private:
    Node* root;

    void addNode(Node* node, Course course);
    void inOrder(Node* node);
    Node* removeNode(Node* node, string courseNum);

public:
    BinarySearchTree();
    virtual ~BinarySearchTree();
    void InOrder();
    void Insert(Course course);
    void Remove(string courseNum);
    Course Search(string courseNum);
};

// Default constructor
 
BinarySearchTree::BinarySearchTree() {
    root = nullptr;
}

// Destructor
BinarySearchTree::~BinarySearchTree() {
    // recurse from root deleting every node
}

// Traverse the tree in order
 
void BinarySearchTree::InOrder() {
    // call inOrder fuction and pass root 
    inOrder(root);
}



// Insert a course
 
void BinarySearchTree::Insert(Course course) {
    // if root equarl to null ptr
    if (root == nullptr) {
        // root is equal to new node course
        root = new Node(course);
    }
    else {
        // add Node root and course
        this->addNode(root, course);
    
    }
}

// Remove a course
 
void BinarySearchTree::Remove(string courseNum) {
    // remove node root course number
    this->removeNode(root, courseNum);
}

// Search for a course
 
Course BinarySearchTree::Search(string courseNum) {
    // set current node equal to root
    Node* current = root;

    // keep looping downwards until bottom reached or matching courseNum found
    while (current != nullptr) {
        // if match found, return current course
        if (current->course.courseNum.compare(courseNum) == 0) {
            return current->course;
        }
        // if course is smaller than current node then traverse left
        if (courseNum.compare(current->course.courseNum) < 0) {
            current = current->left;
        }
        else {
            // else larger so traverse right
            current = current->right;
        }
    }

    Course course;
    return course;
}

// Add a course to a node
void BinarySearchTree::addNode(Node* node, Course course) {
    // if node is larger then add to left
    if (node->course.courseNum.compare(course.courseNum) > 0) {
        // if no left node
        if (node->left == nullptr) {
            // this node becomes left
            node->left = new Node(course);
        }
        else {
            // else recurse down the left node
            this->addNode(node->left, course);
        }
    }
    else {
        // if no right node
        if (node->right == nullptr) {
            // this node becomes right
            node->right = new Node(course);
        }
        else {
            // recurse down the left node
            this->addNode(node->right, course);
        }
    }
}
void BinarySearchTree::inOrder(Node* node) {
      //if node is not equal to null ptr
    if (node != nullptr) {
        //InOrder left
        inOrder(node->left);
        //output course number, course name
        cout << node->course.courseNum << ":  "
            << node->course.courseName << "   "
            << "Prerequisites: ";
        if (node->course.prereqs.size() == 0) {
            cout << "None" << endl;
        }
        else {
            for (int i = 0; i < node->course.prereqs.size(); i++) {
                cout << node->course.prereqs[i] << " ";
            }
            cout << endl;
        }
        //InOrder right
        inOrder(node->right);
      }
}

Node* BinarySearchTree::removeNode(Node* node, string courseNum) {
    // If node is null then tree is empty, so return
    if (node == nullptr) {
        return nullptr;
    }
    // if node is to the left, traverse down left side
    if (courseNum.compare(node->course.courseNum) < 0) {
        node->left = removeNode(node->left, courseNum);
    }
    // if node is to the right, traverse down right side
    else if (courseNum.compare(node->course.courseNum) >0) {
        node->right = removeNode(node->right, courseNum);
    }
    else {
        // if no children just delete node
        if (node->left == nullptr && node->right == nullptr) {
            delete node;
            node = nullptr;
        }
        // if child to the left
        else if (node->left != nullptr && node->right == nullptr) {
            Node* temp = node;
            node = node->left;
            delete temp;
        }
        // if child to the right
        else if (node->left == nullptr && node->right != nullptr) {
            Node* temp = node;
            node = node->right;
            delete temp;
        }
        // two children
        else {
            Node* temp = node->right;
            while (temp->left != nullptr) {
                temp = temp->left;
            }
            node->course = temp->course;
            node->right = removeNode(node->right, temp->course.courseNum);
        }
    }
}



// Display course information

void displayCourse(Course course) {
    cout << course.courseNum << ": " << course.courseName << "  " << endl;
    cout << "Prerequisites: ";
    if (course.prereqs.size() == 0) {
        cout << "No prerequisites" << endl;
    }
    else {
        for (int i = 0; i < course.prereqs.size(); i++) {
            cout << course.prereqs[i] << " ";
        }
        cout << endl;
    }
    return;
}

// Load courses from the CSV
void loadCourses(string csvPath, BinarySearchTree* bst) {

        cout << "Loading courses... " << endl;

        string fname = "ABCU_Advising_Program_Input.csv";
        vector<vector<string>> content;
        vector<string> row;
        string line, word;

        fstream file(fname, ios::in);
        if (file.is_open())
        {
            while (getline(file, line))
            {
                row.clear();

                stringstream str(line);

                while (getline(str, word, ','))
                    row.push_back(word);
                content.push_back(row);
            }
        }

        for (int i = 0; i < content.size(); i++) {
            Course course;
            course.courseNum = content[i][0];
            course.courseName = content[i][1];
            if (content[i].size() >= 3) {
                course.prereqs.push_back(content[i][2]);
                if (content[i].size() == 4) {
                    course.prereqs.push_back(content[i][3]);
                }
            }

            // push this course to the end
            bst->Insert(course);
        } 
}

/**
 * Simple C function to convert a string to a double
 * after stripping out unwanted char
 *
 * credit: http://stackoverflow.com/a/24875936
 *
 * @param ch The character to strip out
 */
double strToDouble(string str, char ch) {
    str.erase(remove(str.begin(), str.end(), ch), str.end());
    return atof(str.c_str());
}

int main(int argc, char* argv[]) {

    // process command line arguments
    string csvPath, courseKey;
    switch (argc) {
    case 2:
        csvPath = argv[1];
        break;
    case 3:
        csvPath = argv[1];
        courseKey = argv[2];
        break;
    default:
        csvPath = "ABCU_Advising_Program_Input.csv";
        
    }


    // Define a binary search tree to hold all courses
    BinarySearchTree* bst;
    bst = new BinarySearchTree();
    Course course;

    int choice = 0;
    while (choice != 9) {
        cout << "Menu:" << endl;
        cout << "  1. Load Courses" << endl;
        cout << "  2. Display All Courses" << endl;
        cout << "  3. Find Course" << endl;
        cout << "  9. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {

        case 1:
            // Complete the method call to load the courses
            loadCourses(csvPath, bst);

            break;

        case 2:
            bst->InOrder();
            break;

        case 3:
            cout << "Enter course number for the course: " << endl;
            cin >> courseKey;
            course = bst->Search(courseKey);

            if (!course.courseNum.empty()) {
                displayCourse(course);
            } else {
            	cout << "Course number " << courseKey << " not found." << endl;
            }

            break;

        }
    }

    cout << "Good bye." << endl;

	return 0;
}
