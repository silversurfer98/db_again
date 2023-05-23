#include <vector>
#include<memory>

struct CP_Const
{
    float A, B, C, D;
};

//PR EOS properties definition
struct PR_props
{
    float a, b, aa, bb, k, alpha, ac, c, d, e;
};

struct base_props
{
    float tc, pc, w;
};

// --------------------------------------------------------
// Class declaration
// --------------------------------------------------------

class db_class
{
public:
// variables
    bool is_mix;
    unsigned int size_of_gas_data;

//class members
    db_class(const char* custom_filename);
    unsigned int get_all_gas_names();
    unsigned int choose_gas_from_user();
    std::unique_ptr<std::vector<std::vector<float>>> get_bip_pointer();
    std::unique_ptr<std::vector<CP_Const>> get_cp_const_pointer();
    std::unique_ptr<std::vector<base_props>> get_base_gas_props_ptr();
    virtual ~db_class();
};