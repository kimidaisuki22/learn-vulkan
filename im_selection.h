#pragma once


#include <string>
#include <vector>
template<typename T>
class Im_selection{

    void draw(){

    }
    void render(T& value,std::string& buffer){

    }
    void add(T value){  
        options_.push_back(std::move(value));
    }
    void on_select(std::vector<T>& list,int index,int old_selection){

    }
    void on_remove(std::vector<T>& list,int index,bool is_selected){

    }
    int selected_index_ =  0;
    bool removable_ = false;
    std::vector<T> options_;
};