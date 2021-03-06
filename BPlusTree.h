#ifndef B_PLUS_TREE
#define B_PLUS_TREE

#include <vector>
#include <iostream>

int ORDER;

template<typename T>
// ahora es un B+
class btree {
private:

    enum state_t { OVERFLOW, UNDERFLOW, B_OK };

    struct node {
        std::vector<T> data;
        std::vector<node*> children;
        size_t count{0};
        bool isLeaf{0};
        node* next{0};
        node* prev{0};
        
        node() : data(ORDER+1), children(ORDER+2, nullptr)
        {

        }

        void insert_into(size_t index, const T& value) {
            size_t j = this->count;
            while (j > index) {
                children[j+1] = children[j];
                data[j] = data[j-1];
                j--;
            }
            children[j+1] = children[j];
            data[j] = value;
            this->count++;
        }

        void push_back(const T& value) {
            insert_into(this->count, value);
        }

        state_t insert(const T& value) {
            // binary_search
            size_t index = 0;
            while (this->data[index] < value  && index < this->count) {
                index += 1; 
            }
            if (this->children[index] == nullptr) {
                // this is a leaf node
                this->insert_into(index, value);
            } else {
                auto state = this->children[index]->insert(value);
                if (state == state_t::OVERFLOW) {
                    // split 
                    this->split(index);
                }
            }
            return this->count > ORDER ? OVERFLOW : B_OK;
        }

        void remove(const T& value) {
            
        }

        void split(size_t position) {
            // leaf nodes / index nodes

            node* parent = this; 
            node* ptr = this->children[position];

            node* ptr_next = ptr->next;
            node* ptr_prev = ptr->prev;
            // 'ptr' es el nodo que va a ser spliteado

            // TODO: reuse ptr buffer 
            node* child1 = new node();
            node* child2 = new node();

            if (ptr->isLeaf) {
                child1->isLeaf = true;
                child2->isLeaf = true;
            }

            size_t i = 0;
            for (; i < ptr->count / 2; i++) { // cambie aca
                child1->children[i] = ptr->children[i];
                child1->data[i] = ptr->data[i];
                child1->count++;
            }
            child1->children[i] = ptr->children[i];

            size_t mid = i;
            i += 1; 
            size_t j = 0;
            // B+
            if (ptr->isLeaf) {
                child2->data[j] = ptr->data[mid];
                child2->count++;
                ++j;
            }
            // B+
            for (; i < ptr->count; i++) {
                child2->children[j] = ptr->children[i];
                child2->data[j] = ptr->data[i];
                child2->count++;
                j++;
            }
            child2->children[j] = ptr->children[i];

            // update dll pointers
            child1->next = child2;
            child1->prev = ptr_prev;
            child2->next = ptr_next;
            child2->prev = child1;

            if (ptr_next) {
                ptr_next->prev = child2;
            }
            if (ptr_prev) {
                ptr_prev->next = child1;
            }
            
            parent->insert_into(position, ptr->data[mid]);
            parent->children[position] = child1;
            parent->children[position + 1] = child2;
        }

        void merge(size_t index){
            node *child = this->children[index];
            node *sibling = this->children[index+1];
            if(sibling!= nullptr){
                if(data[index] != sibling->data[0]){
                    child->data[child->count-1] = data[index];
                    for (int i=0; i<sibling->count; ++i)
                        child->data[i+child->count] = sibling->data[i];

                    if (!child->isLeaf) {
                        for(int i=0; i<sibling->count+1; ++i)
                            child->children[i+child->count] = sibling->children[i];
                    }

                    for (int i=index+1; i<count; ++i)
                        data[i-1] = data[i];


                    for (int i=index+2; i<=count; ++i)
                        children[i-1] = children[i];

                    child->count += sibling->count;
                    count--;
                }else{

                    for (int i=0; i<sibling->count; ++i)
                        child->data[i+child->count-1] = sibling->data[i];

                    if (!child->isLeaf) {
                        for(int i=0; i<sibling->count+1; ++i)
                            child->children[i+child->count-1] = sibling->children[i];
                    }

                    for (int i=index+1; i<count; ++i)
                        data[i-1] = data[i];


                    for (int i=index+2; i<=count; ++i)
                        children[i-1] = children[i];

                    child->count += sibling->count-1;
                    count--;
                }
            }else{
                std::cout << "derecho";
            }



            delete(sibling);
        } 
    };

public:

