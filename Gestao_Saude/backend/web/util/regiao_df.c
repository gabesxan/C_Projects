#include "regiao_df.h"

#include <math.h>
#include <stddef.h>

/* Regioes Administrativas do DF com coordenadas reais (lat/long aproximadas
 * do centro de cada RA). Os ids seguem a numeracao oficial das RAs. */
typedef struct
{
    int id;
    const char *nome;
    double lat;
    double lon;
} RegiaoDF;

static const RegiaoDF REGIOES[] = {
    {1, "Plano Piloto", -15.7939, -47.8828},
    {2, "Gama", -16.0173, -48.0608},
    {3, "Taguatinga", -15.8330, -48.0570},
    {4, "Brazlandia", -15.6760, -48.2000},
    {5, "Sobradinho", -15.6530, -47.7900},
    {6, "Planaltina", -15.6190, -47.6540},
    {7, "Paranoa", -15.7720, -47.7790},
    {8, "Nucleo Bandeirante", -15.8700, -47.9700},
    {9, "Ceilandia", -15.8160, -48.1100},
    {10, "Guara", -15.8200, -47.9700},
    {11, "Cruzeiro", -15.7920, -47.9340},
    {12, "Samambaia", -15.8750, -48.0830},
    {13, "Santa Maria", -16.0090, -48.0190},
    {14, "Sao Sebastiao", -15.9000, -47.7780},
    {15, "Recanto das Emas", -15.9030, -48.0640},
    {16, "Lago Sul", -15.8390, -47.8700},
    {17, "Riacho Fundo", -15.8830, -48.0090},
    {18, "Lago Norte", -15.7430, -47.8400},
    {19, "Candangolandia", -15.8530, -47.9550},
    {20, "Aguas Claras", -15.8345, -48.0249},
    {21, "Riacho Fundo II", -15.9040, -48.0470},
    {22, "Sudoeste/Octogonal", -15.7950, -47.9230},
    {23, "Varjao", -15.7110, -47.8770},
    {24, "Park Way", -15.9020, -47.9620},
    {25, "SCIA/Estrutural", -15.7800, -47.9990},
    {26, "Sobradinho II", -15.6480, -47.8300},
    {27, "Jardim Botanico", -15.8700, -47.8050},
    {28, "Itapoa", -15.7480, -47.7660},
    {29, "SIA", -15.8030, -47.9560},
    {30, "Vicente Pires", -15.8030, -48.0290},
    {31, "Fercal", -15.5990, -47.8720},
};

static const int REGIOES_N = (int)(sizeof(REGIOES) / sizeof(REGIOES[0]));

static const RegiaoDF *buscar(int id)
{
    int i;
    for (i = 0; i < REGIOES_N; i++)
    {
        if (REGIOES[i].id == id)
        {
            return &REGIOES[i];
        }
    }
    return NULL;
}

const char *regiao_df_nome(int id)
{
    const RegiaoDF *r = buscar(id);
    return r != NULL ? r->nome : "";
}

double regiao_df_distancia(int a, int b)
{
    const RegiaoDF *ra = buscar(a);
    const RegiaoDF *rb = buscar(b);
    double dlat, dlon, h, c;
    const double R = 6371.0; /* raio medio da Terra em km */
    const double GRAU = 3.14159265358979323846 / 180.0;

    if (ra == NULL || rb == NULL)
    {
        return 1e6;
    }

    if (ra->id == rb->id)
    {
        return 0.0;
    }

    dlat = (rb->lat - ra->lat) * GRAU;
    dlon = (rb->lon - ra->lon) * GRAU;

    h = sin(dlat / 2) * sin(dlat / 2) +
        cos(ra->lat * GRAU) * cos(rb->lat * GRAU) *
            sin(dlon / 2) * sin(dlon / 2);
    c = 2 * atan2(sqrt(h), sqrt(1 - h));

    return R * c;
}

int regiao_df_ordenar_por_distancia(int origem, int *ids_out, int max)
{
    int i, j;
    int n = 0;
    double dist[64];

    if (ids_out == NULL || max <= 0)
    {
        return 0;
    }

    /* Copia ids + distancias num vetor local. */
    for (i = 0; i < REGIOES_N && n < max; i++)
    {
        ids_out[n] = REGIOES[i].id;
        dist[n] = regiao_df_distancia(origem, REGIOES[i].id);
        n++;
    }

    /* Ordenacao por insercao (n pequeno e estavel). */
    for (i = 1; i < n; i++)
    {
        int idTmp = ids_out[i];
        double dTmp = dist[i];
        j = i - 1;
        while (j >= 0 && dist[j] > dTmp)
        {
            dist[j + 1] = dist[j];
            ids_out[j + 1] = ids_out[j];
            j--;
        }
        dist[j + 1] = dTmp;
        ids_out[j + 1] = idTmp;
    }

    return n;
}
