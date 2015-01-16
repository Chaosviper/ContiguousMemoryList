#include <iostream>

using namespace std;

const bool IS_DEBUG_MODE = true;

class A{
 
	public:
	float x;
	int a;
		A(float x1, int x2): x(x1), a(x2){}

		void print(){ cout << x << " " << a << endl; }
};


template<typename T> class ListMod{

	// *********************************************************
	// ************************ ITERATOR ***********************
	// *********************************************************

	public:

	class iterator{

		T** pointerToPreviousElemNextPointer; // <-- Utile alla insert della classe ListMod
		T** pointerToNextPointer;
		T* pointerActualT;

		T* lastTPointer;

		friend void ListMod::insert(iterator position, const T& value); // rendo friend il metodo Insert della classe ListMod in modo da permettergli di accedere alle variabili private.
		friend iterator ListMod::Begin() const; // rendo friend il metodo Begin della classe ListMod per poter chiamare il costruttore di Iterator.
		friend iterator ListMod::End() const; // endo friend il metodo End della classe ListMod per poter chiamare il costruttore di Iterator.

		iterator(void* startingPoint, const ListMod<T>& source): lastTPointer(static_cast<T*>(source.end)) { // qui, non effettui l'accesso a nessuno dei puntatori calcolati
			T* headCasted = static_cast<T*>(startingPoint);

			pointerActualT = headCasted;
			pointerToNextPointer = reinterpret_cast<T**>(headCasted +1);
			pointerToPreviousElemNextPointer = nullptr;

			if(IS_DEBUG_MODE)
				cout << "Pointers constructor: " << pointerActualT << "     " << pointerToNextPointer << endl;
		}; 

		public:
			
			// Nel costruttore di copia mi basta copiare tutti gli indirizzi
			iterator(const iterator& toCopy){
				pointerToPreviousElemNextPointer = toCopy.pointerToPreviousElemNextPointer;
				pointerToNextPointer = toCopy.pointerToNextPointer;
				pointerActualT = toCopy.pointerActualT;
				lastTPointer = toCopy.lastTPointer;
			}

			// Anche nel caso dell'operatore di assegnamento mi basta copiare tutti gli indirizzi
			iterator& operator=(const iterator& toCopy){
				pointerToPreviousElemNextPointer = toCopy.pointerToPreviousElemNextPointer;
				pointerToNextPointer = toCopy.pointerToNextPointer;
				pointerActualT = toCopy.pointerActualT;
				lastTPointer = toCopy.lastTPointer;

				return *(this);
			}

			// Due iterator sono uguali se il puntatore all'elemento T attuale e' uguale
			bool operator==(iterator& toCheck) const{
				return pointerActualT == toCheck.pointerActualT;
			}

			bool operator!=(iterator& toCheck) const{
				return pointerActualT != toCheck.pointerActualT;
			}

			iterator& operator++(){
				// Se e' arrivato alla fine si aggiorna puntando all'elemento di END e non incrementa
				if(*(pointerToNextPointer) == nullptr){
					pointerActualT = lastTPointer;
					return *(this);
				}

				pointerToPreviousElemNextPointer = pointerToNextPointer;
				pointerActualT = *(pointerToNextPointer);
				pointerToNextPointer = reinterpret_cast<T**>(pointerActualT + 1);

				if(IS_DEBUG_MODE)
					cout  << "Pointers after ++: " << pointerActualT << "     " << pointerToNextPointer << endl;

				return *(this);
			}

			iterator operator++(int dummy){
				iterator toReturn(*(this));
				++(*(this)); // Se facessi (*this)++ entrerei di nuovo in questa funzione => loop infinito
				return toReturn;
			
			}

			// ritorna l'oggetto attuale dell'iterazione (RIFERITO ALL'ITERATOR, NON AL THIS (che e' il suo puntato))
			T& operator*() const{
				return *(pointerActualT);
			}

			// ritorna il puntatore all'oggetto attuale (RIFERITO ALL'ITERATOR, NON AL THIS (che e' il suo puntato))
			T* operator->() const{
				return pointerActualT;
			}

	};
	
	// *********************************************************
	// ************************** END **************************
	// *********************************************************

	private:
	void* head;
	void* end;
	int size;

	void* memoryHead;

	const int INITIAL_LIST_CAPACITY;
	const int BLOCK_SIZE;

	T** lastTPointerElement;

	public:
	ListMod(): size(0), INITIAL_LIST_CAPACITY(10), BLOCK_SIZE( (sizeof(T*)+ sizeof(T)) ) {
		head = malloc(BLOCK_SIZE*INITIAL_LIST_CAPACITY);
		end = head;
		lastTPointerElement = nullptr;

		memoryHead = head;
	}


	void push_front(const T& value){
		void* oldEnd = end;

		// 1) ** inserisco T nella posizione attuale di end
		T* actualPointerT = static_cast<T*>(end);
		*(actualPointerT) = value;

		actualPointerT++;

		// 2) ** inizializzo la memoria per il puntatore al next dell'elemento appena inserito (ad HEAD essendo un push_back)
		T** actualPointerPointerT = reinterpret_cast<T**>(actualPointerT);

		*(actualPointerPointerT) = static_cast<T*>(head);

		actualPointerPointerT++;

		// 3) ** Aggiorno l'ultimo elelmento e il la HEAD della lista 
		head = static_cast<T*>(oldEnd);
		end = static_cast<void*>(actualPointerPointerT);

		size++;
	}

	void push_back(const T& value){

		// 1) ** inserisco T nella posizione attuale di end
		T* actualPointerT = static_cast<T*>(end);

		// 1.1) *** se non e' il primo inserimento, inserisco il puntatore a questo elemento nello spazio riservato al next del T precedente
		if(lastTPointerElement != nullptr)
			*(lastTPointerElement) = actualPointerT;

		// 1.2) *** assegno il valore passato allo spazio allocato
		*(actualPointerT) = value;

		actualPointerT++;

		// 2) ** inizializzo la memoria per il puntatore al next dell'elemento appena inserito (nullptr essendo un push_back)
		T** actualPointerPointerT = reinterpret_cast<T**>(actualPointerT);
		*(actualPointerPointerT) = nullptr;

		// 2.1) *** aggiorno l'ultimo puntatore all'elemento di T
		lastTPointerElement = actualPointerPointerT;

		actualPointerPointerT++;

		// 3) ** Aggiorno l'ultimo elelmento e il puntatore alla fine della lista
		end = static_cast<void*>(actualPointerPointerT);

		size++;
	}

	void insert(iterator position, const T& value){
		T* elemToInsertNextValue = position.pointerActualT; // ricavabile anche da: &(*(position))
		T** previusElemNextValue = position.pointerToPreviousElemNextPointer;

		// 1) ** inserisco T nella posizione attuale di end
		T* actualPointerT = static_cast<T*>(end);

		// 1.1) *** se non e' il primo inserimento, inserisco il puntatore a questo elemento nello spazio riservato al next del T precedente
		if(previusElemNextValue != nullptr)
			*(previusElemNextValue) = actualPointerT;

		// 1.2) *** assegno il valore passato allo spazio allocato
		*(actualPointerT) = value;
/*
		// aggiorno l'iteratore (1/3)
		position.pointerActualT = actualPointerT;
*/
		actualPointerT++;

		// 2) ** inizializzo la memoria per il puntatore al next dell'elemento appena inserito (nullptr essendo un push_back)
		T** actualPointerPointerT = reinterpret_cast<T**>(actualPointerT);
		if(elemToInsertNextValue != nullptr)
			*(actualPointerPointerT) = elemToInsertNextValue;

/*
		// aggiorno l'iteratore (2/3)
		position.pointerToNextPointer = actualPointerPointerT;
*/
		actualPointerPointerT++;

		position.lastTPointer = reinterpret_cast<T*>(actualPointerPointerT);

		// 3) ** Aggiorno l'ultimo elelmento e il puntatore alla fine della lista
		end = static_cast<void*>(actualPointerPointerT);


		size++;		
	}

	// Sfrutto il mio iterator!!
	T& operator[](int pos) const{
		if(pos<size){
			ListMod<A>::iterator iter = Begin();
			
			for(int i=0; i < pos; i++){
				++iter;
			}

			return *(iter);
		}
		else{
			return *( static_cast<T*>(head));
		}

	}

	// NOTA: se mettessi una ref come ritorno di questa funzione avrei un accesso a memoria eliminata!!!!!
	iterator Begin() const{
		return iterator(head, *(this));
	}

	// NOTA: anche qui, se mettessi una ref come ritorno di questa funzione avrei un accesso a memoria eliminata!!!!!
	iterator End() const{
		return iterator(end, *(this));
	}

	~ListMod(){
		free(memoryHead);
	}

	// ******* DEBUG METHODS ********

	void printMemory() const{
		cout << "**********************************************" << endl;
		cout << "*************** MEMORY STATUS ****************" << endl;
		cout << "**********************************************" << endl;

		cout << "size of T*: " << sizeof(T*) << endl;
		cout << "size of T: " << sizeof(T) << endl << endl;

		void* actual = memoryHead;

		cout << "Head pointer : " << head << endl;
		cout << "End pointer : " << end << endl << endl;

		for(int i=0; i<size; i++){
			T* x1 = static_cast<T*>(actual);
			cout << "Indirizzo: " << x1 << endl;
			x1->print();
			x1++;


			T** x2 = reinterpret_cast<T**>(x1);
			cout << "Indirizzo Next: " << *x2 << endl;
			x2 ++;

			actual = static_cast<void*>(x2);
		}

		cout << "**********************************************" << endl;
		cout << "******************** END *********************" << endl;
		cout << "**********************************************" << endl;
	}
};


