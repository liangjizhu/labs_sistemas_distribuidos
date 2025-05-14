/*
 * log.x — Interfaz RPC para enviar logs de operaciones
 */
struct log_entry {
    string username<256>;
    string operation<512>;
    string timestamp<32>;
};

program LOG_PROG {
    version LOG_VERS {
        void SENDLOG(log_entry) = 1;
    } = 1;
} = 0x20000001;
