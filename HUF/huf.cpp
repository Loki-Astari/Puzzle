#include <cstddef>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <memory>

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
        ~Node()
        {
            if (left && !left->dynamic) {
                delete left;
            }
            if (right && !right->dynamic) {
                delete right;
            }
        }

        // Build a non leaf node
        Node(std::size_t cost, Node* left, Node* right)
            : cost(cost)
            , dynamic(true)
            , left(left)
            , right(right)
        {}

        // Used to read a Huffman tree dynamically from the head of a file.
        // The tree is serialized via exportTree() (see below)
        // The user is expected to check ok and eofMark are both true after is read
        Node(std::istream& str, std::size_t pSize, std::size_t pValue, bool& ok, bool& eofMark)
            : dynamic(true)
        {
            char x = '-';
            x = str.get();
            switch (x)
            {
                case 'N':
                    left    = new Node(str, pSize + 1, (pValue << 1) | 0x0, ok, eofMark);
                    right   = new Node(str, pSize + 1, (pValue << 1) | 0x1, ok, eofMark);
                    break;
                case 'C':
                    letter  = str.get();
                    size    = pSize;
                    value   = pValue;
                    break;
                case 'Z':
                    eof     = true;
                    eofMark = true;
                    break;
                default:
                    ok      = false;
            }
        }

        // Encode a Huffman tree to a stream.
        void exportTree(std::ostream& str)
        {
            if (left == nullptr && right == nullptr) {
                if (eof) {
                    str << "Z";
                }
                else {
                    str << "C" << letter;
                }
            }
            else {
                str << "N";
                left->exportTree(str);
                right->exportTree(str);
            }
        }

        // Once a Hoffman tree is built from scratch.
        // Call this function to calculate the representation of each letter.
        // Also calculate the size of the output file that will be generated.
        using DataSize = std::pair<std::size_t, std::size_t>;
        std::size_t setName()
        {
            DataSize size = setName(0, 0);

            // Size of the Huffman tree in bytes.
            std::size_t treeSize    = size.first;

            // size.second is the number of bits.
            // So Calculate the number of `std::size_t` objects needs to be written to the output stream.
            std::size_t streamCount = size.second / (sizeof(std::size_t) * 8);
            std::size_t streamRem   = size.second % (sizeof(std::size_t) * 8);
            std::size_t streamSize  = streamCount * sizeof(std::size_t) + (streamRem == 0 ? 0 : sizeof(std::size_t));

            // return the number of bytes that will be sent to the stream
            return treeSize + streamSize;
        }
    private:
        // Sets the value of each lead node and how it is represented.
        // Return data about the amount of data that will written to the output.
        //  first:  Number of bytes representing the Hoffman tree when encoding.
        //  second: Number of bits to represent the data of the file.
        DataSize setName(std::size_t pSize, std::size_t pValue)
        {
            if (left == nullptr && right == nullptr) {
                size = pSize;
                value = pValue;
                if (eof) {
                    return {1, pSize * cost};
                }
                else {
                    return {2, pSize * cost};
                }
            }
            else {
                DataSize l = left->setName( pSize + 1, (pValue << 1) | 0x0);
                DataSize r = right->setName(pSize + 1, (pValue << 1) | 0x1);

                return {l.first + r.first + 1, l.second + r.second};
            }
        }
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
        HuffmanEncoder()
        {
            for (int loop = 0; loop < 256; ++loop) {
                count[loop].letter = static_cast<unsigned char>(loop);
            }
            count[256].eof = true;
            count[256].cost = 1;
        }

        bool buildTree(std::istream& input)
        {
            // Count the number of each character in a file.
            std::size_t charCount = 0;
            for (unsigned char c = input.get(); input; c = input.get()) {
                ++count[c].cost;
                ++charCount;
            }
            if (charCount == 0) {
                std::cerr << "File is Empty!\n";
                return false;
            }

            // Now build the Hoffman tree.
            // Here we are dynamically allocating nodes without protection.
            // So we need to build it inside a try/catch block so if there
            // is an issue it will be correctly tidied up.
            std::priority_queue<Node*, std::vector<Node*>, std::function<bool(Node*, Node*)>>   p1{[](Node* l, Node* r){return l->cost > r->cost;}};
            try
            {
                for (int loop = 0; loop < 257; ++loop) {
                    if (count[loop].cost != 0) {
                        p1.emplace(&count[loop]);
                    }
                }
                while (p1.size() > 1) {
                    Node*   a = p1.top();p1.pop();
                    Node*   b = p1.top();p1.pop();

                    Node*   r = new Node{a->cost + b->cost, a, b};
                    p1.emplace(r);
                }
            }
            catch(...)
            {
                // There was an exception we need to handle cleanup.
                while (p1.size()) {
                    Node* n = p1.top();
                    p1.pop();
                    if (n && n->dynamic) {
                        delete n;
                    }
                }
                // Re-throw the exception
                throw;
            }

            // We now have the tree correctly rooted.
            root.reset(p1.top());

            // Calculate the representation of all the leaf nodes.
            std::size_t cost = root->setName();
            if (cost > charCount) {
                std::cerr << "Compesssion does not make it smaller\n";
                return false;
            }
            return true;
        }
        // Export the Hoffman tree to the file.
        void exportTree(std::ostream& out)
        {
            root->exportTree(out);
        }
        // using the Hoffman tree encode the input file to the output file.
        void encode(std::istream& in, std::ostream& out)
        {
            std::size_t const   maxSize     = sizeof(std::size_t) * 8;
            std::size_t         currentLen  = 0;
            std::size_t         currentVal  = 0;

            // Add each of the characters one at a time.
            for (unsigned char c = in.get(); in; c = in.get()) {
                add(out, count, maxSize, currentLen, currentVal, c);
            }
            // Add the EOF marker.
            add(out, count, maxSize, currentLen, currentVal, 256);

            // If there is data left in the currentValue
            // Then push that to the stream.
            if (currentLen != 0) {
                std::size_t shift = maxSize - currentLen;
                currentVal = currentVal << shift;
                out.write(reinterpret_cast<char*>(&currentVal), sizeof(currentVal));
            }
        }
    private:
        // Encode a sing character 'c'
        // If this fills the 'currentVal' object then write to the file.
        void add(std::ostream& out, Node* count, std::size_t const& maxSize, std::size_t& currentLen, std::size_t& currentVal, int c)
        {
            // Representation of the char 'c'
            Node& val = count[c];

            // The value that needs to be encoded.
            // This may fit into multiple values
            std::size_t writeLen = val.size;
            std::size_t writeVal = val.value;

            while (writeLen != 0) {
                // How much can we write into the current value.
                std::size_t outLen = std::min(writeLen, (maxSize - currentLen));

                // Add as many bits as we can from encoded 'c' into this value.
                currentVal = (currentVal << outLen) | (writeVal >> (writeLen - outLen));
                currentLen += outLen;

                // Remove what we have encode
                std::size_t mask = (1UL << (writeLen - outLen)) - 1;
                writeVal = writeVal & mask;
                writeLen -= outLen;

                // If we have filled up the current value write to the output file.
                if (currentLen == maxSize) {
                    out.write(reinterpret_cast<char*>(&currentVal), sizeof(currentVal));
                    currentVal = 0;
                    currentLen = 0;
                }
            }
        }
};

