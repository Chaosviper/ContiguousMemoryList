#import <iostream>

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

		T** pointerToNextPointer;
		T* pointerActualT;

		T* lastTPointer;

		public:
			iterator(void* startingPoint, const ListMod<T>& source): lastTPointer(static_cast<T*>(source.end)) { // qui, non effettui l'accesso a nessuno dei puntatori calcolati
				T* headCasted = static_cast<T*>(startingPoint);

				pointerActualT = headCasted;
				pointerToNextPointer = reinterpret_cast<T**>(headCasted +1);

				if(IS_DEBUG_MODE)
					cout << "Pointers constructor: " << pointerActualT << "     " << pointerToNextPointer << endl;
			}; 

			bool operator==(iterator& toCheck){
				return pointerActualT == toCheck.pointerActualT;
			}

			bool operator!=(iterator& toCheck){
				return pointerActualT != toCheck.pointerActualT;
			}

			iterator& operator++(){
				// Se e' arrivato alla fine si aggiorna puntando all'elemento di END e non incrementa
				if(*(pointerToNextPointer) == nullptr){
					pointerActualT = lastTPointer;
					return *(this);
				}

				pointerActualT = *(pointerToNextPointer);
				pointerToNextPointer = reinterpret_cast<T**>(pointerActualT + 1);

				if(IS_DEBUG_MODE)
					cout  << "Pointers after ++: " << pointerActualT << "     " << pointerToNextPointer << endl;

				return *(this);
			}

			// ritorna l'oggetto attuale dell'iterazione
			T& operator*(){
				return *(pointerActualT);
			}

			// ritorna il puntatore all'oggetto attuale
			T* operator->(){
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

	// Sfrutto il mio iterator!!
	T& operator[](int pos){
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
	iterator Begin(){
		return iterator(head, *(this));
	}

	// NOTA: anche qui, se mettessi una ref come ritorno di questa funzione avrei un accesso a memoria eliminata!!!!!
	iterator End(){
		return iterator(end, *(this));
	}

	// ******* DA RIFAREE!!! ********

	void printMemory(){

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
	for(ListMod<A>::iterator i = ciao.Begin(); i != end; ++i){
		i->print();
	}

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
}