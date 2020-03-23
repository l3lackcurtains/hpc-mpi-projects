#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "RTree.h"

// compile
// mpic++ -O3 range_query_rtree_movie_example.cpp -lm -o
// range_query_rtree_movie_example

// To run
// mpirun -np 1 -hostfile myhostfile.txt ./range_query_rtree_movie_example

struct dataStruct {
  double x;
  double y;
};

struct queryStruct {
  double x_min;
  double y_min;
  double x_max;
  double y_max;
};

///////////////////////
// For R-tree

bool MySearchCallback(int id, void *arg) {
  // printf("Hit data rect %d\n", id);
  return true;
}

struct Rect {
  Rect() {}

  Rect(double a_minX, double a_minY, double a_maxX, double a_maxY) {
    min[0] = a_minX;
    min[1] = a_minY;

    max[0] = a_maxX;
    max[1] = a_maxY;
  }

  double min[2];
  double max[2];
};

///////////////////////

void populateSampleData(struct dataStruct *data);

int main(int argc, char **argv) {
  int my_rank, nprocs;

  const int N = 100;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (nprocs != 1) {
    printf("Please run this R-tree example program with 1 rank.");
    MPI_Finalize();
    return 0;
  }

  struct dataStruct *data =
      (struct dataStruct *)malloc(sizeof(struct dataStruct) * N);
  populateSampleData(data);

  RTree<int, double, 2, double> tree;

  for (int i = 0; i < N; i++) {
    Rect tmp = Rect(data[i].x, data[i].y, data[i].x, data[i].y);
    tree.Insert(tmp.min, tmp.max, i);
  }

  Rect search_rect = Rect(1275375600, 7.1, 1309503600, 9.1);
  unsigned int nhits =
      tree.Search(search_rect.min, search_rect.max, MySearchCallback, NULL);

  printf(
      "\nResult of Range Query\nDate range: June 1, 2010 - July 1, 2011\nDate "
      "range since Epoch: 1275375600 - 1309503600\nRatings range: "
      "7.1-9.1\nNumber of movies: %d\n\n",
      nhits);

  MPI_Finalize();
  return 0;
}

