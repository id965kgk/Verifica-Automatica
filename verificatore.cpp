#include <fstream>
#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <list>
#include <set>

using namespace std;

#define NODE(u) G[u]
#define FIND(u) G[u].find
#define FOR_EACH_VERTEX for(std::vector<vertex_t>::iterator it = G.begin() ; it != G.end(); ++it)
//#define ALLOC(type, length) (type*) malloc(sizeof(type) * (length))
#define FOR_EACH_ARGS(v) for(std::list<int>::iterator it_arg = get_vertex(v)->arg.begin(); it_arg != get_vertex(v)->arg.end(); ++it_arg)

//struttura del nodo
typedef struct vertex_struct 
{
	int id; 				//identificativo del nodo
	string fn;				//nome del simbolo di costante o di funzione
	int find;				//identificatore del nodo che rappresenta un termine rappresentante 
							//della classe di congruenza a cui appartiene questo metodo
	list<int> arg;			//lista degli identificatori dei nodi che rappresentano i termini
							//argomenti del simbolo di funzione fn 
							//se simbolo di costante la lista è vuota
	set<int> ccpar;			//insieme degli identificatori dei nodi genitori
							//dei nodi nella classe di congruenza di questo nodo
							//se questo nodo è rappresentante
							//vuota altrimenti 
	set<int> forbidden;		//insieme degli identificatori di simboli che devono essere disuguali
} vertex_t;

std::vector<vertex_t> G;
int ID = 0;
ifstream input;
list<pair<int,int>> uguali;


static void show_usage(string name)
{
    cerr <<"\n***********************************************************************\n"
    		<< "Usage: " << name << " <option(s)>\n"
            << "Options:\n"
            << "\t-h,--help\t\tShow this help message\n"
            << "\t-s,--source SOURCE\tSpecify the source path\n"
            <<"*************************************************************************"
            << endl;
}

static void show_usage_input()
{
	cout <<"\n***********************************************************************\n"
				<< "Usage: \n"
				<< "\tWrite the equational literal in the following way:\n"
				<< " \t\ta = b \n"
				<< " \t\ta != b \n"
				<< " \ta and b can be costant symbols or functions\n"
				<< " \tfor functions the syntax is f(a) or f(a,b)\n"
				<< "\t write q to end the input\n"
		<<"*************************************************************************"
		<< endl;
}

vertex_t * get_vertex(int id) {
	for(std::vector<vertex_t>::iterator it = G.begin() ; it != G.end(); ++it) {
		if(it->id == id) {
			return  &(*it);
		}
	}
	return NULL;
}

//stampa l'albero
static void print_tree()
{
	FOR_EACH_VERTEX
	{
		cout << "VERTICE" << endl;
		cout << "\t id: " << it->id << endl;
		cout << "\t fn: " << it->fn << endl; 
		cout << "\t find: " << it->find << endl;
		cout << "\t args: ";
		FOR_EACH_ARGS(it->id)
		{
			cout << (*it_arg)<< " ";
		}
		cout << endl;


	}
}

//guarda se il vertice era già presente nel Grafo
int already_found(string fn) 
{
	for(std::vector<vertex_t>::iterator it = G.begin() ; it != G.end(); ++it) {
		if(it->fn == fn) {
			return  it->id;
		}
	}
	return -1;
}

//parserizza ricorsivamente i termini
int fillTerm(string next, int father) {
	cout << next << endl;
	int id_trovato = already_found(next);
	if(id_trovato != -1) 
	{
		//aggiornare ccpar con father
		if(father != -1)
			G[id_trovato].ccpar.insert(father);
		return id_trovato;
	}

	vertex_t v;
	v.id = ID++;
	v.fn = next;
	v.find = v.id;
	if(father != -1)
		v.ccpar.insert(father);
	if(next[next.length()-1] == ')') 
	{
		//è una funzione
		int found = next.find('(');
		//contollo che non contenga errori
		if(found == -1) 
			return -1;
		if(string(next, 0, found).find_first_of(")!=") != -1)
			return -1;
		int parentesi = 0;
		int inizio = found+1;
		for(int i = found+1; i < next.length()-1; i++) 
		{	
			if(next[i] == '=' || next[i] == '!')
				return -1;
			if(next[i] == '(')
				parentesi++;
			if(next[i] == ')')
				parentesi--;
			if(next[i] == ',' && parentesi == 0) 
			{
				int id1 = fillTerm(string(next, inizio, (i)-inizio), v.id);
				if (id1 == -1)
					return -1;
				v.arg.push_back(id1);
				inizio = i+1;
			}
		}
		if(inizio != found+1)
		{
			int id3 = fillTerm(string(next, inizio, next.length()-inizio-1), v.id);
			if (id3 == -1)
					return -1;
				v.arg.push_back(id3);
		}else
		{
			int id4 = fillTerm(string(next, inizio, next.length()-inizio-1), v.id);
			if(id4 == -1)
				return -1;
			v.arg.push_back(id4);
		}
		//G.insert(v.id,v);
		G.push_back(v);
		return v.id;
	}else
	{
		//è una costante
		//controllo che non contenga errori
		if(string(next, 0, next.length()-1).find_first_of("(!=)") != -1)
			return -1;
		G.push_back(v);
		return v.id;
	}
}

