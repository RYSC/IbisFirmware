#include <iostream>

char* floatToChar(float input)
{
    char buf[string(input).length];
    sprintf(buf, "%d",float(input));
    return buf
}

int main()
{
    float h = 70.66;

    cout<< "\n\nHello World. This is my char" + floatToChar(h) + ". Yup"<<endl;
    return 0;

}