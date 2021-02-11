#include "core/cert_store.hpp"

namespace iop {
PROGMEM_STRING(
    certListNotSet,
    "Must set iop::CertList with iop::CertStore::setCertList(...). Do that in "
    "setup() using a static iop::CertStore. The iop::CertList should be in a "
    "header file generated by a python script. As `generated::certList` in "
    "(include/generated/certificates.hpp).");

constexpr const uint8_t hashSize = 32;

auto CertStore::findHashedTA(void *ctx, void *hashed_dn, size_t len)
    -> const br_x509_trust_anchor * {
  IOP_TRACE();
  auto *cs = static_cast<CertStore *>(ctx);

  // TODO: are this errors common of a symptom of a bug in our codebase, should
  // we log it? Upstreams just returns nullptr, hashed_dn isn't checked.
  // Technically BearSSL won't ever give us a nullptr hashed_dn
  if (cs == nullptr || hashed_dn == nullptr || len != hashSize)
    return nullptr;

  iop_assert(cs->maybeCertList.isSome(), certListNotSet);
  const auto &list = UNWRAP_REF(cs->maybeCertList);
  for (int i = 0; i < list.count(); i++) {
    const auto cert = list.cert(i);

    if (memcmp_P(hashed_dn, cert.index, hashSize) == 0) {
      auto der = iop::try_make_unique<uint8_t[]>(*cert.size);
      if (!der) // TODO: Should we log this?
        return nullptr;

      memcpy_P(der.get(), cert.cert, *cert.size);

      cs->x509 = iop::try_make_unique<BearSSL::X509List>(der.get(), *cert.size);
      if (!cs->x509) // TODO: Should we log this?
        return nullptr;

      der.reset(nullptr);

      const auto *taTmp = cs->x509->getTrustAnchors();

      // We can const cast because x509 is heap allocated and we own it so it's
      // mutable. This isn't a const function. The upstream API is just that way
      // NOLINTNEXTLINE cppcoreguidelines-pro-type-const-cast
      auto *ta = const_cast<br_x509_trust_anchor *>(taTmp);

      memcpy_P(ta->dn.data, cert.index, hashSize);
      ta->dn.len = hashSize;

      return ta;
    }
  }

  return nullptr;
}

void CertStore::freeHashedTA(void *ctx, const br_x509_trust_anchor *ta) {
  IOP_TRACE();
  (void)ta; // Unused

  static_cast<CertStore *>(ctx)->x509.reset(nullptr);
}

void CertStore::installCertStore(br_x509_minimal_context *ctx) {
  IOP_TRACE();
  iop_assert(maybeCertList.isSome(), certListNotSet);

  auto *ptr = static_cast<void *>(this);
  br_x509_minimal_set_dynamic(ctx, ptr, findHashedTA, freeHashedTA);
}

Cert::Cert(const uint8_t *cert, const uint8_t *index,
           const uint16_t *size) noexcept
    : size(size), index(index), cert(cert) {
  IOP_TRACE();
}
Cert::Cert(Cert &&other) noexcept
    : size(other.size), index(other.index), cert(other.cert) {
  IOP_TRACE();
}

CertList::CertList(const uint8_t *const *certs, const uint8_t *const *indexes,
                   const uint16_t *sizes,
                   const uint16_t numberOfCertificates) noexcept
    : sizes(sizes), indexes(indexes), certs(certs),
      numberOfCertificates(numberOfCertificates) {
  IOP_TRACE();
}

auto CertList::count() const noexcept -> uint16_t {
  IOP_TRACE();
  return this->numberOfCertificates;
}

auto CertList::cert(uint16_t index) const noexcept -> Cert {
  IOP_TRACE();
  // NOLINTNEXTLINE cppcoreguidelines-pro-bounds-pointer-arithmetic
  return {this->certs[index], this->indexes[index], &this->sizes[index]};
}

CertList::CertList(CertList &&other) noexcept
    : sizes(other.sizes), indexes(other.indexes), certs(other.certs),
      numberOfCertificates(other.numberOfCertificates) {
  IOP_TRACE();
}

} // namespace iop