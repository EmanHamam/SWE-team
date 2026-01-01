#ifndef PROPERTY_H
#define PROPERTY_H

#include <string>

using namespace std;

struct Property
{
    int id;
    string name;
    string location;
    double price;
    string type;
    int available;
    string infoNumber;
    int noOfRooms;
    int noOfBaths;
    double area;
};

#endif