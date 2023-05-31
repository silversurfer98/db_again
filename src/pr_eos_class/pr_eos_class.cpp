#include <string>
#include <vector>
#include <set>
#include <map>
#include<memory>
#include<iostream>
#include<math.h>
#include "maths.h"
#include "db_class.h"

//PR EOS properties definition
struct PR_props
{
    float a, b, k, alpha, ac;
};

const float r = 8.3144598; // unit ---> J / K ⋅ mol

float func(float value, std::shared_ptr<std::vector<float>> parameters)
{
    // f(Z) = Z3 - CZ2 + DZ - E = 0
    return pow(value, 3) - (*parameters)[0] * pow(value, 2) + (*parameters)[1] * value - (*parameters)[2];
}

float Derivative_of_func(float value, std::shared_ptr<std::vector<float>> parameters)
{
    // f(Z) = Z3 - CZ2 + DZ - E = 0
    // f'(Z) = 3Z2 - 2CZ + D = 0
    return 3 * pow(value, 2) - 2 * (*parameters)[0] * value + (*parameters)[1];
}


class pr_eos
{
private:
// private variable
    float p=1, t=273.15;
    std::unique_ptr<std::vector<base_props>> base_data_pt;
    std::unique_ptr<std::vector<std::vector<float>>> bip_data_ptr;
    unsigned int size_of_gas_data = 1;
    bool Is_mix = true;
    // std::vector<PR_props> pr_data;

// private member funcs
    void pr_mix_report(PR_props* pr);
    void PR_consts_Calc(base_props* gas_prop, PR_props* prprops);
    void pr_mix_props();
    void pr_single_gas_props();

public:
// public variables
    std::vector<float> parameters;
    float z, zl;
    db_class mydbclass;

// public member funcs
    pr_eos(float pressure, float temperature, const char* db_name);
    ~pr_eos();
    void print_base_data();
    void print_bip_data();
    void getZ();
};

pr_eos::pr_eos(float pressure, float temperature, const char* db_n) : mydbclass(db_n)
{
    p = pressure*0.1 - 0.101325;
    // p = 0.000001 * (pressure*100000 - 101325);
    t = temperature + 273.15;
    // mydbclass = new db_class(db_n);
    
    z = 1; zl = 0;
    parameters.reserve(3);
    for(uint16_t i = 0;i<3;i++)
        parameters.push_back(1.0);
    
 
    unsigned int res = mydbclass.get_all_gas_names();
    if(res==0)
        res = mydbclass.choose_gas_from_user();
    
    size_of_gas_data = mydbclass.get_no_of_gases();
    std::cout<<"size of gase choice = "<<size_of_gas_data<<"\n\n";

    if(size_of_gas_data == 1)
        Is_mix = false;
    
    base_data_pt = mydbclass.get_base_gas_props_ptr();

    if(Is_mix)
        bip_data_ptr = mydbclass.get_bip_pointer();

    std::cout<<"PR class constructor called and finished \n\n";
}

pr_eos::~pr_eos()
{
    std::cout<<"\n\nPR_EOS destructor has been called\n\n";
}

// print funcs
void pr_eos::print_base_data()
{
    if(base_data_pt){
        std::cout<<"\n"<<"Tc"<<"\t"<<"Pc"<<"\t"<<"Acc factor"<<"\t"<<"Xi"<<"\n";
        for(auto i = base_data_pt->begin(); i != base_data_pt->end(); ++i) 
            std::cout<<"\n"<<i->tc<<"\t"<<i->pc<<"\t"<<i->w<<"\t"<<i->xi<<"\n";
    }
    else
        std::cout<<"\nenakune varuveengala da\n";
}

void pr_eos::print_bip_data()
{
    std::cout<<"\n\n";
    if(Is_mix)
        if(bip_data_ptr){
            for(auto row = bip_data_ptr->begin(); row != bip_data_ptr->end(); ++row){
            for(auto col = row->begin(); col != row->end(); ++col)
                    std::cout<<*col<<"\t";
                std::cout<<"\n";}}
    else
        std::cout<<"\nenakune varuveengala da\n";

}

void pr_eos::pr_mix_report(PR_props* pr)
{
    std::cout << "\n m = " << pr->k;
    std::cout << "\n ac = " << pr->ac;
    std::cout << "\n alpha = " << pr->alpha;
    std::cout << "\n a = " << pr->a;
    std::cout << "\n b = " << pr->b;
}

//-------------------------- end of print funcs -----------------------------------

//core PR EOS func contains all math
void pr_eos::PR_consts_Calc(base_props* gas_prop, PR_props* prprops)
{
    // pressure unit is Mpa
    prprops->k = 0.37464 + (1.54226 * gas_prop->w) - (0.26992 * pow(gas_prop->w ,2)); //the m-constant
    prprops->ac = (0.4572355289 * pow(r * gas_prop->tc, 2)) / (gas_prop->pc * 0.000001);  // preliminary PREOS a constant
    prprops->alpha = (1 + prprops->k * (1 - sqrt(t / gas_prop->tc))) * (1 + prprops->k * (1 - sqrt(t / gas_prop->tc)));

    prprops->a = prprops->ac * prprops->alpha;  //refined a-constant PREOS   cm6-pascal/mol2
    prprops->b = (0.077796074 * r * gas_prop->tc) / (gas_prop->pc * 0.000001); //b-constant PREOS   cm3/mol

    // prprops->aa = prprops->a * p / (r * r * t* t);  //A constant fpr Z-equation
    // prprops->bb = prprops->b * p / (r * t);  //B constant fpr Z-equation

    // constants for the equation 
    // prprops->c = (1 - prprops->bb);
    // prprops->d = (prprops->aa - 2 * prprops->bb - 3 * prprops->bb * prprops->bb);
    // prprops->e = (prprops->aa * prprops->bb - prprops->bb * prprops->bb - prprops->bb * prprops->bb * prprops->bb);

}


