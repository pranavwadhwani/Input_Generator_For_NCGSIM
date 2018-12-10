#include<iostream>
#include<cstdlib>
#include<math.h>
#include<fstream>
#include<vector>
#include<limits.h>
#include<string.h>
#include<cmath>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<cstdio>
#include<ctime>
using namespace std;
#define MAX_LINE 1024
#define link_length 200
int gTileNum,gLinkNum,gProcNum;
int g_edge_size;
int numrows;
int slice_num;
int s_size;
int a;
int counting=0;
char type;
char apcgfilename[50];
char load_condition;
int ** to_BW_requirement=NULL;
int ** from_BW_requirement=NULL;
string path;
string path1;
string path2;
string path3;
string traff;
string rtable;
string fname;
string rname;
string topology;
string rem="rm -f ";
void create_files();
void count_lines();
void generate_topology_coordinates_mapperfile();
void generate_topology_TD_config();
void generate_link_length();
bool parse_apcg(char * fileName); 
void initialize();
void generate_traffic();
void generate_application_config();
string tilecordinates;
class traffic
{
public:
vector<int> load;
vector<int> dst;
vector<int> toVolume;
}; 
traffic t[1000];
typedef struct Position
{
    int row;
    int col;
    int slice_id;
} *pPosition, Position;
bool operator==(const Position & pos1, const Position &pos2);
//////////////////////////////////////////////////////////////////// Class Link//////////////////////////////////////////////////////////////////////////////////
typedef class Link{
  // friend ostream & operator<<(ostream &os, const Link & link);
    static int cnt;
    int id;
    Position fromTile;
    Position toTile;
    int fromTileId;
    int toTileId;
    static int count;
    static int slicenum;
    static int lnk;
    static int lnk1;
    static int k;
    static bool status;
    static int record;
    static float powerCoeff;
    public:
    Link();
    float Cost() {return powerCoeff;}
    const Position & FromTilePos();
    const Position & ToTilePos();
    int FromTile() { return fromTileId;}
    int ToTile() {return toTileId;}
    int const GetCnt();
    int GetId() {return id;}
    }*pLink, Link;
///////////////////////////////////////////////////////////Class Link End//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////Class Tile//////////////////////////////////////////////////////////////////////////////////////////////////
typedef class Tile
{
public:
	static int cnt;
	int id;
	static int count;
	static int slicenum;
	Position pos;
	int goLinkNum;
	int comeLinkNum;
	vector<pLink> goLink;
	vector<pLink> comeLink;
	static float powerCoeff;
	int procId;
       class Router{
        Tile * host_tile;               
        int **routing_table;      //routing_table[i][j] shows how to which link to send the 
        //from tile i to tile j. If it's -2, means not reachable.
        //if it's -1, means the destination is the current tile.
        bool generate_xyz_routing_table();
    public:
        Router();
        bool initialize(Tile * host);
        int route_to_link(int src_tile, int dst_tile) const;
        int set_routing_entry(int src_tile, int dst_tile, int link_id);
        ~Router();
        } router;
	float Cost()
	     {
	         return powerCoeff;
	     }
	Tile();
	int GetId() const;
	int GetGoLinkNum() const;
	int GetComeLinkNum() const;
	Position GetPosition() const;
	pLink GoLink(int i);
	pLink ComeLink(int i);
	void AttachLink(pLink gLink);
        bool initialize_router();
        int RouteToLink(int srcId, int dstId) const;
        int RouteToLink(const Tile& srcTile, const Tile& dstTile) const;
        int RouteToLink(int srcId, int dstId, int linkId);
        int set_routing_entry(int src_tile, int dst_tile, int link_id);
        //friend ostream & operator<<(ostream & os, const Tile & tile);
	friend bool operator==(const Position & pos1, const Position &pos2);
	~Tile();
}*pTile, Tile;

//////////////////////////////////////////////Class Tile End///////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////Class Process//////////////////////////////////////////////////////////////////////////////////////////
class Process {
     public:
    int *toComm; 
    int *fromComm;
    int *to_BW_requirement;    // the bandwidth requirement of the out-going traffic
    int *from_BW_requirement;

    int totalCommVol;

    Process();	
friend void initialize();
 };
