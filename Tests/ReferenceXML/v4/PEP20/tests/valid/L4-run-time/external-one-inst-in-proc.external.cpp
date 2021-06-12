#include "poets_protocol/InProcessBinaryUpstreamConnection.hpp"

void poets_in_proc_external_main(InProcessBinaryUpstreamConnection &conn, int argc, const char **argv)
{
    conn.external_log(0, "Calling connect.");
    conn.connect("external_one_inst_in_proc", "external_one_inst_in_proc_inst", {{"ext0","ping"}});
    conn.external_log(0, "Connect complete.");

    auto ext0_address=conn.get_device_address("ext0");

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
                if(dst!= makeEndpoint(ext0_address,poets_pin_index_t{0})){
                    throw std::runtime_error("External received message for unknown dest.");
                }

                conn.external_log(0, "Sending message.");
                conn.send( makeEndpoint(ext0_address, poets_pin_index_t{0}), payload );
            }
        }
    }
}