//rimuove gli spazi nella riga
string removespaces(string &str)
{    
    int m = str.length();
    int i=0;
    while(i<m)
    {
        while(str[i] == 32)
        str.erase(i,1);
        i++;
    }    
    return str;
}

//parserizza riga per riga l'input
int parse_line(string str)
{
	removespaces(str);
	int found = str.find_first_of("!=");
	if (found == -1 ) 
	{	// la stringa non è una uguaglianza/disuguaglianza
	    cout<<"The formula is not correct 1" << endl;
	    return -1;
	}else if(str[found] == '!' && str[found+1] == '=') 
	{	// è una disuguaglianza
	    int id1 = fillTerm(string(str,0, found), -1);
	    int id2 = fillTerm(string(str, found+2, -1), -1);
	    if (id1 == -1 || id2 == -1)
	    {
	    	cout<<"The formula is not correct 2 id1: "<< id1 << " id2: " << id2 <<  endl;
	    	return -1;
	    }

	    //bisogna inserirli uno nella lista proibita dell'altro
	    //o in una lista di coppie proibite
	    get_vertex(id1)->forbidden.insert(id2);
	    get_vertex(id2)->forbidden.insert(id1);
	    
	}else if(str[found] == '=') 
	{	// è una uguaglianza
		int id1 = fillTerm(string(str,0, found), -1);
	    int id2 = fillTerm(string(str, found+1, -1), -1);
	    if (id1 == -1 || id2 == -1)
	    {
	    	cout<<"The formula is not correct 3" << endl;
	    	return -1;
	    }
	    //bisogna inserirli in una lista di coppie per fare i merge
	    //pair<int, int> p (id1,id2);
	    uguali.push_back(make_pair(id1,id2));
	}else
	{	// la stringa è scritta in modo non corretto
		cout<<"The formula is not correct 4" << endl;
	    return -1;
	}
}

//legge l'input da standard input
static void read()
{
	bool b = true;
	while (b){
		string a;
		getline(cin, a);
		cout << a << endl;
		if(a.compare("q") == 0){
			b = false;
		}else{
			int error = parse_line(a);
			if(error == -1)
				return;
		}

	}
	print_tree();

}

//legge l'input da file
static void read_file()
{
	string s;
	while(input.good()) //fino a quando c'è qualcosa da leggere ..
    {
        //legge tutta la riga dal file e la mette nella variabile s
        getline(input, s);
        cout << s << endl;
        int error = parse_line(s);
        if(error == -1){
        	input.close();
        	return;
        }	
    }
    input.close(); 
    print_tree();
}




int main(int argc, char* argv[])
{
    // Check the number of parameters
    if (argc > 3) {
        // Tell the user how to run the program
        show_usage(argv[0]);
        return 1;
    }

    if(argc == 1){
    	show_usage_input();
    	read();
    }

    string source = "";
    for (int i = 1; i < argc; ++i) 
    {
        string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) 
        {
            show_usage(argv[0]);
            return 0;
        } else if ((arg == "-s") || (arg == "--source")) 
        {
            if (i + 1 < argc) 
            { 										// Make sure we aren't at the end of argv!
                source = argv[++i]; 				// Increment 'i' so we don't get the argument as the next argv[i].
            } else 
           	{ 										// there was no argument to the destination option.
                cerr << "--source option requires one argument." << std::endl;
                return 1;
            } 
        }else{
        	show_usage(argv[0]);
        	return 1;
        }
    } 

    if(source.compare("") != 0)
    {
    	
	    //apro il file di input e di output
	    input.open(source);
	    assert(input.is_open());
	    read_file();
	}


    return 0;
}