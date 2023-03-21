#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include "gsl/span"

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.,

class ListeFilms {
public:
	ListeFilms();
	void ajouterFilm(Film* film);
	void enleverFilm(Film* film);
	Acteur* trouverActeur(const std::string& nomActeur);
	void detruireListeFilm();
	void afficherListeFilms() const;
	void afficherFilmographieActeur(const std::string& nomActeur);
	gsl::span<Film*> enSpan() const;
	int getNElements() const;
	Film** getElements();

private:
	int capacite_;
	int nElements_;
	Film** elements_;
};

struct ListeActeurs {
	int capacite, nElements;
	Acteur** elements; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.
};

struct Film
{
	std::string titre, realisateur; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	int anneeSortie, recette; // Année de sortie et recette globale du film en millions de dollars
	ListeActeurs acteurs;
};

struct Acteur
{
	std::string nom; int anneeNaissance; char sexe;
	ListeFilms joueDans;
};
