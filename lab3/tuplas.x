struct Coord {
    int x;
    int y;
};

struct set_args {
    int key;
    string value1<256>;
    int N_value2;
    double V_value2<32>;
    Coord value3;
};

struct get_result {
    int status;
    string value1<256>;
    int N_value2;
    double V_value2<32>;
    Coord value3;
};

struct modify_args {
    int key;
    string value1<256>;
    int N_value2;
    double V_value2<32>;
    Coord value3;
};

program TUPLAS_PROG {
    version TUPLAS_VERS {
        int SET_VALUE(set_args) = 1;
        get_result GET_VALUE(int) = 2;
        int MODIFY_VALUE(modify_args) = 3;
        int DELETE_KEY(int) = 4;
        int EXIST(int) = 5;
        int DESTROY(void) = 6;
    } = 1;
} = 0x31230000;