    btree() {
        root.isLeaf = true;
        head = &root;
    }
    
    void insert(const T& value) {
        auto state = root.insert(value);
        if (state == state_t::OVERFLOW) {
            // split root node
            split_root();
        }
    }

    void remove(const T& value) {
        iterator target_node = find(value);
        if(target_node.ptr == nullptr) {
            return; 
        }

        size_t keys_count = target_node.ptr->count;
        int index = target_node.index;
        auto parent = target_node.parent;

        if (keys_count > (size_t)ORDER/2) {
            if (value == target_node.ptr->data[0]) {
                int parent_key_count = target_node.parent->count;
                for (int i = 0; i < parent_key_count; ++i) {
                    if (target_node.parent->data[i] == value) {
                        target_node.parent->data[i] = target_node.ptr->data[1];
                        for (int j = 0; j < target_node.ptr->count-1; ++j) {
                            target_node.ptr->data[j] = target_node.ptr->data[j+1];
                        }
                        target_node.ptr->count -= 1;
                        return;
                    }
                }
                for (int j = 0; j < target_node.ptr->count-1; ++j) {
                    target_node.ptr->data[j] = target_node.ptr->data[j+1];
                }
                target_node.ptr->count -= 1;
                return;
            } else { // caso 1b
                for (int i = index; i < keys_count-1; ++i) {
                    target_node.ptr->data[i] = target_node.ptr->data[i+1];
                }
                target_node.ptr->count -= 1;
                return;
            }
        } else {
            // Vecino izquierdo caso 2a
            if (index != 0 && parent->children[index - 1]->count > (size_t) ORDER/2) { // En caso tenga un hijo izquierdo
                auto left = parent->children[index - 1];
                std::cout << "Izq" << std::endl;
                T borrowed_value = left->data[left->count];
                left->data[left->count] = 0; // valor nulo?
                --left->count;

                // Ponerlo en el padre
                parent->data[index] = borrowed_value;

                // Copiar los valores a un temp para reescribirlos
                std::vector<T> tmp;
                for (size_t i = 0; i < keys_count; ++i) {
                    if (target_node.ptr->data[i] != value)
                        tmp.push_back(target_node.ptr->data[i]);
                }

                // Reescribir los valores
                target_node.ptr->data[0] = borrowed_value;
                for (size_t i = 0; i < keys_count - 1; ++i) {
                    target_node.ptr->data[i + 1] = tmp[i];
                }
                return;
            }

            // Vecino derecho caso 2b
            if (index != keys_count && parent->children[index + 1]->count > (size_t) ORDER/2) { 
                auto right = parent->children[index + 1];
                std::cout << "Der" << ORDER/2 << std::endl;
                T borrowed_value = right->data[0];
                --right->count;
                // Mover todos los valores del nodo de la derecha 1 posicion a la izquierda
                for (size_t i = 0; i < right->count; ++i) {
                    right->data[i] = right->data[i + 1];
                }
                right->data[right->count] = 0; // valor nulo?

                // Ponerlo en el padre
                parent->data[index] = borrowed_value;

                // Copiar los valores a un temp para reescribirlos
                std::vector<T> tmp;
                for (size_t i = 0; i < keys_count; ++i) {
                    if (target_node.ptr->data[i] != value)
                        tmp.push_back(target_node.ptr->data[i]);
                }

                // Reescribir los valores
                target_node.ptr->data[keys_count - 1] = borrowed_value;
                for (size_t i = 0; i < keys_count - 1; ++i) {
                    target_node.ptr->data[i] = tmp[i];
                }
                return;
            }

            // Merge caso 1c hacer merge con los vecinos
            std::cout << "Merge" << std::endl;
            std::cout << parent->data[0]<<std::endl;
            parent->merge(index);

        }

    }

