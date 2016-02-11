#include <poll.h>
#include <iostream>
#include <string.h>
#include <vector>
#include<map>
#include<algorithm>
#include <boost/lexical_cast.hpp>

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <Hbase.h>

#include <server_details.h>
#include <binaryConverter.h>


#include<Frame.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::hadoop::hbase::thrift;

typedef std::vector<std::string> StrVec;
typedef std::map<std::string,std::string> StrMap;
typedef std::vector<ColumnDescriptor> ColVec;
typedef std::map<std::string,ColumnDescriptor> ColMap;
typedef std::vector<TCell> CellVec;
typedef std::map<std::string,TCell> CellMap;

using std::cout;
using std::endl;
template<
        typename RETURN ,
        typename Iterator ,
        typename Lambda
        >
RETURN fold( Iterator s, Iterator e, RETURN D, Lambda l){
    RETURN r = D;
    for(Iterator i = s; i!=e; i++){
        l(*i, r);
    }
    return r;
}

template<
        typename Iterator ,
        typename Lambda
        >
void forall( Iterator s, Iterator e, Lambda l){
    for( ; s!=e; s++){
        l(*s);

    }
}

template< typename Iterator>
void print(const Iterator start, const Iterator end, const std::string &separator = " "){
    for(auto i=start;i!=end;i++){
        std::cout << *i << separator;
    }
    std::cout << std::endl;
}
template< typename Iterator >
void print(const Iterator start, const Iterator end,  int limit , const std::string &separator = " "){
    int p=0;
    auto i=start;
    for(; i!=end && p < limit;i++,p++){
        std::cout << *i << separator;
    }
    if(i!= end){
        std::cout<<"\n[ OUTPUT CUT AFTER: "<<limit<<" ENTRY ]"<<std::endl;
    }
    std::cout << std::endl;
}



/**
Print info about table name + Y if enabled (N if not)
*/
void printTablesStatus(HbaseClient &client){
    std::vector<std::string> table_names;
    client.getTableNames(table_names);
    auto f = [&] (std::string &s, int& acc)
    {
        std::string enabled = " N";
        if(client.isTableEnabled(s)){
            enabled = " Y";
            std::cout<<acc<<" Table "<<s<<" - "<<enabled<<std::endl;

        }
        std::vector<TRegionInfo> regionOfTable;
        client.getTableRegions(regionOfTable,s);
        print(regionOfTable.begin(),regionOfTable.end() , 100000,"\n");
        return ++acc;

    };

    int a = fold(table_names.begin(),table_names.end(),0, f);
    std::cout<<"Velue is "<<a<<std::endl;
}


/*Create a frame from a timestep*/
template <class T>
auto createFrameHBase(const std::string& rowkey, const std::string& table,  HbaseClient& client){
    std::vector<TRowResult> res;
    std::map<Text, Text>  attributes;
    client.getRow(res, table, rowkey,attributes);

    boost::shared_ptr<Frame<T>> frame(new Frame<T>()) ;

    //binary converter
    converter<int64_t> c_int64;
    converter<double> c_double;

    for(int i=0;i<res.size();i++){
        //foreach particle get its coordinates
        TRowResult a = res[i];
        for(auto it = a.columns.begin(); it != a.columns.end() ; ++it ) {
            std::string key = it->first;

            if(key.find("M:c") != std::string::npos){ //coordinate info for the particle
                //get the particle ID
                Particle<T>* p = new Particle<T>();

                std::string key_suffix = key.substr(10,8); //binario long long
                int64_t p_id =  c_int64.fromBytes(reinterpret_cast<const unsigned char*>(key_suffix.c_str()),true);
                p->id = p_id;

                //read coordinate ffor particle p_id
                std::string value= a.columns.at(key).value; //24 bytes: 3 double




                //firstr coordinate
                std::string value_suffix = value.substr(0,8);
                double res_value =  c_double.fromBytes(reinterpret_cast<const unsigned char*>(value_suffix.c_str()),true);
                p->p.x = res_value;

                //second coordinate
                value_suffix = value.substr(8,8);
                res_value =  c_double.fromBytes(reinterpret_cast<const unsigned char*>(value_suffix.c_str()),true);
                p->p.y = res_value;

                //third coordinate
                value_suffix = value.substr(16,8);
                res_value =  c_double.fromBytes(reinterpret_cast<const unsigned char*>(value_suffix.c_str()),true);
                p->p.z = res_value;

                frame->particles.push_back(*p);
                printf("Coordinate for particle %i are [%.2f,%.2f,%.2f]\n",p->id,p->p.x,p->id,p->p.y,p->id,p->p.z);

            }
        }

    }
    return frame;
}

typedef  double Payload;
int main(int argc, char**argv){

    HPRS::server_details<std::string> sd("localhost",9090);

    /* Connection to the Thrift Server */
    boost::shared_ptr<TSocket> socket(new TSocket(sd.hostname, sd.port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

    /* Create the Hbase client */
    HbaseClient client(protocol);

    try{
        transport->open();

        //print DB status and table infos
        printTablesStatus(client);

        std::string rowkey = "97018a43cc1087b2ab701820357d80ca00000003DEM413f6af80000000000000000";
        std::string table = "Simulations_Data";

        boost::shared_ptr<Frame<Payload>> frame;
        frame = createFrameHBase<Payload>(rowkey, table,  client);

        frame->print(std::cout);



    }catch (const TException &tx) {
        std::cerr << "ERROR: " << tx.what() << std::endl;
    }

    return 0;
}
