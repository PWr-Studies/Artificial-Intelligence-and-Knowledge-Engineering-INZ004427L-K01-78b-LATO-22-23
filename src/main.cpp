#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <locale>
#include <iomanip>

//----------------------------------------------------------------
//---------------------CSV Pasrser--------------------------------
//----------------------------------------------------------------

class CSVRow
{
    public:
        std::string_view operator[](std::size_t index) const
        {
            return std::string_view(&m_line[m_data[index] + 1], m_data[index + 1] -  (m_data[index] + 1));
        }
        std::size_t size() const
        {
            return m_data.size() - 1;
        }
        void readNextRow(std::istream& str)
        {
            std::getline(str, m_line);

            m_data.clear();
            m_data.emplace_back(-1);
            std::string::size_type pos = 0;
            while((pos = m_line.find(',', pos)) != std::string::npos)
            {
                m_data.emplace_back(pos);
                ++pos;
            }
            // This checks for a trailing comma with no data after it.
            pos   = m_line.size();
            m_data.emplace_back(pos);
        }
    private:
        std::string         m_line;
        std::vector<int>    m_data;
};

std::istream& operator>>(std::istream& str, CSVRow& data)
{
    data.readNextRow(str);
    return str;
}   

class CSVIterator
{   
    public:
        typedef std::input_iterator_tag     iterator_category;
        typedef CSVRow                      value_type;
        typedef std::size_t                 difference_type;
        typedef CSVRow*                     pointer;
        typedef CSVRow&                     reference;

        CSVIterator(std::istream& str)  :m_str(str.good()?&str:nullptr) { ++(*this); }
        CSVIterator()                   :m_str(nullptr) {}

        // Pre Increment
        CSVIterator& operator++()               {if (m_str) { if (!((*m_str) >> m_row)){m_str = nullptr;}}return *this;}
        // Post increment
        CSVIterator operator++(int)             {CSVIterator    tmp(*this);++(*this);return tmp;}
        CSVRow const& operator*()   const       {return m_row;}
        CSVRow const* operator->()  const       {return &m_row;}

        bool operator==(CSVIterator const& rhs) {return ((this == &rhs) || ((this->m_str == nullptr) && (rhs.m_str == nullptr)));}
        bool operator!=(CSVIterator const& rhs) {return !((*this) == rhs);}
    private:
        std::istream*       m_str;
        CSVRow              m_row;
};

class CSVRange
{
    public:
        CSVRange(std::istream& str)
            : stream(str)
        {}
        CSVIterator begin() const {return CSVIterator{stream};}
        CSVIterator end()   const {return CSVIterator{};}
    private:
        std::istream&      stream;
};

//----------------------------------------------------------------
//---------------------Graph class--------------------------------
//----------------------------------------------------------------

struct EdgeDetails 
{
    long double cost = 0;
    std::tm departure_time = {};
    std::tm arrival_time = {};
};

class Node 
{
    public:
        typedef std::pair<EdgeDetails, Node*>   Edge;
        Node(const std::string _name, long double posX, long double posY) 
            :  x(posX), y(posY), name(_name) 
        {} 

        long double            x;
        long double            y; 
        std::string            name; 
        std::vector<Edge>      edges;
};

class Graph 
{
    public: 
        typedef std::map<std::string, Node*> NodeMap;

        void addEdge(const std::string& from, const std::string& to, const EdgeDetails details);
        Node* addNode(const std::string &name, long double posX, long double posY);

        void printEdges();
        void printNodes();
    private:
        NodeMap     nodes;
};

Node* Graph::addNode(const std::string &name, long double posX, long double posY)
{
    NodeMap::iterator itr = nodes.find(name);

    if (itr == nodes.end())
    {
        Node *v;
        v = new Node(name, posX, posY);
        nodes[name] = v;
        return v;
    }
    #ifdef DEBUG
    std::cout << "\nNode " << name << " already exists!" << std::endl;
    #endif

    return itr->second;
}

void Graph::addEdge(const std::string& from, const std::string& to, const EdgeDetails details)
{
    Node* f = nodes.find(from)->second;
    Node* t = nodes.find(to)->second;
    Node::Edge edge = std::make_pair(details, t);
    f->edges.push_back(edge);
}

void Graph::printEdges()
{
    for (auto& node : this->nodes) 
    {
        for (auto& edge : node.second->edges)
        {
            std::cout << node.second->name << " --> " << edge.second->name << std::endl;
        }
    }
}

void Graph::printNodes()
{
    for (auto& node : this->nodes)
    {
        std::cout << std::setprecision(8) << node.second->name << " ( " << node.second->x << ", " << node.second->y << " )" << std::endl;
    }
}

//----------------------------------------------------------------
//---------------------Algorithms---------------------------------
//----------------------------------------------------------------

void DijkstraAlgorithm(const std::string& source, const std::string& from, const Graph& graph) {
    
}


int main()
{
    std::ifstream file("resource/connection_graph.csv");

    Graph graph;

    for(auto& row: CSVRange(file))
    {   
        try
        {
            graph.addNode(std::string(row[6]).data(), 
                          std::stold(std::string(row[8]).data()), 
                          std::stold(std::string(row[9]).data()));


            graph.addNode(std::string(row[7]).data(), 
                          std::stold(std::string(row[10]).data()), 
                          std::stold(std::string(row[11]).data()));
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        bool error = false;
        EdgeDetails details;
        details.cost = 0;

        std::istringstream ssd(std::string(row[4]).data());
        ssd >> std::get_time(&details.arrival_time, "%H:%M:%S");
        if (ssd.fail())
        {
            std::cerr << "Parse to time failed! Value: " << std::string(row[4]).data() 
                      << " is not of type %H:%M:%S." << std::endl;
            error = true;
        }

        std::istringstream ssa(std::string(row[5]).data());
        ssa >> std::get_time(&details.departure_time, "%H:%M:%S");
        if (ssa.fail())
        {
            std::cout << "Parse to time failed! Value: " << std::string(row[5]).data() 
                      << " is not of type %H:%M:%S." << std::endl;
            error = true;
        }
        
        if (!error)
            graph.addEdge(std::string(row[6]).data(), std::string(row[7]).data(), details);
    }

    std::cout << "Graph created!" << std::endl;

    // graph.printEdges();
    graph.printNodes();
}