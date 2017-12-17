#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <functional>
#include <type_traits>

#include <zmq.hpp>

#include "maxcpp6.h"

#include "bbb.zmq.sub.hpp"

zmq::context_t ctx;

template <typename t>
inline t_atom_long ptr_conv(void *c) {
    return static_cast<t_atom_long>(*static_cast<t *>(c));
}

std::map<format_token, std::tuple<std::function<t_atom_long(void *)>, std::size_t>> long_converters {
    {
        format_token::int8_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<int8_t>(cursor); }, 1}
    }, {
        format_token::uint8_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<uint8_t>(cursor); }, 1}
    }, {
        format_token::int16_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<int16_t>(cursor); }, 2}
    }, {
        format_token::uint16_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<uint16_t>(cursor); }, 2}
    }, {
        format_token::int32_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<int32_t>(cursor); }, 4}
    }, {
        format_token::uint32_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<uint32_t>(cursor); }, 4}
    }, {
        format_token::int64_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<int64_t>(cursor); }, 8}
    }, {
        format_token::uint64_type,
        {[](void *cursor) -> t_atom_long { return ptr_conv<uint64_t>(cursor); }, 8}
    }
};

std::map<format_token, std::tuple<std::function<t_atom_float(void *)>, std::size_t>> float_converters{
    {
        format_token::float_type,
        {[](void *cursor) -> t_atom_float { return *(float *)cursor; }, 4}
    }, {
        format_token::double_type,
        {[](void *cursor) -> t_atom_float { return *(double *)cursor; }, 8}
    },
};

class MaxZmqSub : public MaxCpp6<MaxZmqSub> {
    std::thread th;
    std::shared_ptr<std::atomic_bool> b_running;
    std::string format_str{"[t]"};
    
    static void print_info() {
        static bool isnt_printed = true;
        if(isnt_printed) {
            post("bbb.zmq.sub: v%d.%d.%d\n", bbb::major_version, bbb::minor_version, bbb::patch_version);
            if(bbb::is_develop_ver) {
                post("  [develop version]\n");
            }
            post("  author: ISHII 2bit [i@2bit.jp]\n");
            post("  url:    %s\n", bbb::distribute_url);
            isnt_printed = false;
        }
    }
    
    inline void set_annotation() {
        for(std::size_t i = 0; i < 4; i++) {
            
        }
    }
    
    inline void dump(const std::string &str) {
        post("bbb.zmq.sub: %s\n", str.c_str());
    }
    
    inline void dump_error(const std::string &str) {
        error("bbb.zmq.sub: %s\n", str.c_str());
    }
    
    bool is_running() const { return b_running && *b_running; }
    
    void output_msg(zmq::message_t &msg) {
        std::size_t current_format_index = 0;
        const std::size_t data_size = msg.size();
        std::size_t data_cursor = 0;
        
        std::vector<t_atom> atoms;
        outlet_int(m_outlets[2], data_size);
        while(data_cursor < data_size && current_format_index < format_str.length()) {
            void *cursor = (void *)((uint8_t *)msg.data() + data_cursor);
            format_token token = (format_token)format_str[current_format_index];
            switch(token) {
                case format_token::int8_type:
                case format_token::uint8_type:
                case format_token::int16_type:
                case format_token::uint16_type:
                case format_token::int32_type:
                case format_token::uint32_type:
                case format_token::int64_type:
                case format_token::uint64_type: {
                    auto &&converter = long_converters[token];
                    atoms.emplace_back();
                    atom_setlong(&atoms.back(), std::get<0>(converter)((uint8_t *)cursor));
                    data_cursor += std::get<1>(converter);
                    break;
                }
                case format_token::float_type:
                case format_token::double_type: {
                    auto &&converter = float_converters[token];
                    atoms.emplace_back();
                    atom_setfloat(&atoms.back(), std::get<0>(converter)((uint8_t *)cursor));
                    data_cursor += std::get<1>(converter);
                    break;
                }
                case format_token::text_type: {
                    char *ptr = (char *)cursor;
                    std::string data{ptr, strnlen(ptr, data_size - data_cursor)};
                    atoms.emplace_back();
                    atom_setsym(&(atoms.back()), gensym(data.c_str()));
                    data_cursor += strlen(ptr) + 1;
                    break;
                }
                case format_token::array_begin: {
                    if(
                       (format_token)format_str[current_format_index + 1] != format_token::array_end
                       && (format_token)format_str[current_format_index + 2] == format_token::array_end
                    ) {
                        format_token loop_token = (format_token)format_str[current_format_index + 1];
                        switch(loop_token) {
                            case format_token::int8_type:
                            case format_token::uint8_type:
                            case format_token::int16_type:
                            case format_token::uint16_type:
                            case format_token::int32_type:
                            case format_token::uint32_type:
                            case format_token::int64_type:
                            case format_token::uint64_type: {
                                std::size_t datum_size = std::get<1>(long_converters[loop_token]);
                                std::size_t num_data = (data_size - data_cursor) / datum_size;
                                auto &&converter = std::get<0>(long_converters[loop_token]);
                                for(std::size_t i = 0; i < num_data; i++) {
                                    atoms.emplace_back();
                                    atom_setlong(&atoms.back(), converter((uint8_t *)cursor + (i * datum_size)));
                                }
                                data_cursor += num_data * datum_size;
                                break;
                            }
                            case format_token::float_type:
                            case format_token::double_type: {
                                std::size_t datum_size = std::get<1>(float_converters[loop_token]);
                                std::size_t num_data = (data_size - data_cursor) / datum_size;
                                auto &&converter = std::get<0>(float_converters[loop_token]);
                                for(std::size_t i = 0; i < num_data; i++) {
                                    atoms.emplace_back();
                                    atom_setfloat(&atoms.back(), converter((uint8_t *)cursor + (i * datum_size)));
                                }
                                data_cursor += num_data * datum_size;
                                break;
                            }
                            case format_token::text_type: {
                                std::size_t str_length;
                                while(data_cursor < data_size) {
                                    char *ptr = (char *)msg.data() + data_cursor;
                                    str_length = strnlen(ptr, data_size - data_cursor);
                                    if(data_size < data_cursor + str_length) break;
                                    std::string data{ptr, str_length};
                                    atoms.emplace_back();
                                    atom_setsym(&atoms.back(), gensym(data.c_str()));
                                    data_cursor += str_length + 1;
                                }
                            }
                            default: break;
                        }
                        if(atoms.empty()) dump_error("data is short than required.");
                        else {
                            outlet_int(m_outlets[1], atoms.size());
                            outlet_atoms(m_outlets[0], atoms.size(), atoms.data());
                        }
                        return;
                    }
                    break;
                }
                case format_token::array_end: {
                    while((format_token)format_str[current_format_index--] != format_token::array_begin);
                    break;
                }
                case format_token::skip_byte: {
                    data_cursor += 1;
                    break;
                }
                default:
                    
                    break;
            }
            current_format_index++;
        }
        outlet_int(m_outlets[1], atoms.size());
        outlet_atoms(m_outlets[0], atoms.size(), atoms.data());
    }
    
