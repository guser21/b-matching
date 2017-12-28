#ifndef A_H
#define A_H
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
///**

unsigned int bvalueq(unsigned int method, unsigned long node_id) {
    switch (node_id) {
        case 2:
            return 2;
        case 3:
            return 1;
        default:
            return 2;
    }
}
// */
//unsigned int bvalue(unsigned int method, unsigned long node_id) {
//
//    switch (method) {
//
//        case 0:
//            return 1;
//        case 1:
//            return 3;
//        default:
//            switch (node_id) {
//
//                case 0:
//                    return 2;
//
//                case 1:
//                    return 2;
//
//                default:
//                    return 1;
//
//            }
//
//    }
//
//}

//unsigned int bvalue(unsigned int method, unsigned long node_id) {
//
//    switch (method) {
//
//        case 0: return 1;
//
//        default: switch (node_id) {
//
//                case 0: return 2;
//
//                case 1: return 2;
//
//                default: return 1;
//
//            }
//
//    }
//
//}

//unsigned int bvalue(unsigned int method, unsigned long node_id) {
//    switch (node_id) {
//        case 0:
//            return 2;
//        case 2:
//            return 2;
//        default:
//            return 1;
//    }
//}
#endif