void populateSampleData(struct dataStruct *data) {
  // Dates
  data[0].x = 1287512981;
  data[1].x = 1248009980;
  data[2].x = 1241738863;
  data[3].x = 1254667001;
  data[4].x = 1331796539;
  data[5].x = 1334688447;
  data[6].x = 1315717936;
  data[7].x = 1333936113;
  data[8].x = 1296698807;
  data[9].x = 1249024605;
  data[10].x = 1299212652;
  data[11].x = 1334738166;
  data[12].x = 1273746466;
  data[13].x = 1252246660;
  data[14].x = 1296843886;
  data[15].x = 1244569813;
  data[16].x = 1314056955;
  data[17].x = 1266815084;
  data[18].x = 1254636890;
  data[19].x = 1287750156;
  data[20].x = 1254836478;
  data[21].x = 1302311330;
  data[22].x = 1274370484;
  data[23].x = 1288727623;
  data[24].x = 1325879111;
  data[25].x = 1307333991;
  data[26].x = 1248238926;
  data[27].x = 1286780624;
  data[28].x = 1324976125;
  data[29].x = 1333621623;
  data[30].x = 1245216943;
  data[31].x = 1261960582;
  data[32].x = 1319212201;
  data[33].x = 1247213937;
  data[34].x = 1332602608;
  data[35].x = 1325457580;
  data[36].x = 1312745537;
  data[37].x = 1338074232;
  data[38].x = 1314705992;
  data[39].x = 1274768893;
  data[40].x = 1340246499;
  data[41].x = 1299575071;
  data[42].x = 1254744553;
  data[43].x = 1302571320;
  data[44].x = 1260990862;
  data[45].x = 1305060595;
  data[46].x = 1307407670;
  data[47].x = 1328819973;
  data[48].x = 1265032609;
  data[49].x = 1249473705;
  data[50].x = 1284415859;
  data[51].x = 1260047156;
  data[52].x = 1277237896;
  data[53].x = 1254199649;
  data[54].x = 1314778011;
  data[55].x = 1250114442;
  data[56].x = 1336276644;
  data[57].x = 1268257984;
  data[58].x = 1319795165;
  data[59].x = 1253794531;
  data[60].x = 1241867417;
  data[61].x = 1327624063;
  data[62].x = 1260084144;
  data[63].x = 1315315593;
  data[64].x = 1278332498;
  data[65].x = 1275733319;
  data[66].x = 1255741175;
  data[67].x = 1281946762;
  data[68].x = 1287908873;
  data[69].x = 1285887448;
  data[70].x = 1243345798;
  data[71].x = 1251321465;
  data[72].x = 1251651123;
  data[73].x = 1294351191;
  data[74].x = 1269273735;
  data[75].x = 1262845140;
  data[76].x = 1324428053;
  data[77].x = 1331970217;
  data[78].x = 1338251715;
  data[79].x = 1286822934;
  data[80].x = 1259279504;
  data[81].x = 1308019986;
  data[82].x = 1317183098;
  data[83].x = 1339272327;
  data[84].x = 1335260671;
  data[85].x = 1336220255;
  data[86].x = 1255902151;
  data[87].x = 1291144841;
  data[88].x = 1327955767;
  data[89].x = 1334571107;
  data[90].x = 1323005132;
  data[91].x = 1288401966;
  data[92].x = 1293896862;
  data[93].x = 1293927184;
  data[94].x = 1278117448;
  data[95].x = 1250690055;
  data[96].x = 1274774055;
  data[97].x = 1294668977;
  data[98].x = 1270883553;
  data[99].x = 1275303539;
  // Movie ratings
  data[0].y = 6.784203561765508;
  data[1].y = 8.358395731086096;
  data[2].y = 3.3174201000225727;
  data[3].y = 3.979762834342001;
  data[4].y = 8.546825100572134;
  data[5].y = 3.208609507276753;
  data[6].y = 9.181878330735962;
  data[7].y = 6.785517327206753;
  data[8].y = 6.135861269942927;
  data[9].y = 9.244951104013168;
  data[10].y = 5.643090388057662;
  data[11].y = 6.768972859959895;
  data[12].y = 7.566092174444795;
  data[13].y = 5.528827141481347;
  data[14].y = 6.997059928295647;
  data[15].y = 7.464855388417735;
  data[16].y = 3.884204195314218;
  data[17].y = 7.831432114286016;
  data[18].y = 7.534245798413433;
  data[19].y = 5.477573621588114;
  data[20].y = 8.342631369005854;
  data[21].y = 5.495722064906385;
  data[22].y = 8.26951847222923;
  data[23].y = 9.169392807438077;
  data[24].y = 3.081684355766255;
  data[25].y = 6.486763487397656;
  data[26].y = 3.516544079804582;
  data[27].y = 8.508660321283525;
  data[28].y = 3.448471308867521;
  data[29].y = 5.487172536914999;
  data[30].y = 9.592858641273956;
  data[31].y = 5.6586230073808785;
  data[32].y = 8.34044051411492;
  data[33].y = 8.401164986310574;
  data[34].y = 5.109523467869254;
  data[35].y = 8.40917402744304;
  data[36].y = 4.070508757878201;
  data[37].y = 7.05043751515006;
  data[38].y = 3.0630559787273004;
  data[39].y = 7.963297653949918;
  data[40].y = 6.294485653636947;
  data[41].y = 8.352147991683488;
  data[42].y = 6.271748782029365;
  data[43].y = 4.883007358804797;
  data[44].y = 8.82184812304585;
  data[45].y = 6.85927458838567;
  data[46].y = 3.490570472228404;
  data[47].y = 6.307325266003238;
  data[48].y = 8.19935381060964;
  data[49].y = 4.3436925476620125;
  data[50].y = 6.250180523559459;
  data[51].y = 4.612539658607851;
  data[52].y = 6.557732762971023;
  data[53].y = 4.4599781815794985;
  data[54].y = 3.3451297132721045;
  data[55].y = 6.632033131047313;
  data[56].y = 4.205948465609259;
  data[57].y = 5.7741930125912795;
  data[58].y = 3.751557120666991;
  data[59].y = 6.568671947147529;
  data[60].y = 3.7149466277673513;
  data[61].y = 5.02102482893927;
  data[62].y = 4.622309251649333;
  data[63].y = 9.773675500558436;
  data[64].y = 4.948231867471436;
  data[65].y = 4.635357662694794;
  data[66].y = 3.640895091744623;
  data[67].y = 6.9904666229126615;
  data[68].y = 5.925485774332371;
  data[69].y = 5.574902684469007;
  data[70].y = 8.690964792701816;
  data[71].y = 5.02832001917675;
  data[72].y = 8.02174120880467;
  data[73].y = 7.290636649094704;
  data[74].y = 5.988368779754586;
  data[75].y = 8.263110788388934;
  data[76].y = 5.994688428146255;
  data[77].y = 5.998676836156745;
  data[78].y = 5.533331099297284;
  data[79].y = 4.073369345437021;
  data[80].y = 9.560332270800046;
  data[81].y = 9.414869112912259;
  data[82].y = 8.473709237426261;
  data[83].y = 7.314302615753731;
  data[84].y = 3.251996054634592;
  data[85].y = 7.580266401826529;
  data[86].y = 3.9083872624026235;
  data[87].y = 5.06051632591288;
  data[88].y = 5.525703280990594;
  data[89].y = 4.922516058557128;
  data[90].y = 3.5177829546204222;
  data[91].y = 4.064960108747936;
  data[92].y = 4.1313296732903;
  data[93].y = 9.570981897931638;
  data[94].y = 5.597937306688278;
  data[95].y = 3.3501266012176982;
  data[96].y = 8.186688890151189;
  data[97].y = 5.090761694732219;
  data[98].y = 4.430506895033195;
  data[99].y = 9.87229648830642;
}