int main(){
	ListMod<A> ciao;

	ciao.push_back(A(0.2f, 2));
	ciao.push_back(A(8.3f, 3));
	ciao.push_back(A(4.2f, 4));
	ciao.push_back(A(3.1f, 5));
	ciao.push_front(A(0.0f, 1));
	ciao.push_back(A(9.0f, 6));

	if(IS_DEBUG_MODE)
		ciao.printMemory();

	ListMod<A>::iterator end = ciao.End();
	for(ListMod<A>::iterator i = ciao.Begin(); i != end; ){
		(*i++).print();  // TEST ITERATOR 1: *i++
		//i->print();
	}

	*(ciao.Begin()) = A(0.1111, 1); // TEST ITERATOR 2: *i = x

	int k=0;

	end = ciao.End();
	for(ListMod<A>::iterator i = ciao.Begin(); i != end; i++, k++){
		if(k==2){
			if(IS_DEBUG_MODE) 
				cout << "Inserted new Value A(100,100) in pos 2" << endl;
			ciao.insert(i, A(100,100));
		}
		i->print();
	}

	k=0;

	end = ciao.End();
	for(ListMod<A>::iterator i = ciao.Begin(); i != end; i++, k++){
		if(k==4){
			if(IS_DEBUG_MODE) 
				cout << "Inserted new Value A(200,200) in pos 4" << endl;
			ciao.insert(i, A(200,200));
		}
		i->print();
	}

	/*ListMod<A>::iterator x = ciao.Begin();
	x++;
	x++;
	x++;
	ciao.insert(x, A(100,100));*/
	ciao[4].print();
	ciao[3].print();
	ciao[2].print();
	ciao[1].a = 15;
	ciao[1].x = 30.2f;
	ciao[1].print();
	ciao[0].print();
	ciao[5].print();

	if(IS_DEBUG_MODE)
		ciao.printMemory();

	return 0;
}