typedef class Process *pProcess, Process;
/////////////////////////////class Process End//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
vector<pProcess> gProcess;
Process::Process() {
    toComm = NULL;
    fromComm = NULL;
    to_BW_requirement = NULL;
    from_BW_requirement = NULL;
    toComm = new int[gProcNum];
    fromComm = new int[gProcNum];
    to_BW_requirement = new int[gProcNum];
    from_BW_requirement = new int[gProcNum];
    for (int i=0; i<gProcNum; i++) 
{
        toComm[i] = 0;
        fromComm[i] = 0;
        to_BW_requirement[i] = NULL;
        from_BW_requirement[i] = NULL;
}
}
///////////////////////////////initialize()///////////////////////////////////

void initialize()
{
to_BW_requirement = new int*[gProcNum];
for (int i=0; i<gProcNum; i++)
to_BW_requirement[i] = new int[gProcNum];
from_BW_requirement = new int*[gProcNum];
for (int i=0; i<gProcNum; i++)
from_BW_requirement[i] = new int[gProcNum];
 for (int i=0; i<gProcNum; i++)
    {
           pProcess p = new Process();
           gProcess.push_back(p);
    }            
}
//////////////////////////////////////////////Class Router/////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef class Router{
    Tile * host_tile;               
    int **routingTable;      //routingTable[i][j] shows how to which link to send the 
    //from tile i to tile j. If it's -2, means not reachable.
    //if it's -1, means the destination is the current tile.

    bool generate_xyz_routing_table();
    
public:
    Router();
    bool initialize(pTile host);
    int route_to_link(int src_tile, int dst_tile) const;
    int set_routing_entry(int src_tile, int dst_tile, int link_id);
    ~Router();
} *pRouter, Router;
//////////////////////////////////////////////Class Router End////////////////////////////////////////////////////////////////////////////////////////////////////////
float Tile :: powerCoeff=0.54;
float Link :: powerCoeff=0.00007;
pLink gLink = NULL;
pTile gTile = NULL;
int Tile::cnt=0;
int Tile::count;
int Tile::slicenum;
Tile::Tile()
{
id=cnt++;
if(count==s_size)
{
count=0;
slicenum++;
}
//cout<<"current id = "<<id<<endl;
pos.row = count / g_edge_size;
pos.col = count % g_edge_size;
pos.slice_id=slicenum;
//cout<<"tile-"<<id<<" = "<<pos.row<<pos.col<<pos.slice_id<<endl;
count++;
}
int Link::cnt;
int Link::count;
int Link::slicenum;
int Link::lnk;
int Link::lnk1;
int Link::k;
int Link::record;
bool Link::status=false;
Link::Link() { 
id = cnt++;
int x=((2 * (g_edge_size-1) * numrows) +((numrows-1) * 2 * g_edge_size));
if(count==x && record < (2*s_size) && slicenum<slice_num)
{
//cout<<"record = "<<record<<endl;
//cout<<"lnk = "<<lnk<<endl;
int a=abs((s_size*(slicenum+1))-s_size);
//cout<<"a = "<<a<<endl;
if(k<g_edge_size && status==false)
{
int localid=a+lnk;
int src=localid;
int dst=localid+s_size;
//cout<<"src="<<src<<endl;
//cout<<"dst="<<dst<<endl;
fromTile.row=(src%s_size)/g_edge_size;
toTile.row=fromTile.row;
fromTile.col=(src%s_size)%g_edge_size;
toTile.col=(dst%s_size)%g_edge_size;
fromTile.slice_id=slicenum;
toTile.slice_id=slicenum+1;
k++;
//cout<<"K = "<<k<<" ";
if(k==g_edge_size)
status=true;
//cout<<"and status = "<<status<<endl;
lnk++;
}
else if(k<=g_edge_size && status==true)
{
//cout<<"hello"<<endl;
int localid=a+lnk1;
//int localid=(localid)%(g_edge_size-1);
//cout<<"localid = "<<localid<<endl;
int src=localid+s_size;
int dst=localid;
//cout<<"src="<<src<<endl;
//cout<<"dst="<<dst<<endl;
toTile.row=(dst%s_size)/g_edge_size;
fromTile.row=toTile.row;
fromTile.col=(src%s_size)%g_edge_size;
toTile.col=(dst%s_size)%g_edge_size;
fromTile.slice_id=slicenum+1;
toTile.slice_id=slicenum;
k--;
//cout<<"k = "<<k<<endl;
if(k==0)
status=false;
lnk1++;
}
record++;
if(record==2*s_size)
{
count=0;
slicenum++;
record=0;
lnk=0;
lnk1=0;
}
}
else
{
//cout<<"Count = "<<count<<endl; 
//cout<<"Slicenum = "<<slicenum<<endl; 
    //There are totally 2*(g_edge_size-1)*g_edge_size*2 links. The first half links are horizontal
    //the second half links are veritcal links. 
    if (count < ((2 * (g_edge_size-1) * numrows))) {
        fromTile.row = count/(2*(g_edge_size-1));
        toTile.row = count/(2*(g_edge_size-1));
        fromTile.slice_id=slicenum;
	toTile.slice_id=slicenum;
        int localId = count%(2*(g_edge_size-1));
        if (localId < (g_edge_size-1)) {
            //from west to east
            fromTile.col = localId;
            toTile.col = localId + 1;	
        }
        else {
            //from east to west
            localId = localId - (g_edge_size-1);
            fromTile.col = localId + 1;
            toTile.col = localId;
        }
    }
    else {
        int localId = count-((2 * (g_edge_size-1) * numrows));
        fromTile.col = localId/(2*(numrows-1));
        toTile.col = localId/(2*(numrows-1));
	fromTile.slice_id=slicenum;
	toTile.slice_id=slicenum;
        localId = localId%(2*(numrows-1));
        if (localId < (numrows-1)) {
            //from south to north
            fromTile.row = localId;
            toTile.row = localId + 1;
        }
        else {
            //from north to south
            localId = localId - (numrows-1);
            fromTile.row = localId + 1;
            toTile.row = localId;
        }
    }
count++;   
}
//For mesh, all the links have the same power coefficency
cout<<"Link "<< id <<" created between tile-";
cout<<fromTile.row<<fromTile.col<<fromTile.slice_id;
cout<<" and tile-"<<toTile.row<<toTile.col<<toTile.slice_id<<endl;
fromTileId = fromTile.row*g_edge_size + fromTile.col+(s_size * fromTile.slice_id);
toTileId = toTile.row*g_edge_size + toTile.col+(s_size * toTile.slice_id);
cout<<"Link "<< id <<" created between tile-"<< fromTileId<<" and tile-"<<toTileId<<endl;
}
int Tile::GetId() const {
    return id;
}

