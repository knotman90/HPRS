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


        std::vector<TRowResult> res;
        std::vector<Text> columns;
        columns.push_back("M");

        std::map<Text, Text>  attributes;


        Bytes cq_prefix;
        cq_prefix="M:c000001_";
        Bytes cq_suffix;


        // a.columns.at((char*)cq).value;

        /*ROW keys
         * cd563d7777f23af71dca030ee46d7f1e00000000000000000000000000000038
         * 97018a43cc1087b2ab701820357d80ca00000003DEM414ce2880000000000000000
         *
         * // client.getRowWithColumns(res, "Simulations_Data", "97018a43cc1087b2ab701820357d80ca00000003DEM414ce2880000000000000000",columns,attributes);
         * */

        std::string rowkey = "97018a43cc1087b2ab701820357d80ca00000003DEM412015300000000000000000";
        //  client.getRowWithColumns(res, "Simulations_Data", "97018a43cc1087b2ab701820357d80ca00000003DEM414ce2880000000000000000",columns,attributes);
        client.getRow(res, "Simulations_Data", rowkey,attributes);



        for(int i=0;i<res.size();i++){

            TRowResult a = res[i];
            unsigned long long  d=38778168;
            cq_suffix= convert_cast<unsigned long long*,  char*>(&d);
            // IntegerToBytes((long long )1,cq,10);
            Bytes cq = cq_prefix + cq_suffix;
            int aa=1000;
            int bb=0;

            for(auto it = a.columns.begin(); it != a.columns.end() && aa >bb ; ++it , bb++) {

                std::string key = it->first;
                cout<<"KEYSIZE= "<<key.size()<<endl;
                if(key.find("M:c") != std::string::npos){
                    //print keys as it is
                    std::cout<<key <<" = ";
                    printHex(key, key.size());
                    printHex(a.columns.at(key).value,sizeof(double)*3);
                    //print number of the particle



                    converter<int64_t> c;


                    std::string key_suffix = key.substr(10,8); //binario long long
                    int64_t res =  c.fromBytes(key_suffix.c_str(),true);//convert_cast<const  char*,   int64_t>(key_suffix.c_str());
                    std::cout <<res<< " \n";




                    int64_t test = c.fromBytes(c.toBytes((int64_t)11000));
    std::cout <<test<< " \n";

                }

                // print_bytes(it->first.c_str(),15);


            }

            /* std::cout<<"CQ\n"<<cq<<std::endl;
            print_bytes(cq.c_str(),15);

            std::cout<<cq<<std::endl;
            print_bytes(cq.c_str(),15);
*/

        }
        // print(res.begin(),res.end(),10);

        //97018a43cc1087b2ab701820357d80ca00000003DEM414ce2880000000000000000



    }catch (const TException &tx) {
        std::cerr << "ERROR: " << tx.what() << std::endl;
    }

    return 0;
}
