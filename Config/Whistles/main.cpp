#include <iostream>
#include <cmath>
#include <vector>
#include <complex>
#include <fftw3.h>

// Fonction pour calculer la transform�e de Fourier rapide (FFT)
void calculateFFT(const std::vector<double>& signal, std::vector<std::complex<double>>& spectrum) {
    int n = signal.size();

    // Utilisation de FFTW pour la FFT
    fftw_complex* in = reinterpret_cast<fftw_complex*>(const_cast<double*>(signal.data()));
    fftw_complex* out = reinterpret_cast<fftw_complex*>(spectrum.data());

    fftw_plan plan = fftw_plan_dft_r2c_1d(n, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    fftw_destroy_plan(plan);
}


struct Position {
    double x;
    double y;
};

Position triangulationAcoustique(double speedOfSound, double deltaT, double distanceBetweenMicrophones) {
    // Distance parcourue par le son entre les deux microphones
    double soundPath = speedOfSound * deltaT;

    // Calcul de la distance de la source � chaque microphone (bas� sur le temps de vol du son)
    double d1 = distanceBetweenMicrophones / 2.0;
    double d2 = sqrt((soundPath * soundPath) - (d1 * d1));

    // Calcul des coordonn�es x et y de la source par triangulation
    Position sourcePosition;
    sourcePosition.x = (d1 * d1 - d2 * d2) / (2 * distanceBetweenMicrophones);
    sourcePosition.y = sqrt(d1 * d1 - sourcePosition.x * sourcePosition.x);

    return sourcePosition;
}




int main() {
    // Exemple d'utilisation
    std::vector<double> audioSignal = { /* Votre signal audio ici */ };

    // Calcul de la FFT
    int n = audioSignal.size();
    std::vector<std::complex<double>> spectrum(n / 2 + 1);  // Nous n'avons besoin que de la moiti� (partie positive) du spectre

    calculateFFT(audioSignal, spectrum);

    // Donn�es connues
    double speedOfSound = 343.0;  // Vitesse du son en m/s � temp�rature ambiante
    double deltaT = 0.001;  // Diff�rence de temps en secondes entre les arriv�es des sons
    double distanceBetweenMicrophones = 0.1;  // Distance entre les deux microphones en m�tres

    // Appel de la fonction pour estimer la position de la source sonore
    Position estimatedPosition = triangulationAcoustique(speedOfSound, deltaT, distanceBetweenMicrophones);

    // Affichage des coordonn�es estim�es de la source
    std::cout << "Position estimee de la source sonore : x = " << estimatedPosition.x << " m, y = " << estimatedPosition.y << " m" << std::endl;


}
// Analyse du spectre
for (int i = 0; i < spectrum.size(); ++i) {
    // Analysez les amplitudes et les fr�quences du spectre ici
    spectrum[i].real();
    i* sampleRate / n;
}

return 0;
}

