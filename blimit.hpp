
inline unsigned int bvalue(unsigned int method, unsigned long node_id) {
    switch (method) {
        case 0:
            return 4;
        case 1:
            return 7;
        default:
            return (2 * node_id + method) % 10;
    }
}