#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <cassert>
#include "gsl/span"
#include <memory>
using gsl::span;
using namespace std;

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.

class ListeFilms {
public:
	ListeFilms() = default;
	ListeFilms(const std::string& nomFichier);
	ListeFilms(const ListeFilms& l) { assert(l.elements == nullptr); } // Pas demandé dans l'énoncé, mais on veut s'assurer qu'on ne fait jamais de copie de liste, car la copie par défaut ne fait pas ce qu'on veut.  Donc on ne permet pas de copier une liste non vide (la copie de liste vide est utilisée dans la création d'un acteur).
	~ListeFilms();
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const std::string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	Film& operator[](int index) const { return *elements[index]; }

	template <typename Func>
	Film* chercherFilmlamda(Func critere) const
	{
		for (int i = 0; i < nElements; i++)
		{
			if (critere(*elements[i]))
			{
				return elements[i];
			}
		}
		return nullptr;
	}

private:
	void changeDimension(int nouvelleCapacite);

	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
	bool possedeLesFilms_ = false; // Les films seront détruits avec la liste si elle les possède.
};


template<typename T>
class Liste {

public:
	int capacite, nElements;
	unique_ptr<shared_ptr<T>[]> elements; // Pointeur vers un tableau de Acteur*, chaque Acteur* pointant vers un Acteur.

	Liste()
	{
		capacite = 0;
		nElements = 0;
		elements = unique_ptr<shared_ptr<T>[]>{ new shared_ptr<T>[nElements] };
	}

	Liste(int nombreElements)
	{
		nElements = nombreElements;
		capacite = nombreElements;
		elements = unique_ptr<shared_ptr<T>[]>{ new shared_ptr <T>[nombreElements] };
	}

	Liste<T>& operator=(const Liste<T>& other)
	{
		if (this != &other)
		{
			nElements = other.nElements;
			capacite = other.capacite;
			elements = unique_ptr<shared_ptr<T>[]>{ new shared_ptr<T>[capacite] };
			for (int i = 0; i < nElements; i++)
			{
				elements[i] = other.elements[i];
			}
		}
		return *this;
	}

	Liste(const Liste<T>& other)
	{
		nElements = other.nElements;
		capacite = other.capacite;
		elements = unique_ptr<shared_ptr<T>[]>{ new shared_ptr<T>[capacite] };
		for (int i = 0; i < nElements; i++)
		{
			elements[i] = other.elements[i];
		}
	}
};

using ListeActeurs = Liste<Acteur>;

class Item
{
public:
	Item() = default;
	Item(string titre, int annee) :
		titre_(titre),
		annee_(annee)
	{}
	friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
	friend shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const;
	friend ostream& operator<<(ostream& os, const Film& film);
	friend void detruireFilm(Film* film);

private:
	string titre_ = " ";
	int annee_ = 0;
};

class Livre : public Item
{
public:
	Livre() = default;
	Livre(string titre, int annee, string auteur, int copieVendues, int nbrePages) :
		Item(titre, annee),
		auteur_(auteur),
		copieVendues_(copieVendues),
		nbrePages_(nbrePages)
	{}

private:
	string auteur_ = " ";
	int copieVendues_ = 0;
	int nbrePages_ = 0;
};

class Film : public Item
{
public:
	Film() = default;
	Film(string titre, int annee, string realisateur, int recette, ListeActeurs acteurs) :
		Item(titre, annee),
		realisateur_(realisateur),
		recette_(recette),
		acteurs_(acteurs)
	{}
	friend ostream& operator<<(ostream& os, const Film& film);
	friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
	friend shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const;

private:
	string realisateur_ = " ";
	int recette_ = 0;
	ListeActeurs acteurs_;
};

struct Acteur
{
	std::string nom = " "; int anneeNaissance = 0; char sexe = ' ';
};