int Tile::GetGoLinkNum() const {
    return goLinkNum;
}

int Tile::GetComeLinkNum() const {
    return comeLinkNum;
}

pLink Tile::GoLink(int i) {
    return goLink[i];
}

pLink Tile::ComeLink(int i) {
    return comeLink[i];
}
Tile::~Tile() {
    goLink.clear();
    comeLink.clear();
}
Tile::Router::Router() {
    routing_table = new int*[gTileNum];
    for(int i=0; i<gTileNum; i++) 
        routing_table[i] = new int[gTileNum];
    for(int i=0; i<gTileNum; i++) 
        for(int j=0; j<gTileNum; j++) 
            routing_table[i][j] = -2;
}

Tile::Router::~Router() {
    if(routing_table) {
        for(int i=0; i<gTileNum; i++) 
            delete []routing_table[i];
        delete []routing_table;
    }
}
void Tile::AttachLink(pLink gLink) {
    goLinkNum = comeLinkNum = 0;
    for (int i=0; i<gLink[0].GetCnt(); i++) {
//cout<<"gLink[0].GetCnt()"<<gLink[0].GetCnt()<<endl;
        if (gLink[i].FromTilePos() == pos) {
            goLink.push_back(&gLink[i]);
 cout<< gLink[i].GetId()<< " ";
            goLinkNum++;
        }
        if (gLink[i].ToTilePos() == pos) {
           cout<< gLink[i].GetId()<< " ";
            comeLink.push_back(&gLink[i]);
            comeLinkNum++;
        }
    }
}
Position Tile::GetPosition() const
{
    return pos;
}
int const Link::GetCnt() {
    return cnt;
}

const Position & Link::FromTilePos() {
    return fromTile;
}