    void print_leaves() {

        node* aux = &root;
        while (aux->children[0] != nullptr) {
            aux = aux->children[0];
        }
        head = aux;
        node* leave = head;

        while (leave) {
            for (size_t i = 0; i < leave->count; ++i) {
                std::cout << leave->data[i] << " ";
            }
            if (leave->next) {
                std::cout << " <-> ";
            }
            leave = leave->next;
        }
        std::cout << "\n";
    }

    void print() {
        print(&root, 0);
        // std::cout << "________________________\n\n";
    }

    void print(node *ptr, int level) {
        if (ptr) {
            int i;
            for (i = ptr->count - 1; i >= 0; i--) {
                print(ptr->children[i + 1], level + 1);

                for (int k = 0; k < level; k++) {
                    std::cout << "    ";
                }
                if (ptr->isLeaf) {
                    std::cout << ptr->data[i] << /*"*" <<*/ "\n";
                } else {
                    std::cout << ptr->data[i] << "\n";
                }
            }
            print(ptr->children[i + 1], level + 1);
        }
    }

    void in_order_print() {
        in_order_helper(&root);
    }

private: 
    void in_order_helper(node* ptr) {
        if (!ptr)
            return;

        for (size_t i = 0; i < ptr->count; ++i) {
            in_order_helper(ptr->children[i]);
            std::cout << ptr->data[i] << ", ";
        }
        in_order_helper(ptr->children[ptr->count]);
    }
 
    void split_root() {
        node* _root = &root;
        node* child1 = new node();
        node* child2 = new node();
        if (_root->isLeaf) {
            child1->isLeaf = true;
            child2->isLeaf = true;
        }
        size_t i = 0;
        for (; i < _root->count / 2; i++) { // cambie aca
            child1->children[i] = _root->children[i];
            child1->data[i] = _root->data[i];
            child1->count++;
        }
        child1->children[i] = _root->children[i];

        size_t mid = i;
        i += 1;
        size_t j = 0;

        // B+
        if (_root->isLeaf) {
            child2->data[j] = _root->data[mid];
            child2->count++;
            ++j;
        }
        // B+

        for (; i < _root->count; i++) {
            child2->children[j] = _root->children[i];
            child2->data[j] = _root->data[i];
            child2->count++;
            j++;
        }
        child2->children[j] = _root->children[i];

        // set dll pointers

        child1->next = child2;
        child2->prev = child1;

        if (_root->isLeaf) {
            _root->isLeaf = false;
            head = child1;
        }

        _root->data[0] = _root->data[mid];
        _root->children[0] = child1;
        _root->children[1] = child2;
        _root->count = 1;
    }

public:

    struct iterator {
        node* ptr;
        node* parent;
        size_t index;

        iterator(node* ptr, size_t index, node* parent = nullptr): ptr{ptr}, index{index}, parent{parent} {}

        iterator& operator++(int) {
            if (index < ptr->count) {
                ++index;
            } else {
                ptr = ptr->next;
                index = 0;
            }
            return *this;
        }

        T& operator*() {
            return ptr->data[index];
        } 

        bool operator==(const iterator& i) {
            return this->ptr == i.ptr && this->index == i.index;
        }

        bool operator!=(const iterator& i) {
            return this->ptr != i.ptr || this->index != i.index;
        }

    };

    iterator find(const T& value) {
        return find_helper(&root, value, &root);
    }

    iterator find_helper(node* ptr, const T& value, node* ptr_parent) {
        size_t index = 0;
        while (ptr->data[index] < value && index < ptr->count) {
            ++index;
        }

        if (ptr->isLeaf) {
            if (index < ptr->count && ptr->data[index] == value) {
                return iterator(ptr, index, ptr_parent);
            }
            return iterator(nullptr, 0);
        }
        return find_helper(ptr->children[index], value, ptr);
    }

private:
    node root;
    node* head; // head of the dll
};

#endif