#pragma once
#ifndef POSITION_H_
#define POSITION_H_

#include <cmath>

class Position {
    private:
        double x = 0, y = 0;

    public:
        Position() {}
        Position(double x, double y) : x(x), y(y) {}

        double getX() const;
        void setX(double x);
        double getY() const;
        void setY(double y);
        double euclideanDistance(const Position &pos2) const;
};

double Position::getX() const {
    return this->x;
}

void Position::setX(double x) {
    this->x = x;
}

double Position::getY() const {
    return this->y;
}

void Position::setY(double y) {
    this->y = y;
}

double Position::euclideanDistance(const Position &pos2) const {
    return sqrt(pow(pos2.x - x, 2) + pow(pos2.y - y, 2));
}

#endif