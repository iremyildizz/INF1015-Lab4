/**
* TD 4.
* \file   structures.hpp
* \author Irem et Albert
* \date   22 Mars 2023
* Créé le 25 Fevrier 2023
*/

#include <string>
#include <memory>
#include <functional>
#include <cassert>
#include "gsl/span"
#include <iostream>
using gsl::span;
using namespace std;

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.
class Affichable {
public:
	virtual ~Affichable() = default;
	friend ostream& operator<<(ostream& os, const Affichable& affichable) {
		return os << affichable.afficher() << endl;
	}
	virtual string afficher() const = 0;
};
class Item : public Affichable {
public:
	virtual ~Item() = default;
	virtual string getTitre() {
		return titre;
	};
	virtual string getTitre() const {
		return titre;
	};
	void setTitre(string nouveauTitre) {
		titre = nouveauTitre;
	}
	virtual int getAnneeSortie() {
		return anneeSortie;
	}
	virtual int getAnneeSortie() const {
		return anneeSortie;
	}
	void setAnneeSortie(int noveauAnnee) {
		anneeSortie = noveauAnnee;
	}
	string afficher() const override {
		const string ligneDeSeparation = "----------------------------------------------";
		string details = "Titre: " + titre + "\n" +
			"  Annee: " + to_string(anneeSortie) + "\n";
			return ligneDeSeparation + "\n" + details;
	}
protected:
	string titre;
	int anneeSortie = 0;
};
struct Acteur
{
	string nom; int anneeNaissance = 0; char sexe = '\0';
};

class ListeFilms {
public:
	ListeFilms() = default;
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	void detruire(bool possedeLesFilms = false);
	Film*& operator[] (int index) { assert(0 <= index && index < nElements);  return elements[index]; }
	Film* trouver(const function<bool(const Film&)>& critere) {
		for (auto& film : enSpan())
			if (critere(*film))
				return film;
		return nullptr;
	}

private:
	void changeDimension(int nouvelleCapacite);

	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
};

template <typename T>
class Liste {
public:
	Liste() = default;
	explicit Liste(int capaciteInitiale) :  // explicit n'est pas matière à ce TD, mais c'est un cas où c'est bon de l'utiliser, pour ne pas qu'il construise implicitement une Liste à partir d'un entier, par exemple "maListe = 4;".
		capacite_(capaciteInitiale),
		elements_(make_unique<shared_ptr<T>[]>(capacite_))
	{
	}
	Liste(const Liste<T>& autre) :
		capacite_(autre.nElements_),
		nElements_(autre.nElements_),
		elements_(make_unique<shared_ptr<T>[]>(nElements_))
	{
		for (int i = 0; i < nElements_; ++i)
			elements_[i] = autre.elements_[i];
	}
	//NOTE: On n'a pas d'operator= de copie, ce n'était pas nécessaire pour répondre à l'énoncé. On aurait facilement pu faire comme dans les notes de cours et définir l'operator= puis l'utiliser dans le constructeur de copie.
	//NOTE: Nos constructeur/operator= de move laissent la liste autre dans un état pas parfaitement valide; il est assez valide pour que la destruction et operator= de move fonctionnent, mais il ne faut pas tenter d'ajouter, de copier ou d'accéder aux éléments de cette liste qui "n'existe plus". Normalement le move sur les classes de la bibliothèque standard C++ laissent les objets dans un "valid but unspecified state" (https://en.cppreference.com/w/cpp/utility/move). Pour que l'état soit vraiment valide, on devrait remettre à zéro la capacité et nombre d'éléments de l'autre liste.
	Liste(Liste<T>&&) = default;  // Pas nécessaire, mais puisque c'est si simple avec unique_ptr...
	Liste<T>& operator= (Liste<T>&&) noexcept = default;  // Utilisé pour l'initialisation dans lireFilm.

	void ajouter(shared_ptr<T> element)
	{
		assert(nElements_ < capacite_);  // Comme dans le TD précédent, on ne demande pas la réallocation pour ListeActeurs...
		elements_[nElements_++] = move(element);
	}

	// Noter que ces accesseurs const permettent de modifier les éléments; on pourrait vouloir des versions const qui retournent des const shared_ptr, et des versions non const qui retournent des shared_ptr.  En C++23 on pourrait utiliser "deducing this".
	shared_ptr<T>& operator[] (int index) const { assert(0 <= index && index < nElements_); return elements_[index]; }
	span<shared_ptr<T>> enSpan() const { return span(elements_.get(), nElements_); }

private:
	int capacite_ = 0, nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_;
};

using ListeActeurs = Liste<Acteur>;

struct Film : virtual public Item
{
	virtual ~Film() = default;

	string realisateur; // Titre et nom du réalisateur (on suppose qu'il n'y a qu'un réalisateur).
	int recette=0; // Année de sortie et recette globale du film en millions de dollars
	ListeActeurs acteurs;

	string nomsActeurs() const {
		string nomActeurs = "Acteurs: ";
		for (const shared_ptr<Acteur>& acteur : acteurs.enSpan()) {
			nomActeurs += acteur->nom + ", " + to_string(acteur->anneeNaissance) + ", " + acteur->sexe;
		}
		return nomActeurs;
	}

	string detailFilm() const {
		return (" Realisateur: " + realisateur + "\n" +
			"  Recette: " + to_string(recette) + "M$\n");
	}

	string afficher() const override {
		return Item::afficher() + detailFilm() + nomsActeurs();
	}
};

struct Livre : virtual public Item
{
	virtual ~Livre() = default;
	string auteur;
	int millionsDeCopiesVendues = 0, nombreDePages = 0;

	string detailLivre() const {
		return(" Auteur: " + auteur + "\n" +
			" Millions de copies vendues: " + to_string(millionsDeCopiesVendues)+"\n" +
			" Nombre de pages: " + to_string(nombreDePages));
	}

	string afficher() const override {
		return Item::afficher() + detailLivre();
	}
};

struct FilmLivre : public Film, public Livre {
public:
	FilmLivre() = default;

	FilmLivre(Film* film, Livre* livre) {
		this->Film::setTitre(film->getTitre());
		this->Film::setAnneeSortie(film->getAnneeSortie());

		realisateur = film->realisateur;
		recette = film->recette;
		acteurs = Liste<Acteur>::Liste(film->acteurs);

		auteur = livre->auteur;
		millionsDeCopiesVendues = livre->millionsDeCopiesVendues;
		nombreDePages = livre->nombreDePages;
	}

	virtual ~FilmLivre() = default;

	string afficher() const override {
		return (Film::afficher() + " \n Livre: " + detailLivre());
	}

};

void afficherListeItems(const vector<Item*> list) {
	for (Item* elem : list) {
		cout << *elem;
	}

}

