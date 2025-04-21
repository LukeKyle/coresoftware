#include "TpcDiodeContainerv1.h"
#include "TpcDiodev1.h"

#include <TClonesArray.h>

static const int NTPCDIODES = 10000;

TpcDiodeContainerv1::TpcDiodeContainerv1()
{
  TpcDiodesTCArray = new TClonesArray("TpcDiodev1", NTPCDIODES);
}

TpcDiodeContainerv1::~TpcDiodeContainerv1()
{
  TpcDiodesTCArray->Clear("C");
  delete TpcDiodesTCArray;
}

void TpcDiodeContainerv1::Reset()
{
  TpcDiodesTCArray->Clear("C");
  TpcDiodesTCArray->Expand(NTPCDIODES);
}

void TpcDiodeContainerv1::identify(std::ostream &os) const
{
  os << "TpcDiodeContainerv1" << std::endl;
  os << "containing " << TpcDiodesTCArray->GetEntriesFast() << " Tpc diodes" << std::endl;
  //TpcDiode *tpcdiode = static_cast<TpcDiode *>(TpcDiodesTCArray->At(0));
  // if (tpcdiode)
  // {
  //   os << "for beam clock: " << std::hex << tpcdiode->get_bco() << std::dec << std::endl;
  // }
}

int TpcDiodeContainerv1::isValid() const
{
  return TpcDiodesTCArray->GetSize();
}

unsigned int TpcDiodeContainerv1::get_ndiodes()
{
  return TpcDiodesTCArray->GetEntriesFast();
}

TpcDiode *TpcDiodeContainerv1::AddDiode()
{
  TpcDiode *newdiode = new ((*TpcDiodesTCArray)[TpcDiodesTCArray->GetLast() + 1]) TpcDiodev1();
  return newdiode;
}

TpcDiode *TpcDiodeContainerv1::AddDiode(TpcDiode *tpcdiode)
{
  TpcDiode *newdiode = new ((*TpcDiodesTCArray)[TpcDiodesTCArray->GetLast() + 1]) TpcDiodev1(tpcdiode);
  return newdiode;
}

TpcDiode *TpcDiodeContainerv1::get_diode(unsigned int index)
{
  return (TpcDiode *) TpcDiodesTCArray->At(index);
}
