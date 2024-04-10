#include <cmath>
#include <iostream>

// Funktion zur Überprüfung, ob auf ein Objekt gedeutet werden kann

// pos1: Array mit x- und y-Koordinate des Nao's
// pos2: Array mit x- und y-Koordinate der Position des Objekts
// rotation: Drehung des Nao's
// field_of_view: Sichtfeld in Grad

bool is_in_field_of_view(double pos1[2], double pos2[2], double rotation, double field_of_view) {
    // Berechne die Differenz der x- und y-Koordinaten zwischen Ihrer Position und der des Objekts
    double dx = pos2[0] - pos1[0];
    double dy = pos2[1] - pos1[1];
    
    // Berechne den Winkel des Objekts relativ zur Nao Position
    double angle_object  = fmod((atan2(dy, dx) * (180.0 / M_PI) + 360.0));
    // Berücksichtige die Drehung, um den Winkelbereich des Sichtfelds zu bestimmen
    rotation = fmod(rotation, 360.0);
    
    // Berechne den minimalen und maximalen Winkel, des Zeigbaren bereiches
    double min_angle = fmod((rotation + field_of_view / 2.0 + 360.0));
    double max_angle = fmod((rotation - field_of_view / 2.0 + 360.0));
    
    // Überprüfe, ob der Winkel des Objekts innerhalb des Zeigbaren bereiches liegt
    if (min_angle < max_angle)
        return min_angle <= angle_object  && angle_object  <= max_angle;
    else
        return angle_object  >= min_angle || angle_object  <= max_angle;
}


int main() {
    
    double pos1[2] = {-1.5, -2.5};  // Ihre Position
    double pos2[2] = {-5, 3.6};  // Position des Objekts
    double rotation = 90;   // Ihre Drehung in Grad
    double sichtfeld = 60; // Ihr Sichtfeld in Grad


    if (is_in_field_of_view(pos1, pos2, rotation, sichtfeld)) {
        std::cout << "Ball ist im Zeigbaren bereich" << std::endl;
    } else {
        std::cout << "Ball nicht im Zeigbaren bereich" << std::endl;
    }

    return 0;
}
