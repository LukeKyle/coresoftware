#ifndef FUN4ALLRAW_TPCDIODECONTAINER_H
#define FUN4ALLRAW_TPCDIODECONTAINER_H

#include <phool/PHObject.h>

class TpcDiode;

class TpcDiodeContainer : public PHObject
{
 public:
  TpcDiodeContainer() = default;
  virtual ~TpcDiodeContainer() = default;

  virtual TpcDiode *AddDiode() { return nullptr; }
  virtual TpcDiode *AddDiode(TpcDiode *) { return nullptr; }
  virtual unsigned int get_ndiodes() { return 0; }
  virtual TpcDiode *get_diode(unsigned int) { return nullptr; }
  virtual void setStatus(const unsigned int) { return; }
  virtual unsigned int getStatus() const { return 0; }
  // virtual void setBco(const uint64_t) { return; }
  // virtual uint64_t getBco() const { return 0; }

 private:
  ClassDefOverride(TpcDiodeContainer, 0)
};

#endif
