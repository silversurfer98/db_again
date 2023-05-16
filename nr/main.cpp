#include "nr.h"
#include<iostream>
#include<vector>
#include<memory>
float func(float x, std::shared_ptr<std::vector<float>> parameters)
{
    float temp = 2*x*x*x - 2*x - (*parameters)[0];
    return temp;
}

float func_derv(float x)
{
    float temp = 6*x*x - 2;
    return temp;
}

float newton_raphson(float* initial_value, float (*func)(float, std::shared_ptr<std::vector<float>>), float (*funcd)(float), std::shared_ptr<std::vector<float>> parameters)
{
    float Next_value = *initial_value - (func(*initial_value, parameters) / funcd(*initial_value));
    //report("Next value",Next_value);

    // to find the diff
    float diff = Next_value - *initial_value;

    // to null the minus sign --> finds abs of diff
    // if (diff < 0) diff = diff * -1;
    *(int *)&diff = *(int *)&diff & 0b01111111111111111111111111111111;
    //std::cout<<count;  
    //report("diff",diff);

    if(diff >= 0.01)
    {
        newton_raphson(float* Next_value, float (*func)(float, std::shared_ptr<std::vector<float>>), float (*funcd)(float), std::shared_ptr<std::vector<float>> parameters);
    }
    else
        return Next_value;
}
int main()
{
    std::vector<float> parameters;
    parameters.push_back(5);

    std::shared_ptr<std::vector<float>> ptr;

    ptr = std::make_shared<std::vector<float>>(parameters);

    float (*fun)(float, std::shared_ptr<std::vector<float>>) = func;

    std::cout<<"ans : "<<fun(0,ptr);
    std::cout<<"\n\nmain";
    return nr();
}