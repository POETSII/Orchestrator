#include "poets_protocol/InProcessBinaryUpstreamConnection.hpp"

void poets_in_proc_external_main(InProcessBinaryUpstreamConnection &conn, int argc, const char **argv)
{
    conn.external_log(0, "Calling connect.");
    conn.connect("external_fan_inst_in_proc", "external_fan_inst_in_proc_inst", {
        {"ext0","ping"}, {"ext1","ping"}, {"ext2","ping"}, {"ext3","ping"}
    });
    conn.external_log(0, "Connect complete.");

    auto ext0_address=conn.get_device_address("ext0");
    auto ext1_address=conn.get_device_address("ext1");
    auto ext2_address=conn.get_device_address("ext2");
    auto ext3_address=conn.get_device_address("ext3");

    while(1){
        conn.external_log(0, "Waiting for message");
        conn.wait_until(false);

        conn.external_log(0, "Receiving message");
        if(conn.can_recv()){
            std::shared_ptr<std::vector<uint8_t>> payload;
            std::pair<unsigned,const poets_endpoint_address_t*> fanout;
            conn.recv(fanout, payload);

            for(unsigned i=0; i<fanout.first; i++){
                auto dst=fanout.second[i];
                conn.external_log(0, "Dst = %llu", dst.value);
                if(dst== makeEndpoint(ext0_address,poets_pin_index_t{0})){
                    conn.external_log(0, "Sending message from ext0 : %u.", *(uint32_t*)&payload->at(0));
                    conn.send( makeEndpoint(ext0_address, poets_pin_index_t{0}), payload );
                
                }else if(dst== makeEndpoint(ext1_address,poets_pin_index_t{0})){
                    conn.external_log(0, "Sending message from ext1 : %u.", *(uint32_t*)&payload->at(0));
                    conn.send( makeEndpoint(ext1_address, poets_pin_index_t{0}), payload );

                }else if(dst== makeEndpoint(ext2_address,poets_pin_index_t{0})){
                    conn.external_log(0, "Sending message from ext2 : %u.", *(uint32_t*)&payload->at(0));
                    conn.send( makeEndpoint(ext2_address, poets_pin_index_t{0}), payload );

                }else if(dst== makeEndpoint(ext3_address,poets_pin_index_t{0})){
                    conn.external_log(0, "Sending message from ext3 : %u.", *(uint32_t*)&payload->at(0));
                    conn.send( makeEndpoint(ext3_address, poets_pin_index_t{0}), payload );

                }else{
                    throw std::runtime_error("Unknown dest device.");
                }
            }

            
        }
    }
}