//all other shenanigans here


// new WTF
void pr_eos::pr_single_gas_props()
{
    PR_props* ans = new PR_props;
    std::cout<<"\nsingle gas\n\n";
    for(auto i = base_data_pt->begin(); i != base_data_pt->end(); ++i)
        PR_consts_Calc(&(*i), ans);
    
    float aa, bb;
    // these are the results to solve for Z
    aa = ans->a * p / (r * r * t * t);  // A constant for Z-equation
    bb = ans->b * p / (r * t);          // B constant for Z-equation
    
    delete ans;

    // float c,d,e;
    // c = (1 - bb);
    // d = (aa - 2 * bb - 3 * pow(bb, 2));
    // e = (aa * bb - pow(bb, 2) - pow(bb, 3));
    // printf("\n\nC : %f\nD : %f\nE : %f\n\n",c,d,e);

    parameters[0] = ((1 - bb));
    parameters[1] = ((aa - 2 * bb - 3 * pow(bb, 2)));
    parameters[2] = ((aa * bb - pow(bb, 2) - pow(bb, 3)));
}

void pr_eos::pr_mix_props()
{
// to implement, 
    // create pr_props vector
    // iterate through each base_gas_data and send to PR_const calc func
    std::vector<PR_props> pr_mix_data;
    pr_mix_data.reserve(size_of_gas_data);
    PR_props* ans = new PR_props;

    if(base_data_pt)
        for(auto i = base_data_pt->begin(); i != base_data_pt->end(); ++i){
            PR_consts_Calc(&(*i), ans);
            pr_mix_data.push_back(*ans);
        }
    delete ans;

    // for printing
        // for(auto i : pr_mix_data)
        //     pr_mix_report(&i);

    float t1, t2, a, b, aa=0, bb=0;

    for (unsigned int i = 0; i < size_of_gas_data; i++)
    {
        a = (pr_mix_data[i].a);
        for (unsigned int j = 0; j < size_of_gas_data; j++)
        {
            b = (pr_mix_data[j].a);
            t1 = sqrt(a * b) * (1 - (*bip_data_ptr)[i][j]);
            // t1 = sqrt((pr_mix_data[i].a) * (pr_mix_data[j].a)) * (1 - (*bip_data_ptr)[i][j]);
            t2 = (*base_data_pt)[i].xi * (*base_data_pt)[j].xi * t1;
        
            // print matrix - DEBUG
            // std::cout<<t2<<"\t";
        
            aa = aa + t2;
        }
        // std::cout<<"\n";
    }

    for (unsigned int i = 0; i < size_of_gas_data; i++)
        bb = bb + ((*base_data_pt)[i].xi * pr_mix_data[i].b);

    // these are the results to solve for Z
    aa = aa * p / (r * r * t * t);  // A constant for Z-equation
    bb = bb * p / (r * t);          // B constant for Z-equation

    // float c,d,e;
    // c = (1 - bb);
    // d = (aa - 2 * bb - 3 * pow(bb, 2));
    // e = (aa * bb - pow(bb, 2) - pow(bb, 3));
    // printf("\n\nC : %f\nD : %f\nE : %f\n\n",c,d,e);

    parameters[0] = ((1 - bb));
    parameters[1] = ((aa - 2 * bb - 3 * pow(bb, 2)));
    parameters[2] = ((aa * bb - pow(bb, 2) - pow(bb, 3)));
}
// --------------------------------

void pr_eos::getZ()
{
    if(Is_mix)
        pr_mix_props();
    else
        pr_single_gas_props();

    // create ptr to parameters
    std::shared_ptr<std::vector<float>> ptr = std::make_shared<std::vector<float>>(parameters);

    // create function pointers
    float (*fun)(float, std::shared_ptr<std::vector<float>>) = func;
    float (*funcd)(float, std::shared_ptr<std::vector<float>>) = Derivative_of_func;

    for(const auto& i : parameters)
        std::cout<<"\n"<<i;

    // before solving anything lets find whether the equation has one or 3 real toots
    float Q1 = parameters[0]*parameters[1]/6 - parameters[2]/2 - pow(parameters[0],3)/27;
    float P1 = pow(parameters[0],2)/9 - parameters[1]/3;
    float D = Q1*Q1 - P1*P1*P1;

    if (D>=0){
        std::cout<<"\nonly one real root so going for newton raphson way\n";
        // float* f = &z;
        bool ans = newton_raphson_controlled(&z,fun,funcd,ptr,0.0001,50);
        std::cout<<"\nans : "<<z;
    }
    else{
        std::cout<<"\n\nhas three roots under cons\n\n";

        std::vector<float> ini;
        ini.reserve(3);

        ini.push_back(0);
        ini.push_back(0.5);
        ini.push_back(1);

        std::shared_ptr<std::vector<float>> ini_p = std::make_shared<std::vector<float>>(ini);

        // for weistrass initial guess is VERY VERY IMPORTANT -------
        if(weistrass_controlled(ini_p, fun, ptr, 0.001, 50))
            std::cout<<"\nweistrass suceeded\n";
        
        for(const auto& i : *ini_p)
            std::cout<<i<<"\n";
    }
}