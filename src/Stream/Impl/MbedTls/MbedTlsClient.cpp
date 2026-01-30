#include <Huenicorn/Stream/Impl/MbedTls/MbedTlsClient.hpp>

#include <stdexcept>

#include <mbedtls/debug.h>
#include <mbedtls/error.h>

#ifdef MBEDTLS_PLATFORM_C
#include <mbedtls/platform.h>
#endif

#include <Huenicorn/Logger.hpp>


namespace Huenicorn::Stream
{
  const std::string MbedTlsClient::Hostname = "Hue";
  const unsigned MbedTlsClient::HandshakeAttempts = 4;

  static void debugCallback(void* ctx, int level, const char* file, int line, const char* str)
  {
    const char* p, *basename;
    (void)ctx;

    for(p = basename = file; *p != '\0'; p++){
      if(*p == '/' || *p == '\\'){
        basename = p + 1;
      }
    }

    mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str);
  }



  MbedTlsClient::MbedTlsClient(const DtlsConfig& dtlsConfig):
  m_dtlsConfig(dtlsConfig)
  {}


  MbedTlsClient::~MbedTlsClient()
  {
    _deallocate();
  }


  bool MbedTlsClient::isConnected() const
  {
    return m_isConnected;
  }


  void MbedTlsClient::init()
  {
    mbedtls_debug_set_threshold(4);

    _initMembers();
    //_initDebug();
    _initRNG();
    _initConnection();
    _initSSL();
    _handshake();
  }


  void MbedTlsClient::shutdown()
  {
    _deallocate();
  }


  bool MbedTlsClient::send(std::span<const std::byte> requestBuffer)
  {
    if(!m_isConnected){
      Logger::error("Dtls client is not connected");
      return false;
    }

    int result = mbedtls_ssl_write(m_ssl.get(), reinterpret_cast<const unsigned char*>(requestBuffer.data()), requestBuffer.size());
    if(result < 0){
      m_isConnected = false;
      return false;
    }
    return true;
  }


  void MbedTlsClient::_initMembers()
  {
    m_serverFd.reset(new mbedtls_net_context{});
    mbedtls_net_init(m_serverFd.get());

    m_ssl.reset(new mbedtls_ssl_context{});
    mbedtls_ssl_init(m_ssl.get());

    m_conf.reset(new mbedtls_ssl_config{});
    mbedtls_ssl_config_init(m_conf.get());

    m_cacert.reset(new mbedtls_x509_crt{});
    mbedtls_x509_crt_init(m_cacert.get());

    m_ctrDrbg.reset(new mbedtls_ctr_drbg_context{});
    mbedtls_ctr_drbg_init(m_ctrDrbg.get());
  }


  void MbedTlsClient::_initDebug()
  {
    mbedtls_ssl_conf_dbg(m_conf.get(), debugCallback, NULL);
  }


  void MbedTlsClient::_initRNG()
  {
    std::string pers = "dtls_client";
    m_entropy.reset(new mbedtls_entropy_context{});
    mbedtls_entropy_init(m_entropy.get());
    int result = mbedtls_ctr_drbg_seed(
      m_ctrDrbg.get(),
      mbedtls_entropy_func,
      m_entropy.get(),
      reinterpret_cast<const unsigned char*>(pers.data()),
      pers.length()
    );

    if(result != 0){
      throw std::runtime_error("mbedtls_ctr_drbg_seed returned: " + std::to_string(result));
    }
  }


  void MbedTlsClient::_initConnection()
  {
    int result = mbedtls_net_connect(
      m_serverFd.get(),
      m_dtlsConfig.address.c_str(),
      m_dtlsConfig.port.c_str(),
      MBEDTLS_NET_PROTO_UDP
    );

    if(result != 0){
      throw std::runtime_error("mbedtls_net_connect failed with code: " + std::to_string(result));
    }
  }


  void MbedTlsClient::_initSSL()
  {
    auto pskRawArray = m_dtlsConfig.credentials.clientkeyBytes();
    auto pskIdRawArray = m_dtlsConfig.credentials.usernameBytes();

    int result = mbedtls_ssl_config_defaults(
      m_conf.get(),
      MBEDTLS_SSL_IS_CLIENT,
      MBEDTLS_SSL_TRANSPORT_DATAGRAM,
      MBEDTLS_SSL_PRESET_DEFAULT
    );

    if(result != 0){
      throw std::runtime_error("mbedtls_ssl_config_defaults failed with code: " + std::to_string(result));
    }

    mbedtls_ssl_conf_authmode(m_conf.get(), MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(m_conf.get(), m_cacert.get(), NULL);
    mbedtls_ssl_conf_rng(m_conf.get(), mbedtls_ctr_drbg_random, m_ctrDrbg.get());

    result = mbedtls_ssl_setup(m_ssl.get(), m_conf.get());

    if(result != 0){
      throw std::runtime_error("mbedtls_ssl_setup failed with code: " + std::to_string(result));
    }

    result = mbedtls_ssl_conf_psk(
      m_conf.get(),
      reinterpret_cast<const unsigned char*>(pskRawArray.data()),
      pskRawArray.size() * sizeof(unsigned char),
      reinterpret_cast<const unsigned char*>(pskIdRawArray.data()),
      pskIdRawArray.size() * sizeof(unsigned char)
    );

    if(result != 0){
      throw std::runtime_error("mbedtls_ssl_conf_psk failed with code: " + std::to_string(result));
    }

    m_ciphers.push_back(MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256);
    m_ciphers.push_back(0);
    mbedtls_ssl_conf_ciphersuites(m_conf.get(), m_ciphers.data());
    result = mbedtls_ssl_set_hostname(m_ssl.get(), Hostname.c_str());

    if(result != 0){
      throw std::runtime_error("mbedtls_ssl_set_hostname failed with code: " + std::to_string(result));
    }

    mbedtls_ssl_set_bio(
      m_ssl.get(),
      m_serverFd.get(),
      mbedtls_net_send,
      mbedtls_net_recv,
      mbedtls_net_recv_timeout
    );

    mbedtls_ssl_set_timer_cb(
      m_ssl.get(),
      &m_timer,
      mbedtls_timing_set_delay,
      mbedtls_timing_get_delay
    );
  }


  void MbedTlsClient::_handshake()
  {
    int result;
    for(unsigned attempt = 0; attempt < HandshakeAttempts; attempt++){
      mbedtls_ssl_conf_handshake_timeout(m_conf.get(), 400, 1000);
      do{
        result = mbedtls_ssl_handshake(m_ssl.get());
      }
      while(result == MBEDTLS_ERR_SSL_WANT_READ || result == MBEDTLS_ERR_SSL_WANT_WRITE);

      if(result == 0){
        break;
      }
    }

    if(result != 0){
      throw std::runtime_error("mbedtls_ssl_handshake failed with code: " + std::to_string(result));
    }

    m_isConnected = true;
    Logger::log("Dtls handshake successful");
  }


  void MbedTlsClient::_deallocate()
  {
    m_isConnected = false;
    m_serverFd.reset();
    m_cacert.reset();
    m_ssl.reset();
    m_conf.reset();
    m_ctrDrbg.reset();
    m_entropy.reset();
  }
}