const Position & Link::ToTilePos() {
    return toTile;
}
bool Tile::initialize_router()
{
    return router.initialize(this);
}

int Tile::RouteToLink(int srcId, int dstId) const {
    return router.route_to_link(srcId, dstId);
}

int Tile::RouteToLink(const Tile& srcTile, const Tile& dstTile) const {
    return router.route_to_link(srcTile.id, dstTile.id);
}

//This method is used to program the routing table
int Tile::RouteToLink(int srcId, int dstId, int linkId) {
    return router.set_routing_entry(srcId, dstId, linkId);
}
bool operator==(const Position & pos1, const Position & pos2) 
{
    return (pos1.row==pos2.row && pos1.col==pos2.col && pos1.slice_id==pos2.slice_id);
}
bool Tile::Router::initialize(pTile host) 
{
    host_tile = host;
    return generate_xyz_routing_table();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////XYZ routing/////////////////////////////////////////////////////

bool Tile::Router::generate_xyz_routing_table() {
cout<<"Generating routing table"<<endl;
    for(int i=0; i<gTileNum; i++) 
        for(int j=0; j<gTileNum; j++) 
            routing_table[i][j] = -2;
  
    for(int dstTile=0; dstTile<gTileNum; dstTile++) {
        if(dstTile == host_tile->id) {                     //deliver to me
            routing_table[0][dstTile] = -1;
            continue;
        }

        //check out the dst Tile's position first
        Position dstPos;
        dstPos.row = (dstTile%s_size)/g_edge_size;
        dstPos.col = (dstTile%s_size)%g_edge_size;
        dstPos.slice_id=dstTile/s_size;
        Position nextStep = host_tile->pos;
cout<<"Source tile is tile-"<<host_tile->pos.row<<host_tile->pos.col<<host_tile->pos.slice_id<<" and "<<"destination tile is tile-"<<dstPos.row<<dstPos.col<<dstPos.slice_id<<endl;
        if(dstPos.col != host_tile->pos.col) {            //We should go horizontally
            if(host_tile->pos.col > dstPos.col) 
                nextStep.col--;
            else 
                nextStep.col++;
cout<<"Next tile is tile-"<<nextStep.row<<nextStep.col<<nextStep.slice_id<<endl;
        }
        else if(dstPos.col == host_tile->pos.col && dstPos.row != host_tile->pos.row)
{            //We should go vertically
            if(host_tile->pos.row > dstPos.row) 
                nextStep.row--;
            else 
                nextStep.row++;
cout<<"Next tile is tile-"<<nextStep.row<<nextStep.col<<nextStep.slice_id<<endl;
        }
else if(dstPos.row==host_tile->pos.row && dstPos.col==host_tile->pos.col && dstPos.slice_id!=host_tile->pos.slice_id)
{
if(host_tile->pos.slice_id > dstPos.slice_id) 
                nextStep.slice_id--;
            else 
                nextStep.slice_id++;
cout<<"Next tile is tile-"<<nextStep.row<<nextStep.col<<nextStep.slice_id<<endl;
}

        int i=0;
        for(i=0; i<host_tile->goLinkNum; i++) {
            pLink pL = host_tile->goLink[i];
            if(pL->ToTilePos() == nextStep) {
                routing_table[0][dstTile] = pL->GetId();
                break;
            }
        }
    }

    //Duplicate this routing row to the other routing rows.
    for(int i=1; i<gTileNum; i++) 
        for(int j=0; j<gTileNum; j++) 
            routing_table[i][j] = routing_table[0][j];

    return true;
}
int Tile::Router::route_to_link(int src_tile, int dst_tile) const {
    return routing_table[src_tile][dst_tile];
}

int Tile::Router::set_routing_entry(int src_tile, int dst_tile, int link_id) {
    routing_table[src_tile][dst_tile] = link_id;
    return link_id;
}

////////////////////////////////////////////////////////////////////////////
void generate_rtable()
{
string rname=string(rem) + string(path1) + string("*");
system(rname.c_str());
create_files();
int toid, tile_id;
char str_id[500];
char str_id1[500];
ofstream rtable;
ifstream rtable1;
ofstream r1;
ofstream spd;
int pathid=0;
string routing_path=string(path1)+string("routingpath.txt");
r1.open(routing_path.c_str(), ios::app);
string spdf=string(path1)+string("same_plane_dest_file.txt");
spd.open(spdf.c_str(), ios::app);
spd<<"ST"<<" "<<"DT"<<" "<<"DTSP"<<endl;
spd.close();
int size;
for(int i=0;i<gTileNum;i++)
{
tile_id=i;
sprintf(str_id, "%d", tile_id);
size=t[i].dst.size();
if(size==0)
continue;
string rtable_filename = string(path1) + string(str_id) + string(".txt");
int linkId,tilid;
for(vector<int>::iterator m=t[i].dst.begin();m!=t[i].dst.end();m++)
{
toid=*m;
if(tile_id==toid)
continue;
else
{
r1<<tile_id<<" ";
if(gTile[tile_id].pos.slice_id!=gTile[toid].pos.slice_id)
{
spd.open(spdf.c_str(), ios::app);
spd<<tile_id<<"   "<<toid<<"    ";
}

		pTile nextTile = &gTile[tile_id];
                while (nextTile->GetId()!= toid) 
		{
tilid=nextTile->GetId();
                linkId = nextTile->RouteToLink(tile_id, toid);         
                pLink pL = &gLink[linkId];
		nextTile = &gTile[pL->ToTile()];
sprintf(str_id1, "%d", tilid);
string rtable_filename1 = string(path1) + string(str_id1) + string(".txt");
rtable.open(rtable_filename1.c_str(), ios::app);
rtable<<tile_id<<" "<<toid<<" "<<nextTile->GetId()<<" "<<pathid<<endl;
rtable.close();
r1<<nextTile->GetId()<<" ";
if(gTile[toid].pos.row==nextTile->pos.row && gTile[toid].pos.col==nextTile->pos.col)
{
spd<<nextTile->GetId()<<endl;
spd.close();
}
}
r1<<endl;
}
}
}
r1.close();
count_lines();
}


///////////////////////////////////////////////////////////////////////////////

void create_files()
{
ofstream file;
char str_id1[500];
for(int i=0;i<gTileNum;i++)
{
sprintf(str_id1, "%d", i);
string rtable_filename1 = string(path1) + string(str_id1) + string(".txt");
file.open(rtable_filename1.c_str(), ios::app);
file<<"NUM_RTABLE_ENTRIES"<<" "<<" "<<" "<<" "<<" "<<" "<<" "<<endl;
file<<"#"<<"Source"<<" "<<"destination"<<" "<<"nexttile"<<" "<<"pathid"<<endl;
file.close();
}
}

/////////////////////////////////////////////////////////////////////////////
void count_lines()
{
int number_of_lines=0, entry;
ifstream fp;
string s;
ofstream file;
char str_id1[500];
for(int i=0;i<gTileNum;i++)
{
number_of_lines=0;
sprintf(str_id1, "%d", i);
string rtable_filename1 = string(path1) + string(str_id1) + string(".txt");
fp.open(rtable_filename1.c_str(), ios::in);
while (getline(fp, s))
{
number_of_lines++;
}
fp.close();
cout<<number_of_lines<<endl;
entry=number_of_lines-2;
if(number_of_lines==2)
{
//remove(rtable_filename1.c_str());
file.open(rtable_filename1.c_str(), ios::in | ios::out);
file.seekp(19, ios::beg);
file<<"0";
file.close();
}
else
{
file.open(rtable_filename1.c_str(), ios::in | ios::out);
file.seekp(19, ios::beg);
file<<entry;
file.close();
}
}
}
////////////////////////////////////////////////////////////////////////////

void generate_topology_cordinates_mapperfile()
{
string removefile=string(rem) + string(path)+string("tileidcord.txt");
system(removefile.c_str());
ofstream tileidcordmapperfile;
tilecordinates=string(path)+string("tileidcord.txt");
tileidcordmapperfile.open(tilecordinates.c_str(), ios::app);
tileidcordmapperfile<<"Tile ID"<<" "<<"Tile-Cordinates"<<endl;
for(int i=0;i<gTileNum;i++)
{
tileidcordmapperfile<<gTile[i].id << "       "<<gTile[i].pos.row<<gTile[i].pos.col<<gTile[i].pos.slice_id<<endl;
}
tileidcordmapperfile.close();
}

//////////////////////////////////////////////////////////////////////////////
void generate_topology_TD_config()
{
ofstream topology;
rname=string(rem) + string(path2) + string("topology_TD.config");
system(rname.c_str());
string topologyconfig=string(path2) + string("topology_TD.config");
topology.open(topologyconfig.c_str(), ios::app);
int a;
Link *glink;
for(int i=0;i<gTileNum;i++)
{
topology<<gTile[i].id<<" ";
for(int j=0;j<gTile[i].GetGoLinkNum();j++)
	{
          a=gTile[i].GoLink(j)->ToTile();
//a=glink->ToTile();
//a=(*glink).ToTile();
topology<<a<<" ";
	}
topology<<"-1";
topology<<endl;
}
topology.close();
}
////////////////////////////////////////////////////////////////////////////
void generate_link_length()
{
rname=string(rem) + string(path2) + string("link_length");
system(rname.c_str());
string linklength=string(path2) + string("link_length");
ofstream file;
int a;
int * b= NULL;
int size;
Link *glink;
int length;
int count;
for(int i=0;i<gTileNum;i++)
{
   size=gTile[i].GetGoLinkNum();
   b=new int[size];
	for(int j=0;j<gTile[i].GetGoLinkNum();j++)
	{
          a=gTile[i].GoLink(j)->ToTile();
	  b[j]=a;
	}
	for(int k=0;k<gTileNum;k++)
	{
	count=0;
	for(int l=0;l<gTile[i].GetGoLinkNum();l++)
	{
	if(k==b[l])
	{
	length=link_length;
	file.open(linklength.c_str(), ios::app);
	file<<i<<" "<<k<<" "<<length<<endl;
	file.close();
	count++;
	break;
	}
	}
	if(count==0)
	{
	length=-1;
	file.open(linklength.c_str(), ios::app);
	file<<i<<" "<<k<<" "<<length<<endl;
	file.close();
	}
	}
}
}
/////////////////////////////////////Parsing Traffic Characteristics////////////////////////////////////////
bool parse_apcg(char * fileName) {
int max=0;
int load;
 
    char inputLine[MAX_LINE];
    ifstream inputFile(fileName);
    int src, dst, commVol, BW;
    if (!inputFile) {
        cerr<<"Error in openning file "<<fileName<<endl;
        return false;
    }
	 
    while (inputFile.getline(inputLine, MAX_LINE, '\n')) {
	
        if (strchr(inputLine, '#'))
            continue;
        if (strlen(inputLine) == 0)
            continue;
        if (inputLine[0] == '\r' || inputLine[0] == '\n')
            continue;
        if (!sscanf(inputLine, "%d\t%d\t%d\t%d", &src, &dst, &commVol, &BW)) 
            return false;
    if(commVol>max)
	max=commVol;
	t[src].dst.push_back(dst);
        t[src].toVolume.push_back(commVol);
         gProcess[src]->toComm[dst] = commVol;
         gProcess[src]->to_BW_requirement[dst] = BW;
         gProcess[dst]->fromComm[src] = commVol;
         gProcess[dst]->from_BW_requirement[src] = BW; 
}
for(int i=0;i<gProcNum;i++)
{
if(t[i].dst.size()==0)
continue;
for(vector<int>::iterator j=t[i].toVolume.begin(); j!=t[i].toVolume.end();j++)
{
if(load_condition=='f')
load = 100;
else if(load_condition=='o')
load=((*j)*100)/max;
t[i].load.push_back(load);
}
}
inputFile.close();
 return true;
}
//////////////////////////////generate_traffic//////////////////////////////
void generate_traffic() {
char str_id[500];
char str_id1[500];
int size;
ofstream traffic;
for(int i=0;i<gProcNum;i++)
{
int tile_id=i;
sprintf(str_id, "%d", tile_id);
size=t[tile_id].dst.size();
for(int k=0;k<size;k++)
{
for(vector<int>::iterator m=t[tile_id].dst.begin();m!=t[tile_id].dst.end();m++)
{
sprintf(str_id1, "%d", k);
string traffic_filename = string(path3) + string(str_id) + string("_") + string(str_id1);
int tid=*m;
traffic.open(traffic_filename.c_str());
traffic<<"PKT_SIZE"<<" "<<"8"<<endl;
traffic<<"DESTINATION FIXED"<<" "<<tid<<endl;
traffic<<"FLIT_ITNTERVAL"<<" "<<"2"<<endl;
traffic.close();
k++;
}
}
for(int k=0;k<size;k++)
{
for(vector<int>::iterator l=t[tile_id].load.begin();l!=t[tile_id].load.end();l++)
{
sprintf(str_id1, "%d", k);
string traffic_filename = string(path3) + string(str_id) + string("_") + string(str_id1);
traffic.open(traffic_filename.c_str(), ios::app);
traffic<<"LOAD"<<" "<<*l<<endl;
traffic.close();
k++;
}
}
}
}
/////////////////////////////////////////////////////////////////////////////
void generate_application_config()
{
rname=string(rem) + string(path) + string("application.config");
system(rname.c_str());
string applicationconfig =  string(path) + string("application.config"); 
ofstream config;
for(int i=0; i<gProcNum; i++)
{
if(t[i].dst.size()==0)
continue;
config.open(applicationconfig.c_str(), ios::app);
config<<i<<" "<<"BWCBR"<<t[i].dst.size()<<".so"<<endl;
config.close();
}
}
/////////////////////////////////////////////////////////////////////////////
int main()
{
cout<<"hello inside main"<<endl;
string config_filename=("./arch.config");
ifstream cfile;
cfile.open(config_filename.c_str());
if(!cfile.is_open())
{
cout<<"Configuration file does not exist";
exit(0);
}
else
{
cout <<"\nLoading parameters from arch.config file..."<<endl;
while(!cfile.eof())
{
string name;
cfile>>name;
if(name=="NUM_ROWS")
{
int value; 
cfile>>value;
numrows=value;
}
else if(name=="NUM_COLS")
{
int value; 
cfile>>value;
g_edge_size=value;
}
else if(name=="NUM_SLICES")
{
int value;
cfile>>value;
slice_num=value;
}
}
cout<<"Configuration Parameters Loaded Successfully!!!!"<<endl;
cout<<"Number of Rows = "<<numrows<<endl;
cout<<"Number of Columns = "<<g_edge_size<<endl;
cout<<"Number of Slices = "<<slice_num<<endl;
}
cfile.close();
s_size=numrows * g_edge_size; 
gTileNum=s_size*slice_num;
gProcNum=gTileNum;
cout<<gTileNum<<endl;
cout<<"Enter the name of file having apllication's traffic characteristics : ";
cin>>apcgfilename;
cout<<endl;
cout<<"Specify type of load you want by pressing o for optimized load and f for full load : ";
cin>>load_condition;
cout<<endl;
initialize();
parse_apcg(apcgfilename);
int single=((2 * (g_edge_size-1) * numrows) +((numrows-1) * 2 * g_edge_size))*slice_num;
int between=(2*s_size)*(slice_num-1);
gLinkNum=single+between;
gTile = new Tile[gTileNum]();
gLink = new Link[gLinkNum]();
for (int i=0; i<gTileNum; i++)
{
cout<<"Tile-"<<i<<" has outgoing and incoming links : ";
gTile[i].AttachLink(gLink);
gTile[i].initialize_router();
cout<<endl;
}
topology=string("topology_config");
fname=string("files3d2d");
rtable=string("rtable");
traff=string("traffic");
path=string(fname)+string("/");
mkdir(path.c_str(), S_IRWXU);
path1=string(path) + string(rtable) + string("/");
path2=string(path) + string(topology) + string("/");
path3=string(path) + string(traff) + string("/");
mkdir(path1.c_str(), S_IRWXU);
mkdir(path2.c_str(), S_IRWXU);
mkdir(path3.c_str(), S_IRWXU);
generate_topology_cordinates_mapperfile();
generate_topology_TD_config();
generate_link_length();
generate_traffic();
generate_application_config();
char ch;
cout<<"Do you like to generate routing tables and other routing related files?? Press 'Y' for YES and 'N' for NO...."<<endl;
cin>>ch;
if(ch=='y' or 'Y')
generate_rtable();
else if(ch=='n' or 'N')
exit(0);
return 0;
}
