#include <cstddef>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>

struct Node {
    std::size_t     cost        = 0;
    bool            eof         = false;
    bool            leaf        = true;
    unsigned char   letter      = '\0';
    Node*           left        = nullptr;
    Node*           right       = nullptr;
    std::string     name;
    std::size_t     size        = 0;
    std::size_t     value       = 0;

    static Node* build(std::istream& str)
    {
        return new Node(str, "", 0, 0);
    }

    Node(std::size_t cost = 0, bool eof = false, bool leaf = true, unsigned char letter = '\0', Node* left = nullptr, Node* right = nullptr, std::string const& name = "", std::size_t size = 0, std::size_t value = 0)
        : cost(cost)
        , eof(eof)
        , leaf(leaf)
        , letter(letter)
        , left(left)
        , right(right)
        , name(name)
        , size(size)
        , value(value)
    {}
    Node(std::istream& str, std::string const& path, std::size_t pSize, std::size_t pValue)
    {
        char x = str.get();
        if (x == 'N') {
            left  = new Node(str, path + "0", pSize + 1, (pValue << 1) | 0x0);
            right = new Node(str, path + "1", pSize + 1, (pValue << 1) | 0x1);
            return;
        }
        if (x == 'C') {
            letter = str.get();
            name = path;
            size = pSize;
            value = pValue;
            return;
        }
        if (x == 'Z') {
            eof = true;
            return;
        }
    }

    std::size_t setName(std::ostream& str, std::string const& path = "", std::size_t pSize = 0, std::size_t pValue = 0)
    {
        if (left == nullptr && right == nullptr) {
            name = path;
            size = pSize;
            value = pValue;
            if (eof) {
                str << "Z";
            }
            else {
                str << "C" << letter;
            }

            return path.size() * cost;
        }
        str << "N";
        std::size_t result = 0;
        result += left->setName(str, path + "0",  pSize + 1, (pValue << 1) | 0x0);
        result += right->setName(str, path + "1", pSize + 1, (pValue << 1) | 0x1);
        return result;
    }

    ~Node()
    {
        if (left && !left->leaf) {
            delete left;
        }
        if (right && !right->leaf) {
            delete right;
        }
    }
};

bool encode(std::istream& in, std::ostream& out);
bool decode(std::istream& in, std::ostream& out);
void add(std::ostream& out, Node* count, std::size_t const& maxSize, std::size_t& currentLen, std::size_t& currentVal, int c);

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
    std::string     outName(std::string(argv[2]) + (argv[1][0] == '+' ? ".huf" : ".dec"));
    std::ofstream   out(outName);
    if (!out) {
        std::cerr << "File: " << outName << " can not be opened for output\n";
        return 1;
    }

    bool ok = true;
    if (argv[1][0] == '+') {
        ok = encode(file, out);
    }
    else {
        ok = decode(file, out);
    }
    return ok ? 0 : 1;
}

bool encode(std::istream& in, std::ostream& out)
{
    Node count[257];
    for (int loop = 0; loop < 256; ++loop) {
        count[loop].letter = static_cast<unsigned char>(loop);
    }
    count[256].eof = true;
    count[256].cost = 1;

    std::size_t fileCount = 0;
    for (unsigned char c = in.get(); in; c = in.get()) {
        ++count[c].cost;
        ++fileCount;
    }
    if (fileCount == 0) {
        std::cerr << "File is Empty!\n";
        return false;
    }


    std::priority_queue<Node*, std::vector<Node*>, std::function<bool(Node*, Node*)>>   p1{[](Node* l, Node* r){return l->cost > r->cost;}};
    for (int loop = 0; loop < 257; ++loop) {
        if (count[loop].cost != 0) {
            p1.emplace(&count[loop]);
        }
    }
    while (p1.size() > 1) {
        Node*   a = p1.top();p1.pop();
        Node*   b = p1.top();p1.pop();

        Node*   r = new Node{a->cost + b->cost, false, false, '\0', a, b, "", 0, 0};
        p1.emplace(r);
    }

    std::unique_ptr<Node> v{p1.top()};

    std::size_t cost = v->setName(out);
    if (cost > (fileCount * 8)) {
        std::cerr << "Compesssion does not make it smaller\n";
        return false;
    }
    in.clear();
    in.seekg(0);
    std::size_t const   maxSize     = sizeof(std::size_t) * 8;
    std::size_t         currentLen  = 0;
    std::size_t         currentVal  = 0;
    for (unsigned char c = in.get(); in; c = in.get()) {
        add(out, count, maxSize, currentLen, currentVal, c);
    }
    add(out, count, maxSize, currentLen, currentVal, 256);

    if (currentLen != 0) {
        std::size_t shift = maxSize - currentLen;
        currentVal = currentVal << shift;
        out.write(reinterpret_cast<char*>(&currentVal), sizeof(currentVal));
    }
    return true;
}

void add(std::ostream& out, Node* count, std::size_t const& maxSize, std::size_t& currentLen, std::size_t& currentVal, int c)
{
    Node& val = count[c];

    std::size_t writeLen = val.size;
    std::size_t writeVal = val.value;

    while (writeLen != 0) {
        std::size_t outLen = std::min(writeLen, (maxSize - currentLen));

        currentVal = (currentVal << outLen) | (writeVal >> (writeLen - outLen));
        currentLen += outLen;

        std::size_t mask = (1UL << (writeLen - outLen)) - 1;
        writeVal = writeVal & mask;
        writeLen -= outLen;

        if (currentLen == maxSize) {
            out.write(reinterpret_cast<char*>(&currentVal), sizeof(currentVal));
            currentVal = 0;
            currentLen = 0;
        }
    }
}

bool decode(std::istream& in, std::ostream& out)
{
    std::unique_ptr<Node> r{Node::build(in)};

    std::size_t const   maxSize     = sizeof(std::size_t) * 8;
    std::size_t const   maskBase    = (1UL << (maxSize - 1));
    std::size_t         currentMask = maskBase;
    std::size_t         currentValue;

    Node*               current     = r.get();

    bool finished = false;
    while (!finished) {

        in.read(reinterpret_cast<char*>(&currentValue), sizeof(currentValue));

        while (currentMask) {

            bool branch = currentValue & currentMask;
            current     = branch ? current->right : current->left;
            currentMask = currentMask >> 1;

            if (current->left == nullptr && current->right == nullptr) {
                if (current->eof) {
                    finished = true;
                    break;
                }
                else {
                    out << current->letter;
                }
                current = r.get();
            }
        }
        currentMask = maskBase;
    }
    return true;
}
