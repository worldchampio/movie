#pragma once

#include <sstream>

template<class T>
struct Node
{
    T value{};
    Node<T>* next{nullptr};
    Node(const T& val) : value{val} {}
    constexpr std::size_t sizeReq() { return sizeof(Node<T>*); }
};

template<class T>
class List
{
    std::size_t m_allocated{0};
    std::size_t m_freed{0};
    std::size_t m_size{0};

    Node<T>* m_head{nullptr};
public:
    List(const T& rootValue)
    {
        m_head = new Node{rootValue};
        m_allocated += m_head->sizeReq();
        ++m_size;
    }
    ~List() 
    {
        clear();
    }

    void append(const T& value) 
    {
        auto newNode{new Node{value}};
        m_allocated+=newNode->sizeReq();
        ++m_size;
        auto temp{m_head};
        while(temp->next) { temp = temp->next; }
        temp->next = newNode;
    }

    bool remove(int n) 
    {
        if(n==0 && m_size==1)
        {
            m_freed+=m_head->sizeReq();
            --m_size;
            delete m_head;
            m_head = nullptr;
            return true;
        }
        if(n<0 || n>=m_size) 
            return false;
        auto temp{m_head};
        Node<T>* prev{nullptr};
        int count{0};
        while(temp)
        {
            if(n==count)
                break;
            prev = temp;
            temp = temp->next;
            ++count;
        }
        if(!temp)
            return false;

        if(temp==m_head && temp->next) // update head when deleting current head
            m_head = temp->next;
  
        if(prev && temp) // connect previous and next node
            prev->next = temp->next;

        m_freed+=temp->sizeReq();
        --m_size;
        delete temp;
        return true;
    }

    void clear()
    {
        auto temp{m_head};
        while(temp)
        {
            auto toDelete = temp;
            --m_size;
            m_freed+=toDelete->sizeReq();
            temp = temp->next;
            delete toDelete;
            toDelete = nullptr;
        }
        m_head = nullptr;
    }

    std::size_t size() { return m_size; }

    void print(std::ostream& os) const
    {

        constexpr auto Bar{"+------------------------------+"};
        os << Bar <<std::endl;
        os <<memory()<< "PRINT\tSize: "<<m_size<<", Type: "<<Utils::typeName(T{})<<std::endl;
        auto curr{m_head};
        int count{0};
        while(curr)
        {
            const std::string ind(count,' ');
            if(curr != m_head)
                os << "\n" <<ind<<"|\n"<<ind << "+->";
            os << str(curr,count);
            ++count;
            curr = curr->next;
        }
        os << std::endl;
        os << Bar <<std::endl;
    }
private:
    std::string memory() const
    {
        std::stringstream ss;
        ss << "MEMORY\t"
            <<"Alloc: " << Utils::storage(m_allocated) 
            <<", Freed: " << Utils::storage(m_freed) 
            << ", Leak: " << Utils::storage(m_allocated-m_freed) <<std::endl;
        return ss.str();
    }
    
    std::string str(Node<T>* node, int n) const
    {
        std::stringstream ss;
        if(!node)
            ss << "Null";
        else if(node==m_head) 
            ss << "Head";
        else if(!node->next) 
            ss << "Tail";
        else 
            ss << "Node"<< n;
        
        ss << "[val = " << node->value << "]";
        return ss.str();
    }
};