    bool connect_impl(t_atom *host) {
        if(is_running()) {
            dump_error("alredy connected.");
            return false;
        }
        
        b_running = std::make_shared<std::atomic_bool>(true);
        auto coppied_b_running = b_running;
        
        th = std::thread([this, coppied_b_running, host] {
            zmq::socket_t socket(ctx, ZMQ_SUB);
            socket.connect(atom_getsym(host)->s_name);
            socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
            
            while(*coppied_b_running) {
                zmq::message_t msg;
                try {
                    if(socket.recv(&msg, ZMQ_NOBLOCK)) {
                        output_msg(msg);
                    }
                } catch(const zmq::error_t& ex) {
                    if(ex.num() != ETERM)
                        throw;
                }
                std::this_thread::sleep_for(std::chrono::nanoseconds(200));
            }
            socket.close();
        });
        th.detach();
        return true;
    }
    
    void disconnect_impl() {
        if(is_running()) {
            *b_running = false;
        }
    }

    void parse_format(t_atom *pat_atom) {
        format_str = atom_getsym(pat_atom)->s_name;
    }
    
public:
    MaxZmqSub() {
        setupIO(1, 4); // inlets / outlets
        print_info();
        dump("bbb.zmq.sub: will initialize...");
    }
    
	MaxZmqSub(t_symbol *sym, long ac, t_atom *av)
        : MaxZmqSub()
    {
        if(0 < ac) {
            if(atom_gettype(av) == A_SYM) {
                connect_impl(av);
            }
        }
        if(1 < ac) {
            if(atom_gettype(av + 1) == A_SYM) {
                parse_format(av + 1);
            }
        }
    }
	
    ~MaxZmqSub() {
        disconnect_impl();
    }
    
	// methods:
    void connect(long inlet, t_symbol *s, long ac, t_atom *av) {
        if(0 < ac && atom_gettype(av) == A_SYM) connect_impl(av);
    }
    void disconnect(long inlet, t_symbol *s, long ac, t_atom *av) {
        disconnect_impl();
    }

    void format(long inlet, t_symbol *s, long ac, t_atom *av) {
        if(0 < ac && atom_gettype(av) == A_SYM) parse_format(av);
    }
    
    void assist(void *b, long io, long index, char *s) {
        switch (io) {
            case 1:
                switch(index) {
                    case 0:
                        strncpy_zero(s, "command input", 32);
                        break;
                    default:
                        break;
                }
                break;
            case 2:
                switch(index) {
                    case 0:
                        strncpy_zero(s, "parsed data", 32);
                        break;
                    case 1:
                        strncpy_zero(s, "num parsed data", 32);
                        break;
                    case 2:
                        strncpy_zero(s, "received data size", 32);
                        break;
                    case 3:
                        strncpy_zero(s, "info out", 32);
                        break;
                    default:
                        break;
                }
                break;
        }
    }
};

C74_EXPORT int main(void) {
	// create a class with the given name:
	MaxZmqSub::makeMaxClass("bbb.zmq.sub");
    REGISTER_METHOD_GIMME(MaxZmqSub, connect);
    REGISTER_METHOD_GIMME(MaxZmqSub, disconnect);
    REGISTER_METHOD_GIMME(MaxZmqSub, format);
    REGISTER_METHOD_ASSIST(MaxZmqSub, assist);
}
