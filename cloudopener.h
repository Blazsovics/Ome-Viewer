#ifndef CLOUDOPENER_H
#define CLOUDOPENER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

struct CloudData{
    std::vector<float> data;
private:
    std::map<std::string, int> attribute_names;
    std::vector<std::string> channel_names;
    unsigned num_attributes = 0;
    unsigned num_vertices = 0;
    std::vector<std::vector<float>> attribute;
    std::vector<std::string> tokens;
    std::fstream file;
    std::stringstream ss;
    std::string line;
    float var = 0.0f;

    void inline open(std::string filename){
        file.open(filename.c_str(), std::fstream::in);
        if(!file.is_open()) std::cout<<"\ncould not open file "<<filename;
    }
    void inline close(){
        file.close();
    }
    void inline tokenizeStr(std::string str){
        //str is just a copy
        size_t first = 0;
        size_t last = 0;
        while(first < str.size()){
            //assuming tab delimiter
            last = str.find_first_of('\t', first);
            if(last == std::string::npos){
                last = str.size();
            }
            std::string token = str.substr(first, last-first);
            tokens.push_back(token);
            first = last + 1;
        }
    }
    void inline readLine(){
        tokens.clear();
        ss.clear();
        ss.str("");
        line = "";
        std::getline(file, line);
        tokenizeStr(line);
    }
    void inline getAttribNames(){
        readLine();
        for(unsigned i = 0; i < tokens.size(); i++)
            attribute_names.insert(std::pair<std::string, int>(tokens[i], i-1));
        tokens.clear();
        line = "";
        num_attributes = attribute_names.size();
        //variable names on console:
        /*
        std::cout<<"\nfound attributes: ";
        for(auto i = attribute_names.begin(); i != attribute_names.end(); i++){
            std::cout<<"\n\t"<<i->first<<" "<<i->second;
        }
        */
    }
    void inline storeChannelName(){
        channel_names.push_back(tokens[0]);
    }
    void inline storeData(unsigned idx){
        ss.clear(); ss.str("");
        ss<<tokens[idx+1];
        var = 0.0f;
        ss>>var;
        attribute[idx].push_back(var);
    }
    void inline storeAllData(){
        std::cout<<"\nloading all data...";
        attribute.resize(num_attributes-1, std::vector<float>(0));
        do {
            readLine();
            storeChannelName();
            for(unsigned i = 0; i < num_attributes-1; i++)
                storeData(i);
            num_vertices++;
        } while(!file.eof());
        std::cout<<"\ndata loaded";
    }
    void inline setupBufferData(){
        std::cout<<"\nsetting render data...";
        data.reserve(num_vertices * 4);
        auto lambda_add = [&](const unsigned &i, std::string key_str){
            unsigned idx = attribute_names[key_str];
            data.push_back( attribute[idx][i] );
        };
        for(unsigned i = 0; i < num_vertices; i++){
            lambda_add(i, "Xc");
            lambda_add(i, "Yc");
            lambda_add(i, "Zc");
            lambda_add(i, "Photons");
        }
        std::cout<<"\nrender data set "<<data.size()/4<<" x 4";
        /*
        for(unsigned i=0; i < data.size()/4; i++)
            std::cout<<"\n\t"<<data[i*4+0]<<' '<<data[i*4+1]<<' '<<data[i*4+2]<<' '<<data[i*4+3];
            */

    }
    void inline clearContainers(){
        data.clear();
        attribute_names.clear();
        channel_names.clear();
        num_attributes = 0;
        num_vertices = 0;
        attribute.clear();
        tokens.clear();
        ss.clear(); ss.str("");
        line = "";
        float var = 0.0f;
    };
public:
    void openFile(std::string filename){
        if(filename == "") return;
        open(filename);
        if(!file.is_open()) return;
        clearContainers();
        getAttribNames();
        storeAllData();
        setupBufferData();
        close();
    }
};

#endif // CLOUDOPENER_H
