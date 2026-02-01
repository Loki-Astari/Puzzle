#ifndef THORSANVIL_PUZZLE_HUFFMAN_H
#define THORSANVIL_PUZZLE_HUFFMAN_H

#include <cstddef>
#include <iostream>
#include <utility>
#include <queue>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>


namespace ThorsAnvil::Puzzle
{

class Huffman
{
    protected:
    struct Node
    {
        std::size_t     cost        = 0;
        bool            dynamic     = false;
        // Leaf Nodes
        // These will have either eof set to true or a letter value.
        bool            eof         = false;
        unsigned char   letter      = '\0';
        // Leaf nodes we calculate the representation of the leaf in the file.
        // The size is the number of bits needed.
        // The value (which fits in size bits) that represents this leaf node when
        // it is serialized into a file.
        std::size_t     size        = 0;
        std::size_t     value       = 0;
        // Non-Leaf nodes (don't use eof/letter)
        // Expected to have both left and right value;
        Node*           left        = nullptr;
        Node*           right       = nullptr;

        // Default constructor used to create automatic storage duration objects.
        // This is used in the code below to create an array of Node objects.
        // Note: This class is a private class that can only be used in
        //       HuffmanEncoder and HuffmanDecoder and it thus this restriction is observed.
        Node() = default;

        // This deletes a Huffman tree.
        // Note the encoder builds the tree using Nodes from an array.
        //      The nodes in the array are not dynamically allocated and thus should not
        //      be deleted.
        ~Node();

        // Build a non leaf node
        Node(std::size_t cost, Node* left, Node* right);

        // Used to read a Huffman tree dynamically from the head of a file.
        // The tree is serialized via exportTree() (see below)
        // The user is expected to check ok and eofMark are both true after is read
        Node(std::istream& str, std::size_t pSize, std::size_t pValue, bool& ok, bool& eofMark);

        // Encode a Huffman tree to a stream.
        void exportTree(std::ostream& str);

        // Once a Hoffman tree is built from scratch.
        // Call this function to calculate the representation of each letter.
        // Also calculate the size of the output file that will be generated.
        std::size_t setName();

        private:
            // Sets the value of each lead node and how it is represented.
            // Return data about the amount of data that will written to the output.
            //  first:  Number of bytes representing the Hoffman tree when encoding.
            //  second: Number of bits to represent the data of the file.
            using DataSize = std::pair<std::size_t, std::size_t>;
            DataSize setName(std::size_t pSize, std::size_t pValue);
    };
};

class HuffmanEncoder: public Huffman
{
    private:
        // Nodes[0-255] represent the ASCII char set.
        // Nodes[256]   represents the EOF character.
        // The constructor initializes this array correctly.
        Node                    count[257];

        // The root of a Hoffman tree.
        std::unique_ptr<Node>   root;

    public:
        HuffmanEncoder();

        bool buildTree(std::istream& input);

        // Export the Hoffman tree to the file.
        void exportTree(std::ostream& out);

        // using the Hoffman tree encode the input file to the output file.
        void encode(std::istream& in, std::ostream& out);

    private:
        // Encode a sing character 'c'
        // If this fills the 'currentVal' object then write to the file.
        void add(std::ostream& out, Node* count, std::uint64_t const& maxSize, std::uint64_t& currentLen, std::uint64_t& currentVal, int c);
};

class HuffmanDecoder: public Huffman
{
    private:
        std::unique_ptr<Node>   root;

    public:
        // Read the Huffman tree from the input stream.
        bool buildTree(std::istream& input);

        // Decode the input stream using the Hoffman stream place the output into out
        void decode(std::istream& in, std::ostream& out);
};

}

#endif