class HuffmanDecode: public Huffman
{
    private:
        std::unique_ptr<Node>   root;

    public:
        // Read the Huffman tree from the input stream.
        bool buildTree(std::istream& input)
        {
            bool                    ok      = true;
            bool                    eofMark = false;
            std::unique_ptr<Node>   result  = std::make_unique<Node>(input, 0, 0, ok, eofMark);
            if (!ok || !eofMark) {
                return false;
            }
            root = std::move(result);
            return true;
        }

        // Decode the input stream using the Hoffman stream place the output into out
        void decode(std::istream& in, std::ostream& out)
        {
            std::size_t const   maxSize     = sizeof(std::size_t) * 8;
            std::size_t const   maskBase    = (1UL << (maxSize - 1));

            Node*               current     = root.get();

            bool finished = false;

            while (!finished) {
                // Read the next vallue from the input stream.
                std::size_t     currentValue;
                in.read(reinterpret_cast<char*>(&currentValue), sizeof(currentValue));

                // Loop over each of the bit this allows us to follow the Hoffman
                // tree to a leaf node and then output the value of the lead node.
                for (std::size_t currentMask = maskBase; currentMask; currentMask >>= 1) {

                    // Get the next bit.
                    bool branch = currentValue & currentMask;
                    // Update the position in the Hoffman tree.
                    current     = branch ? current->right : current->left;

                    // If we are at a leaf node.
                    if (current->left == nullptr && current->right == nullptr) {
                        if (current->eof) {
                            finished = true;
                            break;
                        }
                        else {
                            out << current->letter;
                        }
                        // Reset the current point in the Hoffman tree to the root.
                        current = root.get();
                    }
                }
            }
        }
};

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: huf [+-] <filename>\n";
        return 1;
    }
    if (argv[1][0] != '+' && argv[1][0] != '-') {
        std::cerr << "Usage: huf [+-] <filename>\n";
        return 1;
    }
    std::ifstream   file(argv[2]);
    if (!file) {
        std::cerr << "File: " << argv[2] << " could not be opened\n";
        return 1;
    }

    if (argv[1][0] == '+') {
        HuffmanEncoder  encoder;
        if (encoder.buildTree(file)) {
            std::string     outName(std::string(argv[2]) + ".huf");
            std::ofstream   out(outName);
            if (!out) {
                std::cerr << "File: " << outName << " can not be opened for output\n";
                return 1;
            }
            file.clear();
            file.seekg(0);
            encoder.exportTree(out);
            encoder.encode(file, out);
            return 0;
        }
    }
    else {
        HuffmanDecode   decoder;
        if (decoder.buildTree(file)) {
            std::string     outName(std::string(argv[2]) + ".dec");
            std::ofstream   out(outName);
            if (!out) {
                std::cerr << "File: " << outName << " can not be opened for output\n";
                return 1;
            }
            decoder.decode(file, out);
            return 0;
        }
    }
    return 1;
}
