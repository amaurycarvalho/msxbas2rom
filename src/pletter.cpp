/*
  Pletter v0.5c1

  XL2S Entertainment

  Adapted to MSXBAS2ROM by Amaury Carvalho
*/

#include "pletter.h"

Pletter::Pletter() {
    clean();
}

Pletter::~Pletter() {
    clean();
}


void Pletter::saves::init(unsigned length) {
    ep=dp=p=e=0;
    buf = new unsigned char[ length * 2 ];
}

void Pletter::saves::add0() {
    if( p==0 )
        claimevent();
    e *= 2;
    ++p;
    if( p==8 )
        addevent();
}

void Pletter::saves::add1() {
    if( p==0 )
        claimevent();
    e*=2;
    ++p;
    ++e;
    if( p==8 )
        addevent();
}

void Pletter::saves::addbit(int b) {
    if( b )
        add1();
    else
        add0();
}

void Pletter::saves::add3(int b) {
    addbit(b&4);
    addbit(b&2);
    addbit(b&1);
}

void Pletter::saves::addvar( int i ) {
    int j=32768;
    while( !(i&j) )
        j/=2;
    do {
        if( j==1 ) {
            add0();
            return;
        }
        j/=2;
        add1();
        if( i&j )
            add1();
        else
            add0();
    } while( 1 );
}

void Pletter::saves::adddata(unsigned char d) {
    buf[dp++]=d;
}

void Pletter::saves::addevent() {
    buf[ep]=e;
    e=p=0;
}

void Pletter::saves::claimevent() {
    ep=dp;
    ++dp;
}

int Pletter::saves::done(unsigned char *pDest) {
    if (p!=0) {
        while (p!=8) {
            e*=2;
            ++p;
        }
        addevent();
    }
    memcpy(pDest, buf, dp);
    return dp;
}

void Pletter::initvarcost() {
    int v=1,b=1,r=1;
    while (r!=65536) {
        for (int j=0; j!=r; ++j)
            varcost[v++]=b;
        b+=2;
        r*=2;
    }
}

void Pletter::createmetadata() {
    unsigned i, j;
    last = new unsigned[ 65536 ];
    memset( last, -1, 65536 * sizeof(unsigned) );
    prev = new unsigned[ length + 1 ];
    for( i=0; i!=length; ++i ) {
        m[i].cpos[0]=m[i].clen[0]=0;
        prev[i]=last[d[i]+d[i+1]*256];
        last[d[i]+d[i+1]*256]=i;
    }
    unsigned r=-1,t=0;
    for( i=length-1; i != unsigned(-1); --i ) {
        if( d[i] == r )
            m[i].reeks=++t;
        else {
            r=d[i];
            m[i].reeks=t=1;
        }
    }
    for (int bl=0; bl!=7; ++bl) {
        for (i=0; i<length; ++i) {
            unsigned l,p;
            p=i;
            if (bl) {
                m[i].clen[bl]=m[i].clen[bl-1];
                m[i].cpos[bl]=m[i].cpos[bl-1];
                p=i-m[i].cpos[bl];
            }
            while( ( p=prev[p] ) != unsigned(-1) ) {
                if (i-p>maxlen[bl])
                    break;
                l=0;
                while (d[p+l]==d[i+l] && (i+l)<length) {
                    if (m[i+l].reeks>1) {
                        if( ( j=m[i+l].reeks ) > m[p+l].reeks )
                            j=m[p+l].reeks;
                        l+=j;
                    } else ++l;
                }
                if( l > m[i].clen[bl] ) {
                    m[i].clen[bl]=l;
                    m[i].cpos[bl]=i-p;
                }
            }
        }
    }
}

int Pletter::getlen( pakdata *p, unsigned q ) {
    unsigned i,j,cc,ccc,kc,kmode,kl;
    p[length].cost=0;
    for( i=length-1; i!= unsigned(-1); --i ) {
        kmode=0;
        kl=0;
        kc=9+p[i+1].cost;
        j=m[i].clen[0];
        while( j > 1 ) {
            cc=9+varcost[j-1]+p[i+j].cost;
            if (cc<kc) {
                kc=cc;
                kmode=1;
                kl=j;
            }
            --j;
        }
        j=m[i].clen[q];
        if (q==1)
            ccc=9;
        else
            ccc=9+q;
        while (j>1) {
            cc=ccc+varcost[j-1]+p[i+j].cost;
            if (cc<kc) {
                kc=cc;
                kmode=2;
                kl=j;
            }
            --j;
        }
        p[i].cost=kc;
        p[i].mode=kmode;
        p[i].mlen=kl;
    }
    return p[0].cost;
}

int Pletter::save( pakdata *p, unsigned q, unsigned char *pDest ) {
    s.init(length);
    unsigned i, j;
    if( savelength ) {
        s.adddata( length & 255 );
        s.adddata( length >> 8 );
    }
    s.add3(q-1);
    s.adddata(d[0]);
    i=1;
    while( i < length ) {
        switch (p[i].mode) {
            case 0:
                s.add0();
                s.adddata(d[i]);
                ++i;
                break;
            case 1:
                s.add1();
                s.addvar(p[i].mlen-1);
                j=m[i].cpos[0]-1;
                s.adddata(j);
                i+=p[i].mlen;
                break;
            case 2:
                s.add1();
                s.addvar(p[i].mlen-1);
                j=m[i].cpos[q]-1;
                j-=128;
                s.adddata( 128 | (j & 127) );
                switch (q) {
                    case 6:
                        s.addbit(j&4096);
                    case 5:
                        s.addbit(j&2048);
                    case 4:
                        s.addbit(j&1024);
                    case 3:
                        s.addbit(j&512);
                    case 2:
                        s.addbit(j&256);
                        s.addbit(j&128);
                    case 1:
                        break;
                    default:
                        break;
                }
                i+=p[i].mlen;
                break;
            default:
                break;
        }
    }

    for( i=0; i!=34; ++i )
        s.add1();

    return s.done(pDest);
}

void Pletter::clean() {
    if( last != NULL )
        delete[] last;
    last = NULL;
    if( prev != NULL )
        delete[] prev;
    prev = NULL;
    if( s.buf != NULL )
        delete[] s.buf;
    s.buf = NULL;
    if( m != NULL )
        delete[] m;
    m = NULL;
    for( int i=1; i!=7; ++i ) {
        if( p[i] != NULL )
            delete[] p[i];
        p[i] = NULL;
    }
}

int Pletter::pack(unsigned char* pData, int dataSize, unsigned char* pDest) {
    int packed;
    unsigned char last_byte = pData[dataSize];   // last byte of source data bug fix

    if( pData == NULL ) {
        return false;
    }

    if( dataSize == 0 ) {
        return false;
    }

    savelength = false;
    offset = 0;
    length = 0;

    int i=1;
    length = (unsigned)dataSize;

    d = pData;
    m = new metadata[ length + 1 ];
    d[ length ] = 0;

    initvarcost();
    createmetadata();

    int minlen=length*1000;
    int minbl=0;

    for( i=1; i!=7; ++i ) {
        p[i] = new pakdata[ length + 1 ];
        int l=getlen( p[i], i );
        if( l < minlen && i ) {
            minlen=l;
            minbl=i;
        }
    }

    packed = save( p[minbl], minbl, pDest );

    clean();

    pData[dataSize] = last_byte;    // last byte of source data bug fix

    return packed